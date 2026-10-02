# AHCI Driver Foundations in UnityKernel

This document describes the initial AHCI support, the current controller
inspection API, its limitations, and the next steps toward reading and writing
SATA disks.

## Current Support

The initial implementation provides the AHCI Host Bus Adapter (HBA) register
layouts and a small controller-inspection class:

- `include/drivers/ahci/ahci.hpp` defines the global HBA registers, the 32 port
  register blocks, and their required layout sizes.
- `portable/drivers/ahci/ahci.cpp` validates a supplied mapped register region,
  reads the HBA's implemented-port bitmap, counts implemented ports, and
  classifies connected ports by their SATA signature.
- The source is compiled into the kernel target.

The controller does not discover PCI devices, map the AHCI BAR, change
controller settings, allocate DMA buffers, issue commands, or implement
block-device reads and writes. No AHCI device is currently registered with the
filesystem or block-device layer.

## Using the Controller Inspector

The caller must first find a PCI AHCI controller and map its ABAR (normally
BAR5) into the kernel's address space. Pass the mapped, uncached MMIO region
and its mapped size to `Controller::initialize()`:

```cpp
#include <drivers/ahci/ahci.hpp>

Driver::AHCI::Controller controller;
if (controller.initialize(mappedHbaRegisters, mappedSize)) {
    const uint32_t portMask = controller.implementedPortMask();
    const uint32_t portCount = controller.implementedPortCount();

    for (uint8_t port = 0; port < Driver::AHCI::portCount; ++port) {
        switch (controller.portType(port)) {
            case Driver::AHCI::PortType::SATA:
                // A SATA device is present on this implemented port.
                break;
            case Driver::AHCI::PortType::SATAPI:
                // An ATAPI device is present.
                break;
            default:
                break;
        }
    }
}
```

`initialize()` only stores the caller-provided pointer after checking that it
is non-null, suitably aligned, and covers the complete HBA register layout.
It assumes that the caller has already established a valid MMIO mapping. HBA
and port register members are volatile because they represent hardware
registers. `portRegisters()` returns `nullptr` for an uninitialized controller,
an out-of-range port, or a port not set in the HBA's implemented-port bitmap.

Port classification requires a link with device-detection status `3` and
interface-power status `1`. The signature then distinguishes SATA, SATAPI,
enclosure-management bridge, and port-multiplier devices. This is a snapshot
of status registers, not a guarantee that the link will remain present.

## Safety and Scope

Do not pass a raw PCI BAR physical address unless that address is already
mapped and directly accessible. The current 32-bit kernel has no general
physical-memory mapping interface, PCI configuration-space enumerator, or DMA
address allocator. AHCI command-list, received-FIS, and data buffers must be
DMA-accessible at the physical addresses programmed into the controller;
ordinary kernel virtual addresses are not automatically suitable.

The current inspector does not write HBA registers, so it does not reset the
controller or alter firmware configuration. Actual device I/O must not be
added until MMIO mapping, DMA memory, and timeout/error handling are in place.

## Suggested Implementation Plan

### 1. Add PCI Discovery and BAR Mapping

Implement PCI configuration-space access and enumerate devices matching the
AHCI class code (base class `0x01`, subclass `0x06`, programming interface
`0x01`). Enable memory-space and bus-master access as needed, validate BAR5,
determine its size, and map it as uncached MMIO. Pass the mapped region to
`Controller::initialize()`.

### 2. Create DMA-Safe Memory Support

Provide page-aligned allocations with known physical addresses and required
alignment. Enforce the address-width limits advertised by HBA capabilities and
the controller's DMA mode. Keep virtual and physical addresses distinct, and
ensure the memory is reserved for the lifetime of active commands.

### 3. Initialize a Port and Implement IDENTIFY

For each implemented SATA port:

1. Confirm link status and device signature.
2. Stop command/FIS processing and wait with bounded timeouts.
3. Allocate and program the command list, received-FIS area, and command table.
4. Start the port and issue ATA IDENTIFY DEVICE.
5. Parse capacity and supported addressing modes, checking command completion
   and task-file errors.

Start with polling and interrupts disabled. Add interrupt-driven completion
only after the kernel's IDT and interrupt routing can safely handle the
controller.

### 4. Implement Block Reads and Writes

Implement bounded ATA DMA commands, initially READ DMA EXT and WRITE DMA EXT
when supported. Validate LBA/count arithmetic, buffer sizes, port state, and
device capacity. Handle timeouts, task-file errors, and partial failures
explicitly. Then expose the disk through a `BlockdevDriver` implementation.

### 5. Integrate and Test

Register AHCI block devices only after command I/O works. Test with QEMU's
AHCI controller and disposable disk images. Include no-disk, unsupported
device, malformed capability, timeout, and I/O-error cases. Keep register
inspection and command execution separate so parsing and validation can be
tested without hardware.
