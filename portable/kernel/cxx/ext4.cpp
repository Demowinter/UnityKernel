#include <libbase/memory.hpp>
#include <drivers/blockdev.hpp>
#include <kernel/ext4.hpp>

namespace Driver::FS {
    using namespace EXT4;

    namespace {
        constexpr size_t kSector = Blockdev::sectorSize;
        constexpr size_t kChunk = 1024;
        constexpr uint64_t kUnbounded = 0xFFFFFFFFull;

        // Change here if fs.hpp gets dedicated codes (e.g. Status::readOnly / Status::unsupported).
        constexpr Status kReadOnly = Status::ioError;
        constexpr Status kUnsupported = Status::ioError;
        // Refuse writes on volumes that were not cleanly unmounted (run e2fsck first).
        constexpr bool kRequireCleanForWrite = true;

        constexpr uint16_t kMagic = 0xEF53;
        constexpr uint16_t kExtentMagic = 0xF30A;
        constexpr size_t kSbOffset = 1024;
        constexpr size_t kSbSize = 1024;

        namespace Sb {
            constexpr size_t inodeCount = 0x00, blocksLo = 0x04, freeBlocksLo = 0x0C, freeInodes = 0x10;
            constexpr size_t firstDataBlock = 0x14, logBlockSize = 0x18, logClusterSize = 0x1C;
            constexpr size_t blocksPerGroup = 0x20, clustersPerGroup = 0x24, inodesPerGroup = 0x28;
            constexpr size_t maxMntCount = 0x36, magic = 0x38, state = 0x3A, errors = 0x3C;
            constexpr size_t revLevel = 0x4C, firstIno = 0x54, inodeSize = 0x58;
            constexpr size_t featCompat = 0x5C, featIncompat = 0x60, featRoCompat = 0x64;
            constexpr size_t uuid = 0x68, volumeName = 0x78, descSize = 0xFE, firstMetaBg = 0x104;
            constexpr size_t blocksHi = 0x150, freeBlocksHi = 0x158, minExtra = 0x15C, wantExtra = 0x15E;
            constexpr size_t backupBgs = 0x24C;
        }

        namespace Gd {
            constexpr size_t blockBitmapLo = 0x00, inodeBitmapLo = 0x04, inodeTableLo = 0x08;
            constexpr size_t freeBlocksLo = 0x0C, freeInodesLo = 0x0E, usedDirsLo = 0x10;
            constexpr size_t blockBitmapHi = 0x20, inodeBitmapHi = 0x24, inodeTableHi = 0x28;
            constexpr size_t freeBlocksHi = 0x2C, freeInodesHi = 0x2E, usedDirsHi = 0x30;
        }

        namespace Ino {
            constexpr size_t mode = 0x00, sizeLo = 0x04, links = 0x1A, blocksLo = 0x1C, flags = 0x20;
            constexpr size_t block = 0x28, sizeHi = 0x6C, blocksHi = 0x74, extraIsize = 0x80;
        }

        constexpr uint32_t incFiletype = 0x2, incRecover = 0x4, incMetaBg = 0x10, incExtents = 0x40;
        constexpr uint32_t inc64bit = 0x80, incMmp = 0x100, incFlexBg = 0x200, incEaInode = 0x400;
        constexpr uint32_t incCsumSeed = 0x2000, incLargedir = 0x4000, incInline = 0x8000;
        constexpr uint32_t incEncrypt = 0x10000, incCasefold = 0x20000;
        constexpr uint32_t incReadSupported = incFiletype | incRecover | incMetaBg | incExtents | inc64bit |
                                              incMmp | incFlexBg | incEaInode | incCsumSeed | incLargedir |
                                              incInline | incEncrypt | incCasefold;
        constexpr uint32_t incWriteOk = incFiletype | incExtents | incMetaBg | inc64bit | incFlexBg;

        constexpr uint32_t compatImagic = 0x2, compatSparseSuper2 = 0x200, compatOrphanFile = 0x1000;
        constexpr uint32_t roSparseSuper = 0x1, roLargeFile = 0x2, roHugeFile = 0x8, roDirNlink = 0x20, roExtraIsize = 0x40;
        constexpr uint32_t roWriteOk = roSparseSuper | roLargeFile | roHugeFile | roDirNlink | roExtraIsize;

        constexpr uint32_t flagIndex = 0x1000, flagEncrypt = 0x800, flagHugeFile = 0x40000;
        constexpr uint32_t flagExtents = 0x80000, flagInline = 0x10000000;

        constexpr uint16_t modeTypeMask = 0xF000, modeDirectory = 0x4000, modeRegular = 0x8000;
        constexpr uint8_t ftRegular = 1, ftDirectory = 2;

        uint16_t rd16(const uint8_t* p, size_t o) { return static_cast<uint16_t>(p[o] | (p[o + 1] << 8)); }

        uint32_t rd32(const uint8_t* p, size_t o) {
            return static_cast<uint32_t>(p[o]) | (static_cast<uint32_t>(p[o + 1]) << 8) |
                   (static_cast<uint32_t>(p[o + 2]) << 16) | (static_cast<uint32_t>(p[o + 3]) << 24);
        }

        void wr16(uint8_t* p, size_t o, uint16_t v) {
            p[o] = static_cast<uint8_t>(v & 0xFF);
            p[o + 1] = static_cast<uint8_t>(v >> 8);
        }

        void wr32(uint8_t* p, size_t o, uint32_t v) {
            p[o] = static_cast<uint8_t>(v & 0xFF);
            p[o + 1] = static_cast<uint8_t>((v >> 8) & 0xFF);
            p[o + 2] = static_cast<uint8_t>((v >> 16) & 0xFF);
            p[o + 3] = static_cast<uint8_t>((v >> 24) & 0xFF);
        }

        template <typename T>
        T minOf(T a, T b) { return a < b ? a : b; }

        bool isPowerOfTwo(uint64_t v) { return v != 0 && (v & (v - 1)) == 0; }

        bool isPowerOf(uint64_t value, uint64_t base) {
            for (;;) {
                if (value < base) return false;
                if (value == base) return true;
                if (value % base != 0) return false;
                value /= base;
            }
        }

        bool bytesEqual(const char* a, const char* b, size_t n) {
            for (size_t i = 0; i < n; i++) if (a[i] != b[i]) return false;
            return true;
        }

        bool textEquals(std::string_view lhs, const char* rhs) {
            size_t i = 0;
            while (i < lhs.size() && rhs[i] != 0) {
                if (lhs[i] != rhs[i]) return false;
                i++;
            }
            return i == lhs.size() && rhs[i] == 0;
        }

        template <size_t N>
        void copyName(char (&dst)[N], const char* src, size_t length) {
            size_t n = length < N - 1 ? length : N - 1;
            for (size_t i = 0; i < n; i++) dst[i] = src[i];
            dst[n] = 0;
        }

        bool nextComponent(std::string_view path, size_t& pos, std::string_view& component) {
            while (pos < path.size() && path[pos] == '/') pos++;
            if (pos >= path.size()) return false;

            size_t start = pos;
            while (pos < path.size() && path[pos] != '/') pos++;

            component = path.substr(start, pos - start);
            return true;
        }

        bool hasMoreComponents(std::string_view path, size_t pos) {
            while (pos < path.size() && path[pos] == '/') pos++;
            return pos < path.size();
        }

        std::string_view lastComponent(std::string_view path) {
            size_t pos = 0;
            std::string_view component, last;
            while (nextComponent(path, pos, component)) last = component;
            return last;
        }

        bool validName(std::string_view name) {
            if (name.empty() || name.size() > maxNameLength) return false;
            for (char ch : name) if (ch == '/' || ch == 0) return false;
            return true;
        }

        bool makeName(std::string_view text, Name& name) {
            if (!validName(text)) return false;
            for (size_t i = 0; i < text.size(); i++) name.text[i] = text[i];
            name.text[text.size()] = 0;
            name.length = static_cast<uint32_t>(text.size());
            return true;
        }

        uint32_t entrySize(uint32_t nameLength) { return 8 + ((nameLength + 3) & ~3u); }

        uint32_t recLenFromDisk(uint32_t blockSize, uint32_t raw) {
            if (blockSize < 65536) return raw;
            if (raw == 65535 || raw == 0) return blockSize;
            return (raw & 65532) | ((raw & 3) << 16);
        }

        uint32_t recLenToDisk(uint32_t blockSize, uint32_t len) {
            if (blockSize < 65536) return len;
            if (len == blockSize) return blockSize == 65536 ? 65535 : 0;
            return (len & 65532) | ((len >> 16) & 3);
        }

        uint16_t inodeMode(const Inode& inode) { return rd16(inode.raw, Ino::mode); }
        bool isDirectoryInode(const Inode& inode) { return (inodeMode(inode) & modeTypeMask) == modeDirectory; }
        bool isRegularInode(const Inode& inode) { return (inodeMode(inode) & modeTypeMask) == modeRegular; }
        uint32_t inodeFlags(const Inode& inode) { return rd32(inode.raw, Ino::flags); }

        void setFileSize(Inode& inode, uint64_t size) {
            wr32(inode.raw, Ino::sizeLo, static_cast<uint32_t>(size & 0xFFFFFFFFu));
            wr32(inode.raw, Ino::sizeHi, static_cast<uint32_t>(size >> 32));
        }

        void adjustInodeBlocks(Inode& inode, int64_t delta512) {
            int64_t value = static_cast<int64_t>(rd32(inode.raw, Ino::blocksLo)) |
                            (static_cast<int64_t>(rd16(inode.raw, Ino::blocksHi)) << 32);
            value += delta512;
            if (value < 0) value = 0;
            wr32(inode.raw, Ino::blocksLo, static_cast<uint32_t>(value & 0xFFFFFFFF));
            wr16(inode.raw, Ino::blocksHi, static_cast<uint16_t>((value >> 32) & 0xFFFF));
        }

        void initExtentRoot(uint8_t* iblock) {
            memset(iblock, 0, 60);
            wr16(iblock, 0, kExtentMagic);
            wr16(iblock, 2, 0);
            wr16(iblock, 4, 4);
            wr16(iblock, 6, 0);
        }

        // Appends one block to a depth-0 extent tree stored in the inode (max 4 extents).
        Status appendExtent(Inode& inode, uint64_t logical, uint64_t phys) {
            uint8_t* root = inode.raw + Ino::block;
            if (rd16(root, 0) != kExtentMagic || rd16(root, 6) != 0) return kUnsupported;
            if (logical > 0xFFFFFFFFull || phys >= (1ull << 48)) return Status::ioError;

            uint32_t count = rd16(root, 2);
            if (count > 4) return Status::ioError;

            if (count > 0) {
                uint8_t* last = root + 12 + (count - 1) * 12;
                uint32_t lastLogical = rd32(last, 0);
                uint32_t lastLen = rd16(last, 4);
                if (lastLen > 32768) return kUnsupported;

                uint64_t lastStart = (static_cast<uint64_t>(rd16(last, 6)) << 32) | rd32(last, 8);
                if (logical < static_cast<uint64_t>(lastLogical) + lastLen) return kUnsupported;

                if (logical == static_cast<uint64_t>(lastLogical) + lastLen && phys == lastStart + lastLen && lastLen < 32768) {
                    wr16(last, 4, static_cast<uint16_t>(lastLen + 1));
                    return Status::ok;
                }
            }

            if (count == 4) return Status::noSpace;

            uint8_t* entry = root + 12 + count * 12;
            wr32(entry, 0, static_cast<uint32_t>(logical));
            wr16(entry, 4, 1);
            wr16(entry, 6, static_cast<uint16_t>(phys >> 32));
            wr32(entry, 8, static_cast<uint32_t>(phys & 0xFFFFFFFFu));
            wr16(root, 2, static_cast<uint16_t>(count + 1));

            return Status::ok;
        }

