# UnityKernel FAT32 RAM Disk and File Shell Commands

This document explains the FAT32-related work added to UnityKernel: the new block-device abstraction, the RAM-backed FAT32 volume, the filesystem API, and the Unix-like shell commands that operate on files and directories.

The current implementation is intentionally small and kernel-friendly. It gives UnityKernel real file and directory semantics without requiring an ATA, AHCI, NVMe, USB, or virtio disk driver yet. The storage is volatile and backed by kernel memory, but the internal layout follows FAT32-style structures: a boot sector, a FAT table, clusters, short 8.3 directory entries, cluster chains, and directory traversal.

## What Was Added

The change adds four new source files:

- `include/kernel/block.hpp`
- `portable/kernel/cxx/block.cpp`
- `include/kernel/fat32.hpp`
- `portable/kernel/cxx/fat32.cpp`

It also updates:

- `portable/kernel/CMakeLists.txt`
- `portable/kernel/cxx/kernel.cpp`
- `portable/kernel/cxx/shell.cpp`

At a high level:

- `Kernel::Block::MemoryDevice` provides sector-based read/write access over a memory buffer.
- `Kernel::FAT32` creates and manages a small FAT32-style RAM disk.
- `kernelMain()` mounts the RAM disk during startup.
- The shell exposes file commands such as `ls`, `cd`, `cat`, `touch`, `write`, `append`, `mkdir`, `rm`, and `stat`.

## Current User-Facing Commands

The shell now supports these file-related commands:

| Command | Purpose |
| --- | --- |
| `pwd` | Print the current working directory. |
| `ls [path]` | List a directory or show one file entry. |
| `cd <path>` | Change the current working directory. |
| `cat <file>` | Print file contents. |
| `touch <file>` | Create an empty file if it does not exist. |
| `write <file> <text>` | Replace a file with text. Creates the file if needed. |
| `append <file> <text>` | Append text to a file. Creates the file if needed. |
| `mkdir <dir>` | Create a directory. |
| `rm <path>` | Remove a file or an empty directory. |
| `stat <path>` | Print basic metadata: type, display name, size, and first cluster. |

The prompt now includes the current directory:

```text
/ $
```

If you run `cd tmp`, the prompt becomes:

```text
/tmp $
```

## Example Shell Session

After boot, the filesystem starts with:

- `/welcome.txt`
- `/tmp`

Example session:

```text
/ $ ls
- 37 welcome.txt
d 0 tmp
/ $ cat welcome.txt
UnityKernel FAT32 ram disk is online.
/ $ mkdir docs
/ $ cd docs
/docs $ write note.txt hello from UnityKernel
/docs $ cat note.txt
hello from UnityKernel
/docs $ append note.txt  again
/docs $ cat note.txt
hello from UnityKernel again
/docs $ stat note.txt
file note.txt
size: 28
cluster: 5
/docs $ cd ..
/ $ rm docs
rm: docs: directory not empty
/ $ rm docs/note.txt
/ $ rm docs
```

`write` and `append` treat everything after the file path as text. Quoting is not implemented yet, so file names cannot include spaces, but file contents can.

## Startup Flow

The relevant startup flow is:

1. `kernelMain()` clears the console.
2. Memory is initialized through `Memory::initialize()`.
3. C++ runtime constructors are run through `CXXRuntime::initialize()`.
4. Architecture-specific initialization runs through `Arch::initialize()`.
5. `FAT32::initialize()` creates and mounts the RAM-backed FAT32 volume.
6. The kernel prints `Mounted FAT32 ram disk`.
7. The interactive shell starts.

The filesystem is ready before the shell prompt appears.

## Block Layer

The block layer is currently very small:

```cpp
namespace Kernel::Block {
    constexpr uint32_t sectorSize = 512;

    class MemoryDevice {
    public:
        bool initialize(uint8_t* storage, uint32_t sectorCount);
        bool read(uint32_t lba, void* buffer, uint32_t count = 1) const;
        bool write(uint32_t lba, const void* buffer, uint32_t count = 1);
        uint32_t sectors() const;
    };
}
```

`MemoryDevice` exposes a disk-like interface:

