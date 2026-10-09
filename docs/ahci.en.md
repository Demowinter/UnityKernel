# AHCI SATA Driver in UnityKernel

This document describes the current polling AHCI driver, how it exposes a SATA
disk through the block-device API, and the remaining limitations.

## Implemented

The kernel driver in `portable/drivers/cxx/ahci/ahci.cpp`:

- Scans PCI configuration space for the first AHCI controller (class
  `01:06:01`) and enables PCI memory-space and bus-master access.
- Validates and sizes the controller's 32-bit memory BAR5 (ABAR). The current
  kernel uses the BAR address directly; paging and general MMIO mapping are
  not implemented.
- Performs the optional BIOS/OS ownership handoff, resets the HBA, enables
  AHCI mode, and disables HBA interrupts because commands are polled.
- Finds the first implemented SATA port with an active link.
- Allocates aligned command-list, received-FIS, and command-table buffers from
  the kernel heap, programs the port, and sends ATA IDENTIFY DEVICE.
- Requires LBA48 and 512-byte logical sectors. Disks with other logical sector
  sizes are rejected because `BlockdevDriver` currently uses 512-byte sectors.
- Implements bounded, polling READ DMA EXT and WRITE DMA EXT operations.
  Transfers are limited to 8192 sectors (4 MiB) per command, and the public
  block-device API limits LBAs/capacity to 32 bits.
- Exposes the selected disk as `Driver::AHCI::AHCIDriver`, a
  `BlockdevDriver` with `read(lba, buffer, count)`, `write(...)`, `sectors()`,
  and `portNumber()`.

The driver is added to the driver list and initialized during kernel startup.
If there is no usable AHCI SATA disk, startup logs a warning and continues.
The existing FAT32 RAM disk remains separate; no SATA disk is mounted or
formatted automatically.

## Using the Block Device

The driver manager loads the AHCI block device during `kernelMain()`. Code
with access to the driver can use the normal block-device interface:

```cpp
#include <drivers/ahci/ahci.hpp>
#include <kernel/subsystems/driver.hpp>

auto* disk = static_cast<Driver::AHCI::AHCIDriver*>(
    Kernel::DriverSubsystem::loadDriver(Driver::Category::BLK, Driver::Type::AHCI));

uint8_t sector[Driver::Blockdev::sectorSize];
if (disk != nullptr && disk->read(0, sector, 1)) {
    // Sector 0 was read.
}

// Writes are explicit; the driver never formats or writes a disk at startup.
if (disk != nullptr && disk->write(10, sector, 1)) {
    // Sector 10 was written.
}
```

Check `sectors()` before issuing I/O and treat `false` as an I/O failure. The
implementation accepts one transfer of up to 4 MiB at a time. Split larger
requests into multiple calls and check each result. Do not use a real disk for
write testing; use a disposable QEMU image.

## Run with a QEMU SATA Disk

Create a disposable raw image:

```sh
qemu-img create -f raw sata-test.img 64M
```

Build the ISO using the Docker instructions in `README.md`, then boot it with
an emulated AHCI controller and disk:

```sh
qemu-system-x86_64 \
    -cdrom out/kernel.iso \
    -drive if=none,id=sata0,file=sata-test.img,format=raw \
    -device ahci,id=ahci \
    -device ide-hd,drive=sata0,bus=ahci.0 \
    -serial stdio
```

On startup the kernel should report the selected AHCI port and sector count.
The shell's FAT32 commands still operate on the in-memory RAM disk; SATA
sectors are available through `AHCIDriver` but are not yet mounted as a
filesystem.

## Important Limitations

- Only the first AHCI PCI controller and first usable SATA disk on it are
  exposed. SATAPI, port multipliers, multiple disks, and PCI IDE compatibility
  mode are not supported.
- PCI configuration mechanism 1 (`0xCF8`/`0xCFC`) is used. PCIe ECAM is not
  implemented.
- This is specific to the current 32-bit x86 setup. DMA addresses are the
  pointer values because the kernel currently runs without paging and uses
  identity-addressable RAM. DMA buffers and caller buffers must be contiguous
  and entirely below 4 GiB. This assumption must be replaced with explicit
  virtual-to-physical mapping and DMA allocation before enabling paging.
- Only one PRDT entry is used per request; requests over 4 MiB are rejected.
- Commands use polling with a finite iteration timeout, not a calibrated
  wall-clock timeout. There is no interrupt-driven completion, retry policy,
  cache policy, hotplug handling, or detailed error reporting.
- LBA48 is required. The current public block-device interface uses 32-bit
  LBAs and sector counts, so devices larger than that addressable range are
  only partially exposed.
- There is no partition parser, filesystem mounting for SATA, or automatic
  formatting. A failed write can still leave media contents changed; callers
  must use the right partition and filesystem code before writing.

## Next Work

1. Add a PCI subsystem and proper memory mapping for BARs instead of scanning
   PCI and dereferencing a physical ABAR directly.
2. Add a DMA allocator that returns both virtual and physical addresses,
   supports addresses above/below 4 GiB according to HBA capabilities, and
   provides cache-coherency guarantees.
3. Expose every SATA disk as a separate block device, and support more than one
   controller.
4. Improve command timeout/error reporting, recover ports after errors, and
   add focused tests for IDENTIFY parsing, LBA bounds, and PRDT construction.
5. Add partition discovery and explicitly mount a selected SATA volume; do not
   couple mounting or formatting to driver initialization.
6. Add interrupt-driven I/O only after interrupt routing and handler lifetime
   are ready.