        uint64_t descBlockBitmap(const GroupDesc& gd) {
            uint64_t v = rd32(gd.raw, Gd::blockBitmapLo);
            if (gd.size >= 64) v |= static_cast<uint64_t>(rd32(gd.raw, Gd::blockBitmapHi)) << 32;
            return v;
        }

        uint64_t descInodeBitmap(const GroupDesc& gd) {
            uint64_t v = rd32(gd.raw, Gd::inodeBitmapLo);
            if (gd.size >= 64) v |= static_cast<uint64_t>(rd32(gd.raw, Gd::inodeBitmapHi)) << 32;
            return v;
        }

        uint64_t descInodeTable(const GroupDesc& gd) {
            uint64_t v = rd32(gd.raw, Gd::inodeTableLo);
            if (gd.size >= 64) v |= static_cast<uint64_t>(rd32(gd.raw, Gd::inodeTableHi)) << 32;
            return v;
        }

        uint32_t descFreeBlocks(const GroupDesc& gd) {
            uint32_t v = rd16(gd.raw, Gd::freeBlocksLo);
            if (gd.size >= 64) v |= static_cast<uint32_t>(rd16(gd.raw, Gd::freeBlocksHi)) << 16;
            return v;
        }

        uint32_t descFreeInodes(const GroupDesc& gd) {
            uint32_t v = rd16(gd.raw, Gd::freeInodesLo);
            if (gd.size >= 64) v |= static_cast<uint32_t>(rd16(gd.raw, Gd::freeInodesHi)) << 16;
            return v;
        }
    }

    // ---------------------------------------------------------------- device adapter
    // The only code that touches the block device. BlockdevDriver takes a ResourceID and a 32-bit LBA.

    bool Ext4Driver::devRead(uint64_t lba, void* buffer, size_t count) {
        if (!driver || lba > 0xFFFFFFFFull || count > 0xFFFFFFFFull || lba + count > 0x100000000ull) return false;

        return driver->read(resid_, static_cast<uint32_t>(lba), buffer, static_cast<uint32_t>(count));
    }

    bool Ext4Driver::devWrite(uint64_t lba, const void* buffer, size_t count) {
        if (!driver || lba > 0xFFFFFFFFull || count > 0xFFFFFFFFull || lba + count > 0x100000000ull) return false;

        return driver->write(resid_, static_cast<uint32_t>(lba), buffer, static_cast<uint32_t>(count));
    }

    uint64_t Ext4Driver::devSectors() {
        return driver ? driver->sectors(resid_) : 0;
    }

    bool Ext4Driver::readBytes(uint64_t offset, void* dst, size_t length) {
        if (length == 0) return true;
        if (volumeBytes_ != 0 && (offset > volumeBytes_ || length > volumeBytes_ - offset)) return false;

        uint64_t absolute = partitionOffset_ + offset;
        uint64_t deviceBytes = devSectors() * kSector;
        if (absolute > deviceBytes || length > deviceBytes - absolute) return false;

        auto* out = static_cast<uint8_t*>(dst);
        uint8_t bounce[kSector];

        while (length > 0) {
            uint64_t lba = absolute / kSector;
            size_t inSector = static_cast<size_t>(absolute % kSector);

            if (inSector == 0 && length >= kSector) {
                size_t sectors = length / kSector;
                if (!devRead(lba, out, sectors)) return false;

                out += sectors * kSector;
                absolute += sectors * kSector;
                length -= sectors * kSector;
            } else {
                if (!devRead(lba, bounce, 1)) return false;

                size_t count = minOf(kSector - inSector, length);
                memcpy(out, bounce + inSector, count);

                out += count;
                absolute += count;
                length -= count;
            }
        }

        return true;
    }

    bool Ext4Driver::writeBytes(uint64_t offset, const void* src, size_t length) {
        if (length == 0) return true;
        if (volumeBytes_ != 0 && (offset > volumeBytes_ || length > volumeBytes_ - offset)) return false;

        uint64_t absolute = partitionOffset_ + offset;
        uint64_t deviceBytes = devSectors() * kSector;
        if (absolute > deviceBytes || length > deviceBytes - absolute) return false;

        auto* in = static_cast<const uint8_t*>(src);
        uint8_t bounce[kSector];

        while (length > 0) {
            uint64_t lba = absolute / kSector;
            size_t inSector = static_cast<size_t>(absolute % kSector);

            if (inSector == 0 && length >= kSector) {
                size_t sectors = length / kSector;
                if (!devWrite(lba, in, sectors)) return false;

                in += sectors * kSector;
                absolute += sectors * kSector;
                length -= sectors * kSector;
            } else {
                if (!devRead(lba, bounce, 1)) return false;

                size_t count = minOf(kSector - inSector, length);
                memcpy(bounce + inSector, in, count);
                if (!devWrite(lba, bounce, 1)) return false;

                in += count;
                absolute += count;
                length -= count;
            }
        }

        return true;
    }

    bool Ext4Driver::zeroBytes(uint64_t offset, uint64_t length) {
        uint8_t zeros[kSector];
        memset(zeros, 0, sizeof(zeros));

        while (length > 0) {
            size_t count = static_cast<size_t>(minOf<uint64_t>(length, kSector));
            if (!writeBytes(offset, zeros, count)) return false;

            offset += count;
            length -= count;
        }

        return true;
    }

    // ---------------------------------------------------------------- mounting

    void Ext4Driver::resetState() {
        mounted_ = false;
        writable_ = false;
        dirty_ = false;
        partitionOffset_ = 0;
        volumeBytes_ = 0;
        blockCount_ = 0;
        blockSize_ = 0;
        firstDataBlock_ = 0;
        blocksPerGroup_ = 0;
        inodesPerGroup_ = 0;
        inodeCount_ = 0;
        inodeSize_ = 0;
        groupCount_ = 0;
        descSize_ = 0;
        descPerBlock_ = 0;
        firstMetaBg_ = 0;
        firstIno_ = 11;
        backupGroups_[0] = backupGroups_[1] = 0;
        featCompat_ = featIncompat_ = featRoCompat_ = 0;
        cwdInode_ = rootInode;
        cwdPath_[0] = '/';
        cwdPath_[1] = 0;
        cwdLength_ = 1;
    }

    Status Ext4Driver::mountAt(uint64_t partitionOffset, uint64_t partitionSize, bool& recognized) {
        resetState();
        partitionOffset_ = partitionOffset;

        uint8_t sb[kSbSize];
        if (!readBytes(kSbOffset, sb, kSbSize)) return Status::ioError;
        if (rd16(sb, Sb::magic) != kMagic) return Status::notFound;
        recognized = true;

        uint32_t logBlockSize = rd32(sb, Sb::logBlockSize);
        uint32_t revision = rd32(sb, Sb::revLevel);
        uint32_t incompat = rd32(sb, Sb::featIncompat);
        if (logBlockSize > 6 || revision > 1) return Status::ioError;
        if ((incompat & ~incReadSupported) != 0) return Status::ioError;

        blockSize_ = 1024u << logBlockSize;
        inodeCount_ = rd32(sb, Sb::inodeCount);
        firstDataBlock_ = rd32(sb, Sb::firstDataBlock);
        blocksPerGroup_ = rd32(sb, Sb::blocksPerGroup);
        inodesPerGroup_ = rd32(sb, Sb::inodesPerGroup);
        inodeSize_ = revision == 0 ? 128 : rd16(sb, Sb::inodeSize);
        firstIno_ = revision == 0 ? 11 : rd32(sb, Sb::firstIno);
        featCompat_ = revision == 0 ? 0 : rd32(sb, Sb::featCompat);
        featIncompat_ = revision == 0 ? 0 : incompat;
        featRoCompat_ = revision == 0 ? 0 : rd32(sb, Sb::featRoCompat);
        firstMetaBg_ = rd32(sb, Sb::firstMetaBg);
        backupGroups_[0] = rd32(sb, Sb::backupBgs);
        backupGroups_[1] = rd32(sb, Sb::backupBgs + 4);
        descSize_ = (featIncompat_ & inc64bit) != 0 ? rd16(sb, Sb::descSize) : 32;
        blockCount_ = rd32(sb, Sb::blocksLo);
        if ((featIncompat_ & inc64bit) != 0) blockCount_ |= static_cast<uint64_t>(rd32(sb, Sb::blocksHi)) << 32;
        uint16_t state = rd16(sb, Sb::state);

        if (blockCount_ == 0 || firstDataBlock_ >= blockCount_ || blocksPerGroup_ == 0 ||
            inodesPerGroup_ < 2 || inodesPerGroup_ > blockSize_ * 8 || inodeCount_ < 2 ||
            inodeSize_ < 128 || inodeSize_ > blockSize_ || !isPowerOfTwo(inodeSize_) ||
            descSize_ < 32 || descSize_ > 1024 || !isPowerOfTwo(descSize_) ||
            ((featIncompat_ & inc64bit) != 0 && descSize_ < 64) || firstIno_ < 11) {
            return Status::ioError;
        }

        descPerBlock_ = blockSize_ / descSize_;

        if (blockCount_ > 0xFFFFFFFFFFFFFFFFull / blockSize_) return Status::ioError;
        uint64_t volumeBytes = blockCount_ * blockSize_;
        uint64_t deviceBytes = devSectors() * kSector;
        if (partitionOffset >= deviceBytes || volumeBytes > deviceBytes - partitionOffset ||
            (partitionSize != 0 && volumeBytes > partitionSize)) {
            return Status::ioError;
        }

        uint64_t groups = (blockCount_ - firstDataBlock_ + blocksPerGroup_ - 1) / blocksPerGroup_;
        if (groups == 0 || groups > 0xFFFFFFFFull || static_cast<uint64_t>(inodeCount_) > groups * inodesPerGroup_) {
            return Status::ioError;
        }

        groupCount_ = static_cast<uint32_t>(groups);
        volumeBytes_ = volumeBytes;

        Inode root;
        Status status = readInode(rootInode, root);
        if (status != Status::ok) return status;
        if (!isDirectoryInode(root)) return Status::ioError;

        // Writes are enabled only where they cannot corrupt metadata we do not maintain:
        // no journal replay pending, no checksums, no quota, no encryption/inline data, etc.
        bool safe = (featIncompat_ & incExtents) != 0 &&
                    (featIncompat_ & ~incWriteOk) == 0 &&
                    (featRoCompat_ & ~roWriteOk) == 0 &&
                    (featCompat_ & (compatImagic | compatOrphanFile)) == 0 &&
                    (state & 0x2) == 0 &&
                    (!kRequireCleanForWrite || (state & 0x1) != 0);
        writable_ = safe;

        return Status::ok;
    }