- All access is sector-based.
- The sector size is fixed at 512 bytes.
- Reads and writes use LBA sector numbers.
- Bounds are checked before copying.
- The backing storage is a raw `uint8_t*` memory region owned by the filesystem implementation.

This is deliberately shaped like a real block device so a future hardware driver can replace the memory backend more easily. For example, a future ATA driver could expose the same read/write style while using real disk sectors instead of an in-memory array.

## FAT32 Volume Geometry

The current FAT32 RAM disk uses fixed compile-time geometry:

| Field | Value |
| --- | --- |
| Sector size | 512 bytes |
| Sectors per cluster | 1 |
| Cluster size | 512 bytes |
| Reserved sectors | 32 |
| FAT sectors | 2 |
| Number of FATs in BPB | 1 |
| Data clusters | 128 |
| Root cluster | 2 |
| Total sectors | 162 |
| Total disk size | 82,944 bytes |

The total sector count is:

```text
reservedSectors + fatSectors + clusterCount * sectorsPerCluster
= 32 + 2 + 128 * 1
= 162 sectors
```

The total byte size is:

```text
162 * 512 = 82,944 bytes
```

This is small, but enough for basic shell-level file and directory testing.

## On-Disk Layout Inside RAM

Even though the storage is in memory, it is arranged like a disk image:

```text
+------------------------------+
| Reserved region              |
| - sector 0: boot sector      |
| - other reserved sectors     |
+------------------------------+
| FAT region                   |
| - 2 sectors                  |
+------------------------------+
| Data region                  |
| - cluster 2: root directory  |
| - cluster 3..: files/dirs    |
+------------------------------+
```

The conversion from cluster number to LBA is:

```text
LBA = reservedSectors + fatSectors + (cluster - rootCluster) * sectorsPerCluster
```

With the current constants:

```text
LBA = 32 + 2 + (cluster - 2)
```

So:

- cluster 2 maps to sector 34
- cluster 3 maps to sector 35
- cluster 4 maps to sector 36

## Boot Sector

`writeBootSector()` creates a FAT32-style BIOS Parameter Block in sector 0.

Important fields written:

- jump instruction
- OEM name: `UNITYKRN`
- bytes per sector: `512`
- sectors per cluster: `1`
- reserved sector count: `32`
- FAT count: `1`
- total sectors: `162`
- FAT size: `2`
- root cluster: `2`
- volume serial number
- volume label: `UNITYKERNEL`
- filesystem type text: `FAT32`
- boot signature: `0x55AA`

This boot sector is useful for representing the volume as FAT32-like data. The kernel currently does not parse an external FAT32 boot sector; it creates a known in-memory one.

## FAT Table

The FAT table stores one 32-bit entry per cluster. Only the low 28 bits are used, matching FAT32 convention.

Special values:

| Value | Meaning |
| --- | --- |
| `0` | Free cluster |
| `0x0FFFFFF8` and above | End of cluster chain |
| Cluster number | Next cluster in file/directory chain |

During initialization:

- FAT entry 0 is reserved.
- FAT entry 1 is reserved.
- FAT entry 2 is marked end-of-chain for the root directory.

Cluster allocation scans from cluster 3 upward. When a free cluster is found:

1. Its FAT entry is marked `endOfChain`.
2. The cluster is zeroed.
3. The cluster number is returned to the caller.

## Directory Entries

Directories are stored as clusters containing 32-byte FAT directory entries.

The implementation uses:

```cpp
struct DirectoryEntry {
    char name[11];
    uint8_t attr;
    uint8_t ntReserved;
    uint8_t creationTenths;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t accessDate;
    uint16_t firstClusterHigh;
    uint16_t writeTime;
    uint16_t writeDate;
    uint16_t firstClusterLow;
    uint32_t fileSize;
} __attribute__((packed));
```

Each 512-byte cluster can hold:

```text
512 / 32 = 16 directory entries
```

The implementation skips:

- deleted entries, where first byte is `0xE5`
- long-file-name entries
- `.` and `..` entries when listing normal directory contents

Directories created by `mkdir` receive:

- `.` pointing to the directory itself
- `..` pointing to the parent directory

## File Names

This implementation supports classic FAT short names only.

Allowed form:

