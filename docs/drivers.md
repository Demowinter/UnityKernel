# Driver architecture

> **Status:** this document describes the current early-stage design. Driver interfaces and resource semantics may evolve as the kernel grows.

## Purpose

In UnityKernel, a **driver is a component responsible for servicing resources**. A driver may control physical hardware when that is necessary to provide its resource, but hardware control is not the definition of a driver.

Examples include:

- a PCI or USB bus driver that discovers devices and makes them available to other drivers;
- a block-device driver that services block-storage resources;
- a filesystem driver that interprets a filesystem on a block-device resource;
- a future pipe or socket driver that implements the behavior of those resource types.

The kernel provides foundational mechanisms and shared infrastructure. Resource-specific behavior belongs in the driver that services that resource.

## Drivers and resources are different things

A **driver** is a service implementation. A **resource** is an identifiable object or service endpoint that can be used by the kernel or by another driver.

The relationship is not necessarily one-to-one:

- one driver instance may service multiple resources;
- a resource may be created by one driver and serviced by another;
- a resource ID identifies a resource, not a driver.

For example, one RAM-disk driver can manage multiple RAM-disk resources. Each disk has its own `ResourceID` and backing storage, while the driver implements the common block-device operations for all of them.

## Driver base interface

The common driver interface is declared in `include/drivers/driver.hpp`.

A driver provides:

- `initialize()`: initialize the driver and register or discover the resources it provides, as appropriate;
- `finalize()`: release the driver's resources and other owned state;
- `category()` and `type()`: report the driver's classification;
- `dependencies()`: declare driver dependencies;
- `canSatisfy()` and `useDriver()`: hooks for driver-to-driver dependency or capability coordination.

The exact lifecycle and dependency semantics are still evolving. A driver should only publish a resource after it is ready to service it, and should unregister resources before the driver becomes unavailable.

### Categories and types

The current `Driver::Category` values include `BUS`, `BLK`, and `FS`. The current `Driver::Type` values include examples such as `PCI`, `USB`, `PS2`, `RAMDISK`, `FAT32`, and `AHCI`.

These enums classify implementations; they are not resource IDs. A driver category or type alone does not identify a particular device, disk, filesystem, pipe, or socket.

## ResourceSubsystem

The resource registry is declared in `include/kernel/subsystems/resource.hpp` and implemented in `portable/kernel/cxx/resource.cpp`.

Its role is to maintain **resource metadata and identifiers**, including:

- `ResourceID`;
- `ResourceType`;
- capability bits;
- the driver that created the resource;
- the driver responsible for servicing it.

The registry is not the implementation of a resource and does not own the resource's backing object. Resource behavior remains with the responsible driver.

The main API currently includes:

- `registerResource(ResourceInfo)`: register metadata and return an assigned ID;
- `unregisterResource(ResourceID)`: remove the resource's registry entry;
- `getResourceList()`: obtain the currently registered IDs;
- `getType()` and `getCapabilities()`: inspect metadata;
- `getCreator()` and `getMaintaner()`: inspect the creator and servicing-driver pointers;
- `changeMaintaner()`: change the servicing driver.

The current API spells “maintainer” as `maintaner`; that spelling is present in the code and may be corrected in a later API change.

### Resource ID rules

`ResourceID` is a kernel-side `uint32_t`. `invalidID` is reserved as the invalid sentinel. IDs are assigned by the registry; callers should use the returned ID rather than assume a particular numeric value.

IDs may currently be recycled after unregistration. Code must not retain or use an ID after its resource has been unregistered. Reuse creates a possible stale-ID hazard, so generation-tagged IDs or another stale-reference strategy may be introduced later.

Unregistering removes metadata only. The driver remains responsible for releasing its resource-specific state and storage.

## Specialized resource interfaces

Drivers can expose interfaces specific to a resource category. The current block-device interface is declared in `include/drivers/blockdev.hpp`.

A `BlockdevDriver` implements operations such as:

- `read(resid, lba, buffer, count)`;
- `write(resid, lba, buffer, count)`;
- `sectors(resid)`;
- optionally, `resize(resid, sectors)` for implementations that support resizing.

The `resid` argument identifies which block-device resource the operation targets. This allows one block-device driver instance to service multiple resources. The current sector size constant is 512 bytes.

Not every resource supports the same operations. A block-device interface should not be treated as a universal interface for pipes, sockets, filesystems, or other resource kinds.

## Example: RAM disk

The RAM-disk implementation is declared in `include/drivers/blockdev/ramdisk.hpp` and implemented in `portable/drivers/cxx/blockdev/ramdisk.cpp`.

During initialization, the driver registers a resource and stores its backing byte vector keyed by the returned `ResourceID`. Its read, write, sector-count, and resize methods use that ID to select the corresponding disk. During finalization, it unregisters the resources it registered.

This illustrates the distinction between the driver and its resources: the driver implements block-device behavior; each registered ID refers to one specific RAM disk.

## Planned direction: adapters and userspace descriptors

The following are architectural direction, not claims about already implemented functionality:

- **Universal/category adapters** will provide a consistent contract for each resource category. They will route operations to the driver responsible for the selected resource, while respecting which operations that category supports.
- **Userspace file descriptors** will be process-local handles, not raw `ResourceID` values. Each process will have its own descriptor table mapping its descriptors to kernel resources and associated state or access rights.
- The same descriptor number in two processes may refer to different resources, or to the same resource if both tables refer to it.
- The resource registry will continue to hold metadata; it will not replace drivers or become the implementation of pipes, sockets, files, or block devices.

## Current limitations and considerations

The design is under active development. Before relying on the interfaces in more complex configurations, the implementation still needs clearly defined policies for areas such as:

- validation of IDs passed to a particular driver;
- resource and driver lifetime, including outstanding references during unregistration;
- safe ID reuse and stale references;
- synchronization when multiple execution contexts access registry or driver state;
- the meanings and compatibility rules of capability bits;
- error handling for unsupported operations and invalid resource IDs.

These are design constraints to resolve as the kernel develops, rather than guarantees provided by the current API.