    Status Ext4Driver::mountVolume() {
        bool recognized = false;
        Status status = mountAt(0, 0, recognized);

        if (status == Status::ok) {
            mounted_ = true;
            return Status::ok;
        }

        if (recognized) {
            resetState();
            return status;
        }

        resetState();

        uint8_t mbr[kSector];
        if (!readBytes(0, mbr, sizeof(mbr)) || mbr[510] != 0x55 || mbr[511] != 0xAA) return Status::notFound;

        uint64_t deviceBytes = devSectors() * kSector;
        Status last = Status::notFound;

        for (uint32_t index = 0; index < 4; index++) {
            const uint8_t* entry = mbr + 446 + index * 16;
            if (entry[4] != 0x83) continue;

            uint64_t start = static_cast<uint64_t>(rd32(entry, 8)) * kSector;
            uint64_t size = static_cast<uint64_t>(rd32(entry, 12)) * kSector;
            if (start >= deviceBytes || size == 0 || size > deviceBytes - start) continue;

            bool candidate = false;
            status = mountAt(start, size, candidate);
            if (status == Status::ok) {
                mounted_ = true;
                return Status::ok;
            }

            last = status;
        }

        resetState();
        return last;
    }

    bool Ext4Driver::initialize() {
        if (mounted_) return true;

        return mountVolume() == Status::ok;
    }

    void Ext4Driver::finalize() {
        if (!mounted_) return;

        if (dirty_) {
            uint8_t raw[2];
            if (readBytes(kSbOffset + Sb::state, raw, 2)) {
                wr16(raw, 0, static_cast<uint16_t>(rd16(raw, 0) | 0x1));
                writeBytes(kSbOffset + Sb::state, raw, 2);
            }
        }

        resetState();
    }

    std::string_view Ext4Driver::cwd() {
        return std::string_view(cwdPath_, cwdLength_);
    }

    // ---------------------------------------------------------------- geometry

    bool Ext4Driver::blockInRange(uint64_t block, uint64_t count) const {
        return block >= firstDataBlock_ && block < blockCount_ && count <= blockCount_ - block;
    }

    bool Ext4Driver::groupHasSuper(uint64_t group) const {
        if (group == 0) return true;
        if ((featCompat_ & compatSparseSuper2) != 0) return group == backupGroups_[0] || group == backupGroups_[1];
        if (group <= 1 || (featRoCompat_ & roSparseSuper) == 0) return true;
        if ((group & 1) == 0) return false;

        return isPowerOf(group, 3) || isPowerOf(group, 5) || isPowerOf(group, 7);
    }

    bool Ext4Driver::descriptorOffset(uint32_t group, uint64_t& offset) {
        uint32_t blockIndex = group / descPerBlock_;
        uint32_t index = group % descPerBlock_;
        uint64_t block;

        if ((featIncompat_ & incMetaBg) == 0 || blockIndex < firstMetaBg_) {
            block = static_cast<uint64_t>(firstDataBlock_) + blockIndex + 1;
        } else {
            uint64_t firstGroup = static_cast<uint64_t>(blockIndex) * descPerBlock_;
            uint64_t hasSuper = groupHasSuper(firstGroup) ? 1 : 0;
            if (blockSize_ == 1024 && blockIndex == 0 && firstDataBlock_ == 0) hasSuper++;

            block = static_cast<uint64_t>(firstDataBlock_) + firstGroup * blocksPerGroup_ + hasSuper;
        }

        if (!blockInRange(block)) return false;

        offset = block * blockSize_ + static_cast<uint64_t>(index) * descSize_;
        return true;
    }

    Status Ext4Driver::readGroupDesc(uint32_t group, GroupDesc& gd) {
        uint64_t offset = 0;
        if (group >= groupCount_ || !descriptorOffset(group, offset)) return Status::ioError;

        gd.size = descSize_ >= 64 ? 64 : 32;
        memset(gd.raw, 0, sizeof(gd.raw));

        return readBytes(offset, gd.raw, gd.size) ? Status::ok : Status::ioError;
    }

    Status Ext4Driver::writeGroupDesc(uint32_t group, const GroupDesc& gd) {
        uint64_t offset = 0;
        if (group >= groupCount_ || !descriptorOffset(group, offset)) return Status::ioError;

        return writeBytes(offset, gd.raw, gd.size) ? Status::ok : Status::ioError;
    }

    // ---------------------------------------------------------------- inodes

    bool Ext4Driver::inodeOffset(uint32_t number, uint64_t& offset) {
        if (number == 0 || number > inodeCount_) return false;

        uint32_t group = (number - 1) / inodesPerGroup_;
        uint32_t index = (number - 1) % inodesPerGroup_;

        GroupDesc gd;
        if (readGroupDesc(group, gd) != Status::ok) return false;

        uint64_t table = descInodeTable(gd);
        if (!blockInRange(table)) return false;

        uint64_t result = table * blockSize_ + static_cast<uint64_t>(index) * inodeSize_;
        if (result > volumeBytes_ || inodeSize_ > volumeBytes_ - result) return false;

        offset = result;
        return true;
    }

    Status Ext4Driver::readInode(uint32_t number, Inode& inode) {
        uint64_t offset = 0;
        if (!inodeOffset(number, offset)) return Status::ioError;

        memset(inode.raw, 0, sizeof(inode.raw));
        inode.number = number;
        inode.loaded = inodeSize_ < inodeBufferSize ? inodeSize_ : inodeBufferSize;

        return readBytes(offset, inode.raw, inode.loaded) ? Status::ok : Status::ioError;
    }

    Status Ext4Driver::writeInode(const Inode& inode) {
        uint64_t offset = 0;
        if (!inodeOffset(inode.number, offset)) return Status::ioError;

        return writeBytes(offset, inode.raw, inode.loaded) ? Status::ok : Status::ioError;
    }

    Status Ext4Driver::zeroInodeOnDisk(uint32_t number) {
        uint64_t offset = 0;
        if (!inodeOffset(number, offset)) return Status::ioError;

        return zeroBytes(offset, inodeSize_) ? Status::ok : Status::ioError;
    }

    uint64_t Ext4Driver::fileSize(const Inode& inode) const {
        uint64_t size = rd32(inode.raw, Ino::sizeLo);
        if ((featIncompat_ & incLargedir) != 0 || isRegularInode(inode)) {
            size |= static_cast<uint64_t>(rd32(inode.raw, Ino::sizeHi)) << 32;
        }

        return size;
    }

    // ---------------------------------------------------------------- block mapping

    bool Ext4Driver::readPointer(uint64_t block, uint64_t index, uint32_t& value) {
        uint8_t raw[4];
        if (!readBytes(block * blockSize_ + index * 4, raw, 4)) return false;

        value = rd32(raw, 0);
        return true;
    }

    Status Ext4Driver::mapLegacy(const Inode& inode, uint64_t logical, uint64_t& phys) {
        phys = 0;
        const uint64_t pointers = blockSize_ / 4;

        if (logical < 12) {
            phys = rd32(inode.raw, Ino::block + logical * 4);
        } else {
            logical -= 12;

            uint32_t levels = 0;
            uint64_t indices[3] = {0, 0, 0};

            if (logical < pointers) {
                levels = 1;
                indices[0] = logical;
            } else {
                logical -= pointers;

                if (logical < pointers * pointers) {
                    levels = 2;
                    indices[0] = logical / pointers;
                    indices[1] = logical % pointers;
                } else {
                    logical -= pointers * pointers;
                    if (logical >= pointers * pointers * pointers) return Status::ioError;

                    levels = 3;
                    indices[0] = logical / (pointers * pointers);
                    indices[1] = (logical / pointers) % pointers;
                    indices[2] = logical % pointers;
                }
            }

            // i_block[12] is the single, [13] the double, [14] the triple indirect pointer.
            uint32_t block = rd32(inode.raw, Ino::block + (11 + levels) * 4);

            for (uint32_t level = 0; level < levels; level++) {
                if (block == 0) return Status::ok;

                uint32_t next = 0;
                if (!blockInRange(block) || !readPointer(block, indices[level], next)) return Status::ioError;

                block = next;
            }

            phys = block;
        }

        if (phys != 0 && !blockInRange(phys)) return Status::ioError;

        return Status::ok;
    }

    Status Ext4Driver::mapExtent(const Inode& inode, uint64_t logical, uint64_t& phys, uint64_t& run, bool* unwritten) {
        phys = 0;
        run = 1;

        uint64_t nodeBlock = 0;
        int expectedDepth = -1;
        uint8_t header[12];
        memcpy(header, inode.raw + Ino::block, sizeof(header));

        for (uint32_t level = 0; level < 8; level++) {
            if (rd16(header, 0) != kExtentMagic) return Status::ioError;

            uint32_t entries = rd16(header, 2);
            uint32_t maximum = rd16(header, 4);
            uint32_t depth = rd16(header, 6);
            size_t capacity = nodeBlock == 0 ? 60 : blockSize_;

            if (depth > 5 || (expectedDepth >= 0 && depth != static_cast<uint32_t>(expectedDepth)) ||
                entries > maximum || 12 + static_cast<size_t>(entries) * 12 > capacity) {
                return Status::ioError;
            }

            auto fetch = [&](uint32_t index, uint8_t* entry) -> bool {
                if (nodeBlock == 0) {
                    memcpy(entry, inode.raw + Ino::block + 12 + static_cast<size_t>(index) * 12, 12);
                    return true;
                }

                return readBytes(nodeBlock * blockSize_ + 12 + static_cast<uint64_t>(index) * 12, entry, 12);
            };

            uint8_t entry[12];

            if (depth == 0) {
                for (uint32_t i = 0; i < entries; i++) {
                    if (!fetch(i, entry)) return Status::ioError;

                    uint64_t first = rd32(entry, 0);
                    uint32_t rawLength = rd16(entry, 4);
                    uint64_t length = rawLength > 32768 ? rawLength - 32768 : rawLength;
                    uint64_t start = (static_cast<uint64_t>(rd16(entry, 6)) << 32) | rd32(entry, 8);

                    if (length == 0 || !blockInRange(start, length)) return Status::ioError;

                    if (logical < first) {
                        run = first - logical;
                        return Status::ok;
                    }

                    if (logical < first + length) {
                        run = first + length - logical;

                        if (rawLength > 32768) {
                            if (unwritten) *unwritten = true;
                            return Status::ok;
                        }

                        phys = start + (logical - first);
                        return Status::ok;
                    }
                }

                run = kUnbounded;
                return Status::ok;
            }

            int64_t pick = -1;
            for (uint32_t i = 0; i < entries; i++) {
                if (!fetch(i, entry)) return Status::ioError;
                if (rd32(entry, 0) <= logical) pick = i; else break;
            }

            if (pick < 0) {
                run = kUnbounded;
                if (entries > 0 && fetch(0, entry)) run = rd32(entry, 0) - logical;
                return Status::ok;
            }

            if (!fetch(static_cast<uint32_t>(pick), entry)) return Status::ioError;

            uint64_t child = (static_cast<uint64_t>(rd16(entry, 8)) << 32) | rd32(entry, 4);
            if (!blockInRange(child) || !readBytes(child * blockSize_, header, sizeof(header))) return Status::ioError;

            nodeBlock = child;
            expectedDepth = static_cast<int>(depth) - 1;
        }

        return Status::ioError;
    }