```text
BASE.EXT
```

Rules:

- Base name: 1 to 8 characters.
- Extension: 0 to 3 characters.
- Names are stored uppercase in directory entries.
- Names are displayed lowercase in shell output.
- `.` and `..` are reserved.
- Long file names are not supported.
- Spaces in file names are not supported.

Allowed characters:

- `A-Z`
- `a-z`
- `0-9`
- `_`
- `-`
- `$`
- `~`

Examples:

| Input | Stored short name | Displayed |
| --- | --- | --- |
| `note.txt` | `NOTE    TXT` | `note.txt` |
| `KERNEL.BIN` | `KERNEL  BIN` | `kernel.bin` |
| `tmp` | `TMP        ` | `tmp` |

Invalid examples:

- `verylongname.txt`
- `archive.tar.gz`
- `.hidden`
- `my file.txt`

## Path Handling

The filesystem supports absolute and relative paths.

Examples:

```text
/welcome.txt
tmp/file.txt
./note.txt
../other.txt
```

Path resolution walks one component at a time:

1. Pick the starting cluster:
   - `/...` starts at root cluster 2.
   - relative paths start at the current working directory cluster.
2. Split the path by `/`.
3. Handle `.` by staying in the current directory.
4. Handle `..` by following the directory's `..` entry.
5. Convert normal components to FAT 8.3 short names.
6. Search the current directory for that short name.
7. If more path components remain, the matched entry must be a directory.

There are two important resolver paths in the implementation:

- `resolveExisting()` finds an existing file or directory.
- `resolveParent()` finds the parent directory and the final short name for create operations.

For example:

```text
write /tmp/log.txt hello
```

uses `resolveParent()`:

- parent path: `/tmp`
- leaf name: `log.txt`
- parent cluster: cluster for `/tmp`
- resulting short name: `LOG     TXT`

Then the code creates or updates the file entry in that parent directory.

## File Creation and Writing

`writeFile(path, data, append)` handles both `write` and `append`.

When the file does not exist:

1. Resolve the parent directory.
2. Validate the leaf name as FAT 8.3.
3. Allocate enough clusters for the data.
4. Write file data into the cluster chain.
5. Create a directory entry with archive attribute and file size.

When the file exists:

- If it is a directory, return `isDirectory`.
- For overwrite mode:
  - free the old cluster chain
  - allocate a new chain if needed
  - write from offset 0
  - set the new file size
- For append mode:
  - preserve existing data
  - grow the cluster chain if needed
  - write from the old file size
  - set the new file size

Appending does not automatically insert a space or newline. The shell command uses the exact text after the file path.

## Reading Files

`readFile(path, callback, context)` reads a file cluster-by-cluster.

It does not return one large buffer. Instead, it calls a callback for each chunk:

```cpp
using ChunkCallback = void (*)(std::string_view chunk, void* context);
```

The shell uses this to print file contents directly to the console. This is a good pattern for kernel code because it avoids unnecessary heap allocations and allows streaming later.

## Listing Directories

`list(path, callback, context)` lists either:

- one file entry, if `path` points to a file
- all visible entries in a directory, if `path` points to a directory

It reports entries through:

```cpp
using ListCallback = void (*)(const EntryInfo& entry, void* context);
```

`EntryInfo` contains:

- display name
- directory/file boolean
- file size
- first cluster

The shell prints a compact listing:

```text
- 37 welcome.txt
d 0 tmp
```

The first column is:

- `-` for a file
- `d` for a directory

## Removing Files and Directories

`remove(path)` supports:

- deleting files
- deleting empty directories

For files:

1. Resolve the path.
2. Free the cluster chain if the file has data clusters.
3. Mark the directory entry deleted by writing `0xE5` to the first name byte.

For directories:

1. Resolve the path.
2. Reject root, `.`, and `..`.
3. Check that the directory is empty except for `.` and `..`.
4. Free the directory's cluster chain.
5. Mark the parent directory entry deleted.

Non-empty directory removal returns `directoryNotEmpty`.

## Status and Error Handling

Filesystem functions return `FAT32::Status`.

Current statuses:

