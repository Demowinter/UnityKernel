# ACPI Table Discovery in UnityKernel

This document describes the initial ACPI support in UnityKernel, how to use
the current API, its limitations, and a suggested order for implementing the
next ACPI features.

## Current Support

UnityKernel receives ACPI information through the Multiboot2 boot protocol:

1. UnityBoot scans the Multiboot2 information tags for `AcpiOld` and
   `AcpiNew`.
2. If both are present, it keeps the ACPI 2.0-or-newer RSDP from `AcpiNew`.
3. UnityBoot passes the RSDP address and tag payload size to the kernel in
   `UnityBootProtocol::BootInfo`.
4. During kernel startup, `Driver::ACPI::initialize()` validates the RSDP
   checksums and locates the RSDT or XSDT.

The ACPI implementation is in `portable/drivers/acpi/acpi.cpp`, with its
public declarations in `include/drivers/acpi/acpi.hpp`. Its source is compiled
as part of the kernel target.

The current implementation can:

- Validate the RSDP signature and legacy checksum.
- Validate the extended checksum for an ACPI 2.0-or-newer RSDP.
- Locate an RSDT or XSDT, validate its System Description Table (SDT)
  checksum and entry layout, and expose it through `rootTable()`.
- Search root-table entries by their four-character signature and return a
  table only when its SDT checksum is valid.

This is table discovery only. It does not interpret AML, enumerate ACPI
devices, configure interrupt controllers, or perform ACPI power management.

## Using the API

The public API is in the `Driver::ACPI` namespace:

```cpp
#include <drivers/acpi/acpi.hpp>

if (Driver::ACPI::isInitialized()) {
    const auto* madt = Driver::ACPI::findTable("APIC");
    if (madt != nullptr) {
        const auto* tableData =
            reinterpret_cast<const uint8_t*>(madt) + sizeof(*madt);
        const uint32_t tableDataSize = madt->length - sizeof(*madt);

        // Parse `tableData` using `tableDataSize` bytes.
    }
}
```

`"APIC"` is the signature of the Multiple APIC Description Table (MADT). The
returned pointer refers to firmware-provided memory and is read-only. The
common SDT header is described by `SystemDescriptionTable`; table-specific
data starts immediately after that header. Always use the SDT `length` to
bound reads of table-specific data.

`initialize(rsdp, rsdpSize)` returns `true` only when a usable root table is
found. Kernel startup currently reports success or warns if ACPI is missing
or invalid. `findTable()` returns `nullptr` when ACPI has not initialized, the
signature is null, or no valid table with that signature is present.

## Addressing and Safety Limitations

The kernel is currently built for 32-bit x86. Firmware ACPI addresses are
physical addresses, while this implementation currently treats them as
directly dereferenceable pointers. This works only when the relevant ACPI
memory is identity-mapped and addressable by the kernel. A 64-bit XSDT address
that does not fit in `uintptr_t` cannot be used; the implementation then tries
the RSDT when one is available.

Before relying on ACPI on other machines, implement physical-memory mapping
and translate firmware physical addresses through that mapping. The parser
also trusts that addressable firmware memory can be read for the lengths in
its table headers. A robust memory-map-aware parser should validate that
tables lie in readable firmware memory and impose reasonable size bounds
before checksumming or walking them.

No ACPI tables are copied or reserved by UnityBoot. Their lifetime and memory
reservation must be considered when the kernel's physical memory manager is
introduced.

## Suggested Implementation Plan

### 1. Parse the MADT

Use `findTable("APIC")` and walk the variable-length records following the
common SDT header. Each record begins with a type and a length; reject records
shorter than their type's minimum size, zero-length records, and records that
extend beyond the MADT length.

Recognize at least:

- Processor Local APIC records.
- I/O APIC records.
- Interrupt Source Override records.
- Local APIC Address Override records.
- Local APIC NMI records, when needed.

Preserve unknown record types by skipping their validated lengths so newer
firmware records do not break parsing. Keep parsing separate from hardware
initialization and test it with synthetic MADT byte buffers.

### 2. Add Physical Memory Access

Create a physical-address mapping interface before dereferencing firmware
tables outside the currently accessible identity-mapped region. Use it for
both ACPI tables and memory-mapped APIC registers. Validate address ranges
and lengths against the physical memory map.

### 3. Initialize APIC Interrupt Routing

After parsing and mapping the MADT:

1. Detect CPU APIC support and enable the local APIC as required.
2. Map and initialize the I/O APIC.
3. Apply MADT interrupt-source overrides when choosing interrupt vectors.
4. Configure interrupt masking and routing.
5. Test timer and keyboard interrupts before depending on APIC routing for
   normal kernel operation.

This work also depends on a functioning IDT and interrupt-handler framework.
Do not replace the existing interrupt-controller setup until the APIC path
has been validated.

### 4. Add FADT and Power Management

Use `findTable("FACP")` to locate the Fixed ACPI Description Table (FADT).
Implement only the needed fixed hardware interfaces first. AML interpretation
is a larger, separate feature and should not be assumed to be present just
because the FADT can be found.

### 5. Improve Validation and Testing

Add focused tests for valid and invalid RSDPs, RSDT/XSDT selection, checksum
failures, malformed root entries, and table lookup. Keep physical-memory
access behind a testable interface so parser tests do not dereference real
firmware addresses.