    // phys == 0 means "hole or unwritten extent" (reads as zeros). run is the number of blocks, starting
    // at `logical`, that share this mapping (contiguous when phys != 0).
    Status Ext4Driver::mapBlock(const Inode& inode, uint64_t logical, uint64_t& phys, uint64_t& run, bool* unwritten) {
        phys = 0;
        run = 1;
        if (unwritten) *unwritten = false;

        if ((inodeFlags(inode) & flagExtents) != 0) return mapExtent(inode, logical, phys, run, unwritten);

        return mapLegacy(inode, logical, phys);
    }

    // ---------------------------------------------------------------- allocation

    Status Ext4Driver::markDirty() {
        if (!writable_) return kReadOnly;
        if (dirty_) return Status::ok;

        uint8_t raw[2];
        if (!readBytes(kSbOffset + Sb::state, raw, 2)) return Status::ioError;

        wr16(raw, 0, static_cast<uint16_t>(rd16(raw, 0) & ~0x1));
        if (!writeBytes(kSbOffset + Sb::state, raw, 2)) return Status::ioError;

        dirty_ = true;
        return Status::ok;
    }

    bool Ext4Driver::bitmapUpdate(uint64_t bitmapBlock, uint32_t first, uint32_t count, bool set) {
        uint8_t buffer[kSector];
        uint64_t base = bitmapBlock * blockSize_;

        while (count > 0) {
            uint32_t byteIndex = first / 8;
            uint32_t window = byteIndex - (byteIndex % kSector);

            if (!readBytes(base + window, buffer, kSector)) return false;

            while (count > 0 && first / 8 < window + kSector) {
                uint32_t local = first / 8 - window;
                uint8_t mask = static_cast<uint8_t>(1u << (first % 8));

                if (set) buffer[local] |= mask; else buffer[local] = static_cast<uint8_t>(buffer[local] & ~mask);

                first++;
                count--;
            }

            if (!writeBytes(base + window, buffer, kSector)) return false;
        }

        return true;
    }

    Status Ext4Driver::bitmapFindZero(uint64_t bitmapBlock, uint32_t from, uint32_t limit, uint32_t& bit, bool& found) {
        uint8_t buffer[kSector];
        uint64_t base = bitmapBlock * blockSize_;
        found = false;

        uint32_t current = from;
        while (current < limit) {
            uint32_t byteIndex = current / 8;
            uint32_t window = byteIndex - (byteIndex % kSector);

            if (!readBytes(base + window, buffer, kSector)) return Status::ioError;

            while (current < limit && current / 8 < window + kSector) {
                if ((buffer[current / 8 - window] & (1u << (current % 8))) == 0) {
                    bit = current;
                    found = true;
                    return Status::ok;
                }

                current++;
            }
        }

        return Status::ok;
    }

    Status Ext4Driver::adjustGroup(uint32_t group, int64_t blocks, int64_t inodes, int64_t dirs) {
        GroupDesc gd;
        Status status = readGroupDesc(group, gd);
        if (status != Status::ok) return status;

        bool big = gd.size >= 64;
        auto update = [&](size_t lo, size_t hi, int64_t delta) {
            if (delta == 0) return;

            int64_t value = rd16(gd.raw, lo);
            if (big) value |= static_cast<int64_t>(rd16(gd.raw, hi)) << 16;

            value += delta;
            if (value < 0) value = 0;

            wr16(gd.raw, lo, static_cast<uint16_t>(value & 0xFFFF));
            if (big) wr16(gd.raw, hi, static_cast<uint16_t>((value >> 16) & 0xFFFF));
        };

        update(Gd::freeBlocksLo, Gd::freeBlocksHi, blocks);
        update(Gd::freeInodesLo, Gd::freeInodesHi, inodes);
        update(Gd::usedDirsLo, Gd::usedDirsHi, dirs);

        return writeGroupDesc(group, gd);
    }

    Status Ext4Driver::adjustSuper(int64_t blocks, int64_t inodes) {
        uint8_t raw[4];

        if (blocks != 0) {
            bool big = (featIncompat_ & inc64bit) != 0;

            if (!readBytes(kSbOffset + Sb::freeBlocksLo, raw, 4)) return Status::ioError;
            int64_t value = rd32(raw, 0);

            uint8_t hiRaw[4] = {0, 0, 0, 0};
            if (big) {
                if (!readBytes(kSbOffset + Sb::freeBlocksHi, hiRaw, 4)) return Status::ioError;
                value |= static_cast<int64_t>(rd32(hiRaw, 0)) << 32;
            }

            value += blocks;
            if (value < 0) value = 0;

            wr32(raw, 0, static_cast<uint32_t>(value & 0xFFFFFFFFll));
            if (!writeBytes(kSbOffset + Sb::freeBlocksLo, raw, 4)) return Status::ioError;

            if (big) {
                wr32(hiRaw, 0, static_cast<uint32_t>((value >> 32) & 0xFFFFFFFFll));
                if (!writeBytes(kSbOffset + Sb::freeBlocksHi, hiRaw, 4)) return Status::ioError;
            }
        }

        if (inodes != 0) {
            if (!readBytes(kSbOffset + Sb::freeInodes, raw, 4)) return Status::ioError;

            int64_t value = static_cast<int64_t>(rd32(raw, 0)) + inodes;
            if (value < 0) value = 0;

            wr32(raw, 0, static_cast<uint32_t>(value));
            if (!writeBytes(kSbOffset + Sb::freeInodes, raw, 4)) return Status::ioError;
        }

        return Status::ok;
    }

    Status Ext4Driver::allocBlock(uint64_t goal, uint64_t& block) {
        uint32_t startGroup = 0;
        bool goalValid = goal >= firstDataBlock_ && goal < blockCount_;
        if (goalValid) startGroup = static_cast<uint32_t>((goal - firstDataBlock_) / blocksPerGroup_);

        for (uint32_t i = 0; i < groupCount_; i++) {
            uint32_t group = (startGroup + i) % groupCount_;

            GroupDesc gd;
            Status status = readGroupDesc(group, gd);
            if (status != Status::ok) return status;
            if (descFreeBlocks(gd) == 0) continue;

            uint64_t bitmap = descBlockBitmap(gd);
            if (!blockInRange(bitmap)) return Status::ioError;

            uint64_t groupStart = static_cast<uint64_t>(firstDataBlock_) + static_cast<uint64_t>(group) * blocksPerGroup_;
            uint64_t groupBlocks = minOf<uint64_t>(blocksPerGroup_, blockCount_ - groupStart);
            uint32_t from = (i == 0 && goalValid) ? static_cast<uint32_t>(goal - groupStart) : 0;

            uint32_t bit = 0;
            bool found = false;

            status = bitmapFindZero(bitmap, from, static_cast<uint32_t>(groupBlocks), bit, found);
            if (status != Status::ok) return status;

            if (!found && from > 0) {
                status = bitmapFindZero(bitmap, 0, from, bit, found);
                if (status != Status::ok) return status;
            }

            if (!found) continue;

            if (!bitmapUpdate(bitmap, bit, 1, true)) return Status::ioError;

            status = adjustGroup(group, -1, 0, 0);
            if (status != Status::ok) return status;

            status = adjustSuper(-1, 0);
            if (status != Status::ok) return status;

            block = groupStart + bit;
            return Status::ok;
        }

        return Status::noSpace;
    }

    Status Ext4Driver::allocInode(uint32_t parent, bool directory, uint32_t& number) {
        uint32_t startGroup = parent > 0 ? (parent - 1) / inodesPerGroup_ : 0;
        if (startGroup >= groupCount_) startGroup = 0;

        for (uint32_t i = 0; i < groupCount_; i++) {
            uint32_t group = (startGroup + i) % groupCount_;

            GroupDesc gd;
            Status status = readGroupDesc(group, gd);
            if (status != Status::ok) return status;
            if (descFreeInodes(gd) == 0) continue;

            uint64_t bitmap = descInodeBitmap(gd);
            if (!blockInRange(bitmap)) return Status::ioError;

            uint32_t from = group == 0 ? firstIno_ - 1 : 0;
            uint32_t bit = 0;
            bool found = false;

            status = bitmapFindZero(bitmap, from, inodesPerGroup_, bit, found);
            if (status != Status::ok) return status;
            if (!found) continue;

            uint64_t candidate = static_cast<uint64_t>(group) * inodesPerGroup_ + bit + 1;
            if (candidate > inodeCount_) continue;

            if (!bitmapUpdate(bitmap, bit, 1, true)) return Status::ioError;

            status = adjustGroup(group, 0, -1, directory ? 1 : 0);
            if (status != Status::ok) return status;

            status = adjustSuper(0, -1);
            if (status != Status::ok) return status;

            number = static_cast<uint32_t>(candidate);
            return Status::ok;
        }

        return Status::noSpace;
    }

    Status Ext4Driver::freeRun(uint64_t start, uint64_t count) {
        while (count > 0) {
            if (!blockInRange(start)) return Status::ioError;

            uint32_t group = static_cast<uint32_t>((start - firstDataBlock_) / blocksPerGroup_);
            uint64_t groupStart = static_cast<uint64_t>(firstDataBlock_) + static_cast<uint64_t>(group) * blocksPerGroup_;
            uint64_t inGroup = minOf<uint64_t>(count, groupStart + blocksPerGroup_ - start);
            if (start + inGroup > blockCount_) return Status::ioError;

            GroupDesc gd;
            Status status = readGroupDesc(group, gd);
            if (status != Status::ok) return status;

            uint64_t bitmap = descBlockBitmap(gd);
            if (!blockInRange(bitmap)) return Status::ioError;

            if (!bitmapUpdate(bitmap, static_cast<uint32_t>(start - groupStart), static_cast<uint32_t>(inGroup), false)) {
                return Status::ioError;
            }

            status = adjustGroup(group, static_cast<int64_t>(inGroup), 0, 0);
            if (status != Status::ok) return status;

            status = adjustSuper(static_cast<int64_t>(inGroup), 0);
            if (status != Status::ok) return status;

            start += inGroup;
            count -= inGroup;
        }

        return Status::ok;
    }

    Status Ext4Driver::freeInode(uint32_t number, bool directory) {
        if (number < firstIno_ || number > inodeCount_) return Status::ioError;

        uint32_t group = (number - 1) / inodesPerGroup_;
        uint32_t bit = (number - 1) % inodesPerGroup_;

        GroupDesc gd;
        Status status = readGroupDesc(group, gd);
        if (status != Status::ok) return status;

        uint64_t bitmap = descInodeBitmap(gd);
        if (!blockInRange(bitmap)) return Status::ioError;

        status = zeroInodeOnDisk(number);
        if (status != Status::ok) return status;

        if (!bitmapUpdate(bitmap, bit, 1, false)) return Status::ioError;

        status = adjustGroup(group, 0, 1, directory ? -1 : 0);
        if (status != Status::ok) return status;

        return adjustSuper(0, 1);
    }