| Status | Meaning |
| --- | --- |
| `ok` | Operation succeeded. |
| `notReady` | Filesystem was not initialized. |
| `invalidName` | Path component is not a valid FAT 8.3 name or operation targets an invalid entry. |
| `notFound` | Entry does not exist. |
| `alreadyExists` | Create operation found an existing entry. |
| `notDirectory` | A path component should be a directory but is not. |
| `isDirectory` | Operation expected a file but found a directory. |
| `directoryNotEmpty` | Tried to remove a non-empty directory. |
| `noSpace` | No free clusters remain. |
| `ioError` | Block operation or internal metadata operation failed. |

The shell converts statuses to user-facing messages through `statusText()`.

Example:

```text
cat: tmp: is a directory
rm: docs: directory not empty
write: verylongname.txt: invalid FAT32 8.3 name
```

## Current Limitations

This is a practical first filesystem layer, not a full FAT32 driver yet.

Important limitations:

- Storage is RAM-backed and volatile.
- Files disappear after reboot.
- No real disk driver is connected yet.
- The kernel creates a fixed known FAT32 layout instead of mounting an arbitrary external FAT32 image.
- Long file names are not supported.
- Only FAT 8.3 short names are supported.
- No timestamps are maintained.
- No permission model exists yet.
- No file descriptors exist yet.
- No seek API exists yet.
- No binary shell input exists yet.
- `write` and `append` are text-oriented.
- No `cp`, `mv`, `rmdir`, `hexdump`, `find`, or `tree` commands yet.
- No quoting or escaping in the shell parser.
- The volume size is fixed and small.
- The implementation uses one FAT region and does not mirror multiple FATs.
- There is no fsck-style consistency checker.

## Why This Shape Is Useful

Even with these limitations, this design gives the kernel several useful foundations:

- A filesystem namespace.
- Current working directory support.
- Directory traversal.
- File creation and deletion.
- Cluster allocation and freeing.
- A sector-based block abstraction.
- A user-facing shell workflow for validating kernel file behavior.

Most importantly, the shell does not directly manipulate file structures. It calls the `Kernel::FAT32` API. That separation makes it easier to:

- replace the RAM disk with a real block device later
- add another filesystem later
- build a VFS layer above FAT32
- add file descriptors and process-like APIs

## Suggested Next Steps

Reasonable next steps are:

1. Add a real block-device driver.
   - Start with ATA PIO or virtio-blk for QEMU.
   - Keep the same sector-based interface.

2. Parse an existing FAT32 boot sector.
   - Read BPB values instead of using compile-time constants.
   - Validate FAT count, sector size, cluster size, root cluster, and FAT size.

3. Add a VFS layer.
   - Define generic file operations independent of FAT32.
   - Mount FAT32 under `/`.
   - Later mount other filesystems or device nodes.

4. Add file descriptors.
   - `open`
   - `read`
   - `write`
   - `seek`
   - `close`

5. Improve shell parsing.
   - Quoted strings.
   - Escapes.
   - Multiple arguments.
   - Better error messages.

6. Add more Unix-like commands.
   - `cp`
   - `mv`
   - `rmdir`
   - `hexdump`
   - `tree`
   - `more`

7. Add tests or a host-side harness.
   - Build the FAT32 logic in a host test binary.
   - Test path resolution, allocation, deletion, append, and directory behavior.

## Build and Run Through Docker

Build the kernel image:

```sh
docker build -t unitykernel .
```

Export the ISO:

```sh
mkdir -p out
docker run --rm -v "$PWD/out:/out" unitykernel \
    sh -c 'cp build/kernel.iso /out/kernel.iso'
```

Run QEMU from Docker without a graphical display:

```sh
docker run --rm -it unitykernel \
    qemu-system-x86_64 \
    -cdrom /workspace/build/kernel.iso \
    -display none \
    -serial stdio
```

For a bounded smoke test, use `timeout`:

```sh
docker run --rm unitykernel \
    timeout 8 qemu-system-x86_64 \
    -cdrom /workspace/build/kernel.iso \
    -display none \
    -serial stdio \
    -no-reboot
```

An exit code of `124` from `timeout` means QEMU was still running when the timeout expired. In this case that is expected because the kernel enters the interactive shell and waits for input.