    Status Ext4Driver::freeLegacyTree(uint64_t block, uint32_t level) {
        if (!blockInRange(block)) return Status::ioError;

        const uint32_t pointers = blockSize_ / 4;
        const uint32_t perWindow = kSector / 4;
        uint8_t buffer[kSector];

        for (uint32_t i = 0; i < pointers; i += perWindow) {
            if (!readBytes(block * blockSize_ + static_cast<uint64_t>(i) * 4, buffer, sizeof(buffer))) return Status::ioError;

            for (uint32_t j = 0; j < perWindow; j++) {
                uint32_t pointer = rd32(buffer, j * 4);
                if (pointer == 0) continue;

                Status status = level == 1 ? freeRun(pointer, 1) : freeLegacyTree(pointer, level - 1);
                if (status != Status::ok) return status;
            }
        }

        return freeRun(block, 1);
    }

    Status Ext4Driver::freeExtentNode(const Inode* inode, uint64_t nodeBlock, uint32_t guard) {
        if (guard == 0) return Status::ioError;

        uint8_t header[12];
        if (nodeBlock == 0) {
            memcpy(header, inode->raw + Ino::block, sizeof(header));
        } else if (!blockInRange(nodeBlock) || !readBytes(nodeBlock * blockSize_, header, sizeof(header))) {
            return Status::ioError;
        }

        if (rd16(header, 0) != kExtentMagic) return Status::ioError;

        uint32_t entries = rd16(header, 2);
        uint32_t maximum = rd16(header, 4);
        uint32_t depth = rd16(header, 6);
        size_t capacity = nodeBlock == 0 ? 60 : blockSize_;

        if (depth > 5 || entries > maximum || 12 + static_cast<size_t>(entries) * 12 > capacity) return Status::ioError;

        for (uint32_t i = 0; i < entries; i++) {
            uint8_t entry[12];

            if (nodeBlock == 0) {
                memcpy(entry, inode->raw + Ino::block + 12 + static_cast<size_t>(i) * 12, 12);
            } else if (!readBytes(nodeBlock * blockSize_ + 12 + static_cast<uint64_t>(i) * 12, entry, 12)) {
                return Status::ioError;
            }

            Status status;
            if (depth == 0) {
                uint32_t rawLength = rd16(entry, 4);
                uint64_t length = rawLength > 32768 ? rawLength - 32768 : rawLength;
                uint64_t start = (static_cast<uint64_t>(rd16(entry, 6)) << 32) | rd32(entry, 8);

                if (length == 0 || !blockInRange(start, length)) return Status::ioError;

                status = freeRun(start, length);
            } else {
                uint64_t child = (static_cast<uint64_t>(rd16(entry, 8)) << 32) | rd32(entry, 4);

                status = freeExtentNode(nullptr, child, guard - 1);
                if (status == Status::ok) status = freeRun(child, 1);
            }

            if (status != Status::ok) return status;
        }

        return Status::ok;
    }

    Status Ext4Driver::freeFileBlocks(const Inode& inode) {
        if ((inodeFlags(inode) & flagExtents) != 0) return freeExtentNode(&inode, 0, 8);

        for (uint32_t i = 0; i < 12; i++) {
            uint32_t pointer = rd32(inode.raw, Ino::block + i * 4);
            if (pointer == 0) continue;

            Status status = freeRun(pointer, 1);
            if (status != Status::ok) return status;
        }

        for (uint32_t level = 1; level <= 3; level++) {
            uint32_t pointer = rd32(inode.raw, Ino::block + (11 + level) * 4);
            if (pointer == 0) continue;

            Status status = freeLegacyTree(pointer, level);
            if (status != Status::ok) return status;
        }

        return Status::ok;
    }

    // Attaches one more block to the file's extent tree. Supports a depth-0 root (up to 4 extents) and,
    // once that is full, converts it to a depth-1 tree (up to 4 leaf blocks, each holding hundreds of extents).
    Status Ext4Driver::appendBlock(Inode& inode, uint64_t logical, uint64_t phys) {
        uint8_t* root = inode.raw + Ino::block;
        if (rd16(root, 0) != kExtentMagic) return Status::ioError;
        if (logical > 0xFFFFFFFFull || phys >= (1ull << 48)) return Status::ioError;

        uint32_t depth = rd16(root, 6);

        if (depth == 0) {
            Status status = appendExtent(inode, logical, phys);
            if (status != Status::noSpace) return status;

            // The root is full: push its extents down into a freshly allocated leaf block.
            uint64_t leaf = 0;
            status = allocBlock(phys + 1, leaf);
            if (status != Status::ok) return status;

            uint32_t entries = rd16(root, 2);
            uint8_t node[12 + 4 * 12];
            memset(node, 0, sizeof(node));
            wr16(node, 0, kExtentMagic);
            wr16(node, 2, static_cast<uint16_t>(entries));
            wr16(node, 4, static_cast<uint16_t>((blockSize_ - 12) / 12));
            memcpy(node + 12, root + 12, entries * 12);

            if (!zeroBytes(leaf * blockSize_, blockSize_) || !writeBytes(leaf * blockSize_, node, 12 + entries * 12)) {
                freeRun(leaf, 1);
                return Status::ioError;
            }

            uint32_t firstLogical = rd32(root, 12);
            memset(root + 12, 0, 4 * 12);
            wr16(root, 2, 1);
            wr16(root, 6, 1);
            wr32(root, 12 + 0, firstLogical);
            wr32(root, 12 + 4, static_cast<uint32_t>(leaf & 0xFFFFFFFFu));
            wr16(root, 12 + 8, static_cast<uint16_t>(leaf >> 32));

            adjustInodeBlocks(inode, blockSize_ / 512);
            depth = 1;
        }

        if (depth != 1) return kUnsupported;

        uint32_t indexCount = rd16(root, 2);
        if (indexCount == 0 || indexCount > 4) return Status::ioError;

        const uint8_t* lastIndex = root + 12 + (indexCount - 1) * 12;
        uint64_t leaf = (static_cast<uint64_t>(rd16(lastIndex, 8)) << 32) | rd32(lastIndex, 4);
        if (!blockInRange(leaf)) return Status::ioError;

        uint8_t header[12];
        if (!readBytes(leaf * blockSize_, header, sizeof(header))) return Status::ioError;
        if (rd16(header, 0) != kExtentMagic || rd16(header, 6) != 0) return Status::ioError;

        uint32_t count = rd16(header, 2);
        uint32_t maximum = rd16(header, 4);
        if (count > maximum || 12 + static_cast<uint64_t>(count) * 12 > blockSize_) return Status::ioError;

        uint64_t base = leaf * blockSize_;
        uint8_t entry[12];

        if (count > 0) {
            uint64_t lastOffset = base + 12 + static_cast<uint64_t>(count - 1) * 12;
            if (!readBytes(lastOffset, entry, sizeof(entry))) return Status::ioError;

            uint32_t lastLogical = rd32(entry, 0);
            uint32_t lastLength = rd16(entry, 4);
            uint64_t lastStart = (static_cast<uint64_t>(rd16(entry, 6)) << 32) | rd32(entry, 8);

            if (lastLength > 32768) return kUnsupported;
            if (logical < static_cast<uint64_t>(lastLogical) + lastLength) return kUnsupported;

            if (logical == static_cast<uint64_t>(lastLogical) + lastLength && phys == lastStart + lastLength && lastLength < 32768) {
                wr16(entry, 4, static_cast<uint16_t>(lastLength + 1));
                return writeBytes(lastOffset + 4, entry + 4, 2) ? Status::ok : Status::ioError;
            }
        }

        memset(entry, 0, sizeof(entry));
        wr32(entry, 0, static_cast<uint32_t>(logical));
        wr16(entry, 4, 1);
        wr16(entry, 6, static_cast<uint16_t>(phys >> 32));
        wr32(entry, 8, static_cast<uint32_t>(phys & 0xFFFFFFFFu));

        if (count < maximum) {
            if (!writeBytes(base + 12 + static_cast<uint64_t>(count) * 12, entry, sizeof(entry))) return Status::ioError;

            wr16(header, 2, static_cast<uint16_t>(count + 1));
            return writeBytes(base + 2, header + 2, 2) ? Status::ok : Status::ioError;
        }

        // The last leaf is full: start a new one if the root still has a free index slot.
        if (indexCount >= 4) return Status::noSpace;

        uint64_t fresh = 0;
        Status status = allocBlock(phys + 1, fresh);
        if (status != Status::ok) return status;

        uint8_t node[12 + 12];
        memset(node, 0, sizeof(node));
        wr16(node, 0, kExtentMagic);
        wr16(node, 2, 1);
        wr16(node, 4, static_cast<uint16_t>((blockSize_ - 12) / 12));
        memcpy(node + 12, entry, 12);

        if (!zeroBytes(fresh * blockSize_, blockSize_) || !writeBytes(fresh * blockSize_, node, sizeof(node))) {
            freeRun(fresh, 1);
            return Status::ioError;
        }

        uint8_t* slot = root + 12 + indexCount * 12;
        memset(slot, 0, 12);
        wr32(slot, 0, static_cast<uint32_t>(logical));
        wr32(slot, 4, static_cast<uint32_t>(fresh & 0xFFFFFFFFu));
        wr16(slot, 8, static_cast<uint16_t>(fresh >> 32));
        wr16(root, 2, static_cast<uint16_t>(indexCount + 1));

        adjustInodeBlocks(inode, blockSize_ / 512);
        return Status::ok;
    }

    // ---------------------------------------------------------------- directories

    Status Ext4Driver::dirOpen(uint32_t number, DirIter& it) {
        Status status = readInode(number, it.dir);
        if (status != Status::ok) return status;
        if (!isDirectoryInode(it.dir)) return Status::notDirectory;

        if ((inodeFlags(it.dir) & (flagInline | flagEncrypt)) != 0) return kUnsupported;

        uint64_t size = fileSize(it.dir);
        if (size == 0) return Status::ioError;

        it.blocks = (size + blockSize_ - 1) / blockSize_;
        it.logical = 0;
        it.phys = 0;
        it.pos = 0;
        it.prevPos = noOffset;
        it.open = false;

        return Status::ok;
    }

    // Returns every record, including deleted ones (inode == 0), so insertion can reuse the space.
    Status Ext4Driver::dirNext(DirIter& it, DirEntry& entry, bool& end) {
        end = false;

        for (;;) {
            if (!it.open) {
                if (it.logical >= it.blocks) {
                    end = true;
                    return Status::ok;
                }

                uint64_t run = 0;
                Status status = mapBlock(it.dir, it.logical, it.phys, run, nullptr);
                if (status != Status::ok) return status;
                if (it.phys == 0) return Status::ioError;

                it.pos = 0;
                it.prevPos = noOffset;
                it.open = true;
            }

            if (it.pos + 8 > blockSize_) {
                if (it.pos != blockSize_) return Status::ioError;

                it.open = false;
                it.logical++;
                continue;
            }

            uint8_t header[8];
            uint64_t base = it.phys * blockSize_;
            if (!readBytes(base + it.pos, header, sizeof(header))) return Status::ioError;

            uint32_t inode = rd32(header, 0);
            uint32_t recLen = recLenFromDisk(blockSize_, rd16(header, 4));
            uint32_t nameLen = header[6];

            if (recLen < 8 || (recLen & 3) != 0 || recLen > blockSize_ - it.pos ||
                (inode != 0 && 8 + nameLen > recLen)) {
                return Status::ioError;
            }

            entry.inode = inode;
            entry.recLen = recLen;
            entry.offset = it.pos;
            entry.prevOffset = it.prevPos;
            entry.block = it.phys;
            entry.nameLen = static_cast<uint8_t>(nameLen);
            entry.type = header[7];
            entry.name[0] = 0;

            if (inode != 0 && nameLen > 0) {
                if (!readBytes(base + it.pos + 8, entry.name, nameLen)) return Status::ioError;
                entry.name[nameLen] = 0;
            }

            it.prevPos = it.pos;
            it.pos += recLen;

            return Status::ok;
        }
    }

    Status Ext4Driver::findEntry(uint32_t dir, const Name& name, DirEntry& entry) {
        DirIter it;
        Status status = dirOpen(dir, it);
        if (status != Status::ok) return status;

        bool end = false;
        for (;;) {
            status = dirNext(it, entry, end);
            if (status != Status::ok) return status;
            if (end) return Status::notFound;

            if (entry.inode != 0 && entry.nameLen == name.length && bytesEqual(entry.name, name.text, name.length)) {
                return Status::ok;
            }
        }
    }

    Status Ext4Driver::lookup(uint32_t dir, std::string_view name, uint32_t& number) {
        Name key;
        if (!makeName(name, key)) return Status::invalidName;

        DirEntry entry;
        Status status = findEntry(dir, key, entry);
        if (status != Status::ok) return status;

        number = entry.inode;
        return Status::ok;
    }

    Status Ext4Driver::dirIsEmpty(uint32_t dir, bool& empty) {
        DirIter it;
        Status status = dirOpen(dir, it);
        if (status != Status::ok) return status;

        DirEntry entry;
        bool end = false;
        empty = true;

        for (;;) {
            status = dirNext(it, entry, end);
            if (status != Status::ok) return status;
            if (end) return Status::ok;
            if (entry.inode == 0) continue;

            std::string_view text(entry.name, entry.nameLen);
            if (textEquals(text, ".") || textEquals(text, "..")) continue;

            empty = false;
            return Status::ok;
        }
    }

    bool Ext4Driver::writeEntryAt(uint64_t block, uint32_t offset, uint32_t inode, uint32_t recLen, const Name& name, uint8_t type) {
        uint8_t buffer[8 + maxNameLength];

        wr32(buffer, 0, inode);
        wr16(buffer, 4, static_cast<uint16_t>(recLenToDisk(blockSize_, recLen)));
        buffer[6] = static_cast<uint8_t>(name.length);
        buffer[7] = type;
        memcpy(buffer + 8, name.text, name.length);

        return writeBytes(block * blockSize_ + offset, buffer, 8 + name.length);
    }

    Status Ext4Driver::insertEntry(uint32_t parentNumber, const Name& name, uint32_t child, uint8_t type) {
        Inode parent;
        Status status = readInode(parentNumber, parent);
        if (status != Status::ok) return status;
        if (!isDirectoryInode(parent)) return Status::notDirectory;

        uint32_t flags = inodeFlags(parent);
        if ((flags & (flagInline | flagEncrypt)) != 0) return kUnsupported;

        const uint32_t needed = entrySize(name.length);
        const uint8_t fileType = (featIncompat_ & incFiletype) != 0 ? type : 0;

        // We insert without maintaining the htree, so demote the directory to a plain linear one.
        if ((flags & flagIndex) != 0) {
            wr32(parent.raw, Ino::flags, flags & ~flagIndex);

            status = writeInode(parent);
            if (status != Status::ok) return status;
        }

        DirIter it;
        status = dirOpen(parentNumber, it);
        if (status != Status::ok) return status;

        DirEntry entry;
        bool end = false;

        for (;;) {
            status = dirNext(it, entry, end);
            if (status != Status::ok) return status;
            if (end) break;

            if (entry.inode == 0) {
                if (entry.recLen >= needed) {
                    return writeEntryAt(entry.block, entry.offset, child, entry.recLen, name, fileType) ? Status::ok : Status::ioError;
                }

                continue;
            }

            uint32_t used = entrySize(entry.nameLen);
            if (entry.recLen < used + needed) continue;

            if (!writeEntryAt(entry.block, entry.offset + used, child, entry.recLen - used, name, fileType)) return Status::ioError;

            uint8_t raw[2];
            wr16(raw, 0, static_cast<uint16_t>(recLenToDisk(blockSize_, used)));

            return writeBytes(entry.block * blockSize_ + entry.offset + 4, raw, 2) ? Status::ok : Status::ioError;
        }

        // No room in the existing blocks: append a fresh directory block.
        if ((flags & flagExtents) == 0) return Status::noSpace;

        uint64_t size = fileSize(parent);
        if (size % blockSize_ != 0) return Status::ioError;

        uint64_t logical = size / blockSize_;
        uint64_t goal = firstDataBlock_;
        if (logical > 0) {
            uint64_t previous = 0, run = 0;
            if (mapBlock(parent, logical - 1, previous, run, nullptr) == Status::ok && previous != 0) goal = previous + 1;
        }

        uint64_t block = 0;
        status = allocBlock(goal, block);
        if (status != Status::ok) return status;

        if (!zeroBytes(block * blockSize_, blockSize_) ||
            !writeEntryAt(block, 0, child, blockSize_, name, fileType)) {
            freeRun(block, 1);
            return Status::ioError;
        }

        status = appendBlock(parent, logical, block);
        if (status != Status::ok) {
            freeRun(block, 1);
            return status;
        }

        setFileSize(parent, size + blockSize_);
        adjustInodeBlocks(parent, blockSize_ / 512);

        return writeInode(parent);
    }

    Status Ext4Driver::removeEntry(const DirEntry& entry) {
        uint64_t base = entry.block * blockSize_;

        if (entry.prevOffset == noOffset) {
            uint8_t zero[4] = {0, 0, 0, 0};
            return writeBytes(base + entry.offset, zero, 4) ? Status::ok : Status::ioError;
        }

        uint8_t raw[2];
        if (!readBytes(base + entry.prevOffset + 4, raw, 2)) return Status::ioError;

        uint32_t previous = recLenFromDisk(blockSize_, rd16(raw, 0));
        wr16(raw, 0, static_cast<uint16_t>(recLenToDisk(blockSize_, previous + entry.recLen)));

        return writeBytes(base + entry.prevOffset + 4, raw, 2) ? Status::ok : Status::ioError;
    }

    // ---------------------------------------------------------------- paths

    Status Ext4Driver::resolve(std::string_view path, uint32_t& number) {
        if (!mounted_) return Status::notReady;

        uint32_t current = (!path.empty() && path[0] == '/') ? rootInode : cwdInode_;
        size_t pos = 0;
        std::string_view component;

        while (nextComponent(path, pos, component)) {
            if (textEquals(component, ".")) continue;

            uint32_t next = 0;
            Status status = lookup(current, component, next);
            if (status != Status::ok) return status;

            current = next;
        }

        number = current;
        return Status::ok;
    }

    Status Ext4Driver::resolveParent(std::string_view path, uint32_t& parent, Name& leaf) {
        if (!mounted_) return Status::notReady;

        uint32_t current = (!path.empty() && path[0] == '/') ? rootInode : cwdInode_;
        size_t pos = 0;
        std::string_view component;

        while (nextComponent(path, pos, component)) {
            bool last = !hasMoreComponents(path, pos);

            if (last) {
                if (textEquals(component, ".") || textEquals(component, "..")) return Status::invalidName;
                if (!makeName(component, leaf)) return Status::invalidName;

                parent = current;
                return Status::ok;
            }

            if (textEquals(component, ".")) continue;

            uint32_t next = 0;
            Status status = lookup(current, component, next);
            if (status != Status::ok) return status;

            current = next;
        }

        return Status::invalidName;
    }

    Status Ext4Driver::changeDirectory(std::string_view path) {
        if (!mounted_) return Status::notReady;
        if (path.empty()) path = "/";

        uint32_t number = 0;
        Status status = resolve(path, number);
        if (status != Status::ok) return status;

        Inode inode;
        status = readInode(number, inode);
        if (status != Status::ok) return status;
        if (!isDirectoryInode(inode)) return Status::notDirectory;

        char next[maxPathLength];
        size_t length;

        if (path[0] == '/') {
            next[0] = '/';
            next[1] = 0;
            length = 1;
        } else {
            memcpy(next, cwdPath_, cwdLength_ + 1);
            length = cwdLength_;
        }

        size_t pos = 0;
        std::string_view component;

        while (nextComponent(path, pos, component)) {
            if (textEquals(component, ".")) continue;

            if (textEquals(component, "..")) {
                while (length > 1 && next[length - 1] != '/') length--;
                if (length > 1) length--;

                next[length] = 0;
                continue;
            }

            size_t needed = length + (length > 1 ? 1 : 0) + component.size();
            if (needed + 1 > maxPathLength) return Status::invalidName;

            if (length > 1) next[length++] = '/';
            for (char ch : component) next[length++] = ch;
            next[length] = 0;
        }

        cwdInode_ = number;
        memcpy(cwdPath_, next, length + 1);
        cwdLength_ = static_cast<uint32_t>(length);

        return Status::ok;
    }

    // ---------------------------------------------------------------- reading

    Status Ext4Driver::fillInfo(uint32_t number, const char* name, size_t nameLength, EntryInfo& info) {
        Inode inode;
        Status status = readInode(number, inode);
        if (status != Status::ok) return status;

        copyName(info.name, name, nameLength);

        bool directory = isDirectoryInode(inode);
        info.directory = directory;
        uint64_t size = directory ? 0 : fileSize(inode);
        if (size > 0xFFFFFFFFull) size = 0xFFFFFFFFull;   // EntryInfo::size is 32-bit
        info.size = static_cast<decltype(info.size)>(size);
        info.cluster = static_cast<decltype(info.cluster)>(number);

        return Status::ok;
    }

    Status Ext4Driver::listDirectory(std::string_view path, ListCallback callback, void* context) {
        if (!callback) return Status::ioError;

        std::string_view target = path.empty() ? std::string_view(".") : path;

        uint32_t number = 0;
        Status status = resolve(target, number);
        if (status != Status::ok) return status;

        Inode inode;
        status = readInode(number, inode);
        if (status != Status::ok) return status;

        if (!isDirectoryInode(inode)) {
            std::string_view leaf = lastComponent(target);

            EntryInfo info;
            status = fillInfo(number, leaf.data(), leaf.size(), info);
            if (status != Status::ok) return status;

            callback(info, context);
            return Status::ok;
        }

        DirIter it;
        status = dirOpen(number, it);
        if (status != Status::ok) return status;

        DirEntry entry;
        bool end = false;

        for (;;) {
            status = dirNext(it, entry, end);
            if (status != Status::ok) return status;
            if (end) return Status::ok;
            if (entry.inode == 0 || entry.nameLen == 0) continue;

            std::string_view text(entry.name, entry.nameLen);
            if (textEquals(text, ".") || textEquals(text, "..")) continue;

            EntryInfo info;
            status = fillInfo(entry.inode, entry.name, entry.nameLen, info);
            if (status != Status::ok) return status;

            callback(info, context);
        }
    }

    Status Ext4Driver::readFile(std::string_view path, ChunkCallback callback, void* context) {
        if (!callback) return Status::ioError;

        uint32_t number = 0;
        Status status = resolve(path, number);
        if (status != Status::ok) return status;

        Inode inode;
        status = readInode(number, inode);
        if (status != Status::ok) return status;
        if (isDirectoryInode(inode)) return Status::isDirectory;
        if (!isRegularInode(inode)) return Status::ioError;
        if ((inodeFlags(inode) & (flagInline | flagEncrypt)) != 0) return kUnsupported;

        uint64_t size = fileSize(inode);
        uint64_t position = 0;
        uint8_t buffer[kChunk];

        while (position < size) {
            uint64_t logical = position / blockSize_;
            uint32_t inBlock = static_cast<uint32_t>(position % blockSize_);
            uint64_t phys = 0, run = 0;

            status = mapBlock(inode, logical, phys, run, nullptr);
            if (status != Status::ok) return status;

            uint64_t available = run * blockSize_ - inBlock;
            size_t count = static_cast<size_t>(minOf<uint64_t>(minOf<uint64_t>(size - position, available), sizeof(buffer)));

            if (phys == 0) {
                memset(buffer, 0, count);
            } else if (!readBytes(phys * blockSize_ + inBlock, buffer, count)) {
                return Status::ioError;
            }

            callback(std::string_view(reinterpret_cast<const char*>(buffer), count), context);
            position += count;
        }

        return Status::ok;
    }

    Status Ext4Driver::stat(std::string_view path, EntryInfo& info) {
        std::string_view target = path.empty() ? std::string_view(".") : path;

        uint32_t number = 0;
        Status status = resolve(target, number);
        if (status != Status::ok) return status;

        std::string_view leaf = lastComponent(target);
        if (leaf.empty() || textEquals(leaf, ".") || textEquals(leaf, "..")) {
            const char* synthetic = number == rootInode ? "/" : ".";
            return fillInfo(number, synthetic, 1, info);
        }

        return fillInfo(number, leaf.data(), leaf.size(), info);
    }

    // ---------------------------------------------------------------- writing

    Status Ext4Driver::adjustLinks(uint32_t number, int32_t delta) {
        Inode inode;
        Status status = readInode(number, inode);
        if (status != Status::ok) return status;

        int32_t links = static_cast<int32_t>(rd16(inode.raw, Ino::links)) + delta;
        if (links < 0) links = 0;
        if (links > 65000) links = 65000;

        wr16(inode.raw, Ino::links, static_cast<uint16_t>(links));
        return writeInode(inode);
    }

    Status Ext4Driver::makeNode(uint32_t parent, const Name& name, bool directory, uint32_t& number) {
        if (!writable_) return kReadOnly;

        Status status = markDirty();
        if (status != Status::ok) return status;

        DirEntry existing;
        status = findEntry(parent, name, existing);
        if (status == Status::ok) return Status::alreadyExists;
        if (status != Status::notFound) return status;

        uint32_t created = 0;
        status = allocInode(parent, directory, created);
        if (status != Status::ok) return status;

        status = zeroInodeOnDisk(created);
        if (status != Status::ok) {
            freeInode(created, directory);
            return status;
        }

        Inode inode;
        memset(inode.raw, 0, sizeof(inode.raw));
        inode.number = created;
        inode.loaded = inodeSize_ < inodeBufferSize ? inodeSize_ : inodeBufferSize;

        wr16(inode.raw, Ino::mode, directory ? 0x41ED : 0x81A4);
        wr16(inode.raw, Ino::links, directory ? 2 : 1);
        wr32(inode.raw, Ino::flags, flagExtents);
        initExtentRoot(inode.raw + Ino::block);
        if (inodeSize_ >= 160) wr16(inode.raw, Ino::extraIsize, 32);

        uint64_t block = 0;

        if (directory) {
            uint64_t goal = static_cast<uint64_t>(firstDataBlock_) +
                            static_cast<uint64_t>((created - 1) / inodesPerGroup_) * blocksPerGroup_;

            status = allocBlock(goal, block);
            if (status != Status::ok) {
                freeInode(created, true);
                return status;
            }

            Name dot, dotdot;
            makeName(".", dot);
            makeName("..", dotdot);

            bool ok = zeroBytes(block * blockSize_, blockSize_) &&
                      writeEntryAt(block, 0, created, 12, dot, (featIncompat_ & incFiletype) ? ftDirectory : 0) &&
                      writeEntryAt(block, 12, parent, blockSize_ - 12, dotdot, (featIncompat_ & incFiletype) ? ftDirectory : 0);

            if (ok) status = appendBlock(inode, 0, block);
            if (!ok || status != Status::ok) {
                freeRun(block, 1);
                freeInode(created, true);
                return ok ? status : Status::ioError;
            }

            setFileSize(inode, blockSize_);
            adjustInodeBlocks(inode, blockSize_ / 512);
        }

        status = writeInode(inode);
        if (status == Status::ok) status = insertEntry(parent, name, created, directory ? ftDirectory : ftRegular);

        if (status != Status::ok) {
            if (block != 0) freeRun(block, 1);
            freeInode(created, directory);
            return status;
        }

        if (directory) adjustLinks(parent, 1);

        number = created;
        return Status::ok;
    }

    Status Ext4Driver::createFile(std::string_view path) {
        if (!mounted_) return Status::notReady;
        if (!writable_) return kReadOnly;

        uint32_t parent = 0;
        Name leaf;
        Status status = resolveParent(path, parent, leaf);
        if (status != Status::ok) return status;

        DirEntry existing;
        status = findEntry(parent, leaf, existing);
        if (status == Status::ok) {
            Inode inode;
            status = readInode(existing.inode, inode);
            if (status != Status::ok) return status;

            return isDirectoryInode(inode) ? Status::isDirectory : Status::ok;
        }
        if (status != Status::notFound) return status;

        uint32_t number = 0;
        return makeNode(parent, leaf, false, number);
    }

    Status Ext4Driver::createDirectory(std::string_view path) {
        if (!mounted_) return Status::notReady;
        if (!writable_) return kReadOnly;

        uint32_t parent = 0;
        Name leaf;
        Status status = resolveParent(path, parent, leaf);
        if (status != Status::ok) return status;

        uint32_t number = 0;
        return makeNode(parent, leaf, true, number);
    }

    Status Ext4Driver::writeAt(Inode& inode, uint64_t offset, std::string_view data, uint64_t& written) {
        written = 0;

        while (written < data.size()) {
            uint64_t position = offset + written;
            uint64_t logical = position / blockSize_;
            uint32_t inBlock = static_cast<uint32_t>(position % blockSize_);
            uint64_t phys = 0, run = 0;
            bool unwritten = false;

            Status status = mapBlock(inode, logical, phys, run, &unwritten);
            if (status != Status::ok) return status;
            if (unwritten) return kUnsupported;

            bool fresh = false;

            if (phys == 0) {
                uint64_t goal = static_cast<uint64_t>(firstDataBlock_) +
                                static_cast<uint64_t>((inode.number - 1) / inodesPerGroup_) * blocksPerGroup_;

                if (logical > 0) {
                    uint64_t previous = 0, previousRun = 0;
                    if (mapBlock(inode, logical - 1, previous, previousRun, nullptr) == Status::ok && previous != 0) goal = previous + 1;
                }

                status = allocBlock(goal, phys);
                if (status != Status::ok) return status;

                status = appendBlock(inode, logical, phys);
                if (status != Status::ok) {
                    freeRun(phys, 1);
                    return status;
                }

                adjustInodeBlocks(inode, blockSize_ / 512);
                run = 1;
                fresh = true;
            }

            uint64_t available = run * blockSize_ - inBlock;
            size_t count = static_cast<size_t>(minOf<uint64_t>(data.size() - written, available));

            if (fresh && (inBlock != 0 || count < blockSize_)) {
                if (!zeroBytes(phys * blockSize_, blockSize_)) return Status::ioError;
            }

            if (!writeBytes(phys * blockSize_ + inBlock, data.data() + written, count)) return Status::ioError;

            written += count;
        }

        return Status::ok;
    }

    Status Ext4Driver::writeFile(std::string_view path, std::string_view data, bool append) {
        if (!mounted_) return Status::notReady;
        if (!writable_) return kReadOnly;

        uint32_t parent = 0;
        Name leaf;
        Status status = resolveParent(path, parent, leaf);
        if (status != Status::ok) return status;

        status = markDirty();
        if (status != Status::ok) return status;

        DirEntry entry;
        status = findEntry(parent, leaf, entry);

        uint32_t number = 0;
        if (status == Status::notFound) {
            status = makeNode(parent, leaf, false, number);
            if (status != Status::ok) return status;
        } else if (status == Status::ok) {
            number = entry.inode;
        } else {
            return status;
        }

        Inode inode;
        status = readInode(number, inode);
        if (status != Status::ok) return status;
        if (isDirectoryInode(inode)) return Status::isDirectory;
        if (!isRegularInode(inode)) return Status::ioError;

        uint32_t flags = inodeFlags(inode);
        if ((flags & (flagInline | flagEncrypt | flagHugeFile)) != 0) return kUnsupported;

        uint64_t size = fileSize(inode);

        if (!append) {
            if (size > 0 || rd32(inode.raw, Ino::blocksLo) != 0) {
                status = freeFileBlocks(inode);
                if (status != Status::ok) return status;
            }

            initExtentRoot(inode.raw + Ino::block);
            wr32(inode.raw, Ino::flags, flags | flagExtents);
            wr32(inode.raw, Ino::blocksLo, 0);
            wr16(inode.raw, Ino::blocksHi, 0);
            setFileSize(inode, 0);
            size = 0;

            status = writeInode(inode);
            if (status != Status::ok) return status;
        } else if ((flags & flagExtents) == 0 && size > 0) {
            return kUnsupported;
        } else if ((flags & flagExtents) == 0) {
            initExtentRoot(inode.raw + Ino::block);
            wr32(inode.raw, Ino::flags, flags | flagExtents);
        }

        uint64_t written = 0;
        status = writeAt(inode, size, data, written);

        // Keep i_size in step with the blocks that were really attached, even if writing stopped early.
        setFileSize(inode, size + written);
        Status saved = writeInode(inode);

        return status != Status::ok ? status : saved;
    }

    Status Ext4Driver::remove(std::string_view path) {
        if (!mounted_) return Status::notReady;
        if (!writable_) return kReadOnly;

        uint32_t parent = 0;
        Name leaf;
        Status status = resolveParent(path, parent, leaf);
        if (status != Status::ok) return status;

        DirEntry entry;
        status = findEntry(parent, leaf, entry);
        if (status != Status::ok) return status;

        Inode inode;
        status = readInode(entry.inode, inode);
        if (status != Status::ok) return status;

        bool directory = isDirectoryInode(inode);
        if (entry.inode == rootInode || entry.inode == cwdInode_) return Status::invalidName;

        status = markDirty();
        if (status != Status::ok) return status;

        if (directory) {
            bool empty = false;
            status = dirIsEmpty(entry.inode, empty);
            if (status != Status::ok) return status;
            if (!empty) return Status::directoryNotEmpty;
        }

        status = removeEntry(entry);
        if (status != Status::ok) return status;

        if (!directory) {
            uint16_t links = rd16(inode.raw, Ino::links);
            if (links > 1) {
                wr16(inode.raw, Ino::links, static_cast<uint16_t>(links - 1));
                return writeInode(inode);
            }
        }

        bool freeData = directory || isRegularInode(inode);
        if (freeData && (rd32(inode.raw, Ino::blocksLo) != 0 || fileSize(inode) != 0)) {
            // fast symlinks / device nodes keep no blocks, so only regular files and directories get here
            status = freeFileBlocks(inode);
            if (status != Status::ok) return status;
        }

        status = freeInode(entry.inode, directory);
        if (status != Status::ok) return status;

        if (directory) return adjustLinks(parent, -1);

        return Status::ok;
    }

    // ---------------------------------------------------------------- mkfs

    Status Ext4Driver::format() {
        resetState();

        const uint64_t deviceBytes = devSectors() * kSector;
        const uint32_t bs = deviceBytes >= (64ull << 20) ? 4096 : 1024;
        const uint32_t logBs = bs == 4096 ? 2 : 0;
        const uint32_t fdb = bs == 1024 ? 1 : 0;
        const uint32_t bpg = bs * 8;
        const uint32_t isz = 256;

        uint64_t blocks = deviceBytes / bs;
        if (blocks > 0xFFFFFFFFull) blocks = 0xFFFFFFFFull;
        if (blocks < 64) return Status::noSpace;

        uint32_t groups, gdtBlocks, ipg, itb;
        for (;;) {
            groups = static_cast<uint32_t>((blocks - fdb + bpg - 1) / bpg);
            gdtBlocks = static_cast<uint32_t>((static_cast<uint64_t>(groups) * 32 + bs - 1) / bs);

            uint64_t groupBlocks = minOf<uint64_t>(blocks - fdb, bpg);
            uint64_t wanted = groupBlocks * bs / 16384;
            if (wanted < 32) wanted = 32;

            ipg = static_cast<uint32_t>((wanted + 15) / 16 * 16);
            if (ipg > bs * 8) ipg = bs * 8;
            itb = (ipg * isz + bs - 1) / bs;

            uint64_t lastBlocks = (blocks - fdb) - static_cast<uint64_t>(groups - 1) * bpg;
            uint64_t lastMeta = 2 + itb + (groups == 1 ? 1 + gdtBlocks : 0);

            // a tiny trailing group is not worth its metadata: drop it, like mke2fs does
            if (groups > 1 && lastBlocks < lastMeta + 16) {
                blocks = fdb + static_cast<uint64_t>(groups - 1) * bpg;
                continue;
            }

            break;
        }

        const uint64_t gdt0 = fdb + 1;
        const uint64_t bitmap0 = gdt0 + gdtBlocks;
        const uint64_t inodeBitmap0 = bitmap0 + 1;
        const uint64_t table0 = inodeBitmap0 + 1;
        const uint64_t rootBlock = table0 + itb;
        const uint64_t lostBlock = rootBlock + 1;
        const uint32_t prefix0 = static_cast<uint32_t>(lostBlock + 1 - fdb);
        const uint32_t usedInodes0 = 11;

        if (blocks - fdb < prefix0 + 2) return Status::noSpace;

        // wipe the first KiB (old FAT boot sector / MBR signature) and the descriptor table
        if (!zeroBytes(0, 2048) || !zeroBytes(gdt0 * bs, static_cast<uint64_t>(gdtBlocks) * bs)) return Status::ioError;

        auto writeBitmap = [&](uint64_t block, uint32_t prefix, uint32_t padStart) -> bool {
            uint8_t buffer[kSector];

            for (uint32_t window = 0; window < bs; window += kSector) {
                memset(buffer, 0, sizeof(buffer));

                for (uint32_t i = 0; i < kSector * 8; i++) {
                    uint32_t bit = window * 8 + i;
                    if (bit < prefix || bit >= padStart) buffer[i / 8] |= static_cast<uint8_t>(1u << (i % 8));
                }

                if (!writeBytes(block * bs + window, buffer, sizeof(buffer))) return false;
            }

            return true;
        };

        uint64_t totalFreeBlocks = 0;
        uint64_t totalFreeInodes = 0;

        for (uint32_t group = 0; group < groups; group++) {
            uint64_t groupStart = fdb + static_cast<uint64_t>(group) * bpg;
            uint32_t groupBlocks = static_cast<uint32_t>(minOf<uint64_t>(bpg, blocks - groupStart));

            uint64_t blockBitmap = group == 0 ? bitmap0 : groupStart;
            uint64_t inodeBitmap = group == 0 ? inodeBitmap0 : groupStart + 1;
            uint64_t table = group == 0 ? table0 : groupStart + 2;
            uint32_t prefix = group == 0 ? prefix0 : 2 + itb;
            uint32_t usedInodes = group == 0 ? usedInodes0 : 0;

            if (!zeroBytes(table * bs, static_cast<uint64_t>(itb) * bs)) return Status::ioError;
            if (!writeBitmap(blockBitmap, prefix, groupBlocks)) return Status::ioError;
            if (!writeBitmap(inodeBitmap, usedInodes, ipg)) return Status::ioError;

            uint8_t gd[32];
            memset(gd, 0, sizeof(gd));
            wr32(gd, Gd::blockBitmapLo, static_cast<uint32_t>(blockBitmap));
            wr32(gd, Gd::inodeBitmapLo, static_cast<uint32_t>(inodeBitmap));
            wr32(gd, Gd::inodeTableLo, static_cast<uint32_t>(table));
            wr16(gd, Gd::freeBlocksLo, static_cast<uint16_t>(groupBlocks - prefix));
            wr16(gd, Gd::freeInodesLo, static_cast<uint16_t>(ipg - usedInodes));
            wr16(gd, Gd::usedDirsLo, group == 0 ? 2 : 0);

            if (!writeBytes(gdt0 * bs + static_cast<uint64_t>(group) * 32, gd, sizeof(gd))) return Status::ioError;

            totalFreeBlocks += groupBlocks - prefix;
            totalFreeInodes += ipg - usedInodes;
        }

        // root directory and lost+found
        auto putEntry = [&](uint64_t block, uint32_t offset, uint32_t inode, uint32_t recLen, const char* text, uint8_t length) -> bool {
            uint8_t buffer[8 + 16];
            memset(buffer, 0, sizeof(buffer));

            wr32(buffer, 0, inode);
            wr16(buffer, 4, static_cast<uint16_t>(recLen));
            buffer[6] = length;
            buffer[7] = ftDirectory;
            memcpy(buffer + 8, text, length);

            return writeBytes(block * bs + offset, buffer, 8 + length);
        };

        auto putInode = [&](uint32_t number, uint16_t mode, uint16_t links, uint64_t block) -> bool {
            uint8_t raw[inodeBufferSize];
            memset(raw, 0, sizeof(raw));

            wr16(raw, Ino::mode, mode);
            wr32(raw, Ino::sizeLo, bs);
            wr16(raw, Ino::links, links);
            wr32(raw, Ino::blocksLo, bs / 512);
            wr32(raw, Ino::flags, flagExtents);
            wr16(raw, Ino::extraIsize, 32);

            uint8_t* root = raw + Ino::block;
            initExtentRoot(root);
            wr16(root, 2, 1);
            wr32(root, 12 + 0, 0);
            wr16(root, 12 + 4, 1);
            wr16(root, 12 + 6, static_cast<uint16_t>(block >> 32));
            wr32(root, 12 + 8, static_cast<uint32_t>(block & 0xFFFFFFFFu));

            return writeBytes(table0 * bs + static_cast<uint64_t>(number - 1) * isz, raw, sizeof(raw));
        };

        bool ok = zeroBytes(rootBlock * bs, bs) && zeroBytes(lostBlock * bs, bs) &&
                  putEntry(rootBlock, 0, rootInode, 12, ".", 1) &&
                  putEntry(rootBlock, 12, rootInode, 12, "..", 2) &&
                  putEntry(rootBlock, 24, 11, bs - 24, "lost+found", 10) &&
                  putEntry(lostBlock, 0, 11, 12, ".", 1) &&
                  putEntry(lostBlock, 12, rootInode, bs - 12, "..", 2) &&
                  putInode(rootInode, 0x41ED, 3, rootBlock) &&
                  putInode(11, 0x41C0, 2, lostBlock);
        if (!ok) return Status::ioError;

        uint8_t sb[kSbSize];
        memset(sb, 0, sizeof(sb));

        wr32(sb, Sb::inodeCount, groups * ipg);
        wr32(sb, Sb::blocksLo, static_cast<uint32_t>(blocks));
        wr32(sb, Sb::freeBlocksLo, static_cast<uint32_t>(totalFreeBlocks));
        wr32(sb, Sb::freeInodes, static_cast<uint32_t>(totalFreeInodes));
        wr32(sb, Sb::firstDataBlock, fdb);
        wr32(sb, Sb::logBlockSize, logBs);
        wr32(sb, Sb::logClusterSize, logBs);
        wr32(sb, Sb::blocksPerGroup, bpg);
        wr32(sb, Sb::clustersPerGroup, bpg);
        wr32(sb, Sb::inodesPerGroup, ipg);
        wr16(sb, Sb::maxMntCount, 0xFFFF);
        wr16(sb, Sb::magic, kMagic);
        wr16(sb, Sb::state, 1);
        wr16(sb, Sb::errors, 1);
        wr32(sb, Sb::revLevel, 1);
        wr32(sb, Sb::firstIno, 11);
        wr16(sb, Sb::inodeSize, isz);
        wr32(sb, Sb::featCompat, compatSparseSuper2);
        wr32(sb, Sb::featIncompat, incFiletype | incExtents);
        wr32(sb, Sb::featRoCompat, roLargeFile);
        wr16(sb, Sb::minExtra, 32);
        wr16(sb, Sb::wantExtra, 32);

        const char label[] = "UNITYKERNEL";
        memcpy(sb + Sb::volumeName, label, sizeof(label) - 1);

        const char uuidSeed[] = "UnityKernelExt4";
        memcpy(sb + Sb::uuid, uuidSeed, 16);

        if (!writeBytes(kSbOffset, sb, sizeof(sb))) return Status::ioError;

        return mountVolume() == Status::ok ? Status::ok : Status::ioError;
    }
}