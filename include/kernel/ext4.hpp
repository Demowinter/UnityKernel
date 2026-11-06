#pragma once
#include <string_view>
#include <cstddef>
#include <cstdint>
#include <drivers/fs.hpp>
#include <kernel/subsystems/resource.hpp>

namespace Driver::FS {
    namespace EXT4 {
        constexpr uint32_t rootInode = 2;
        constexpr uint32_t maxNameLength = 255;
        constexpr uint32_t maxPathLength = 256;
        constexpr uint32_t inodeBufferSize = 256;

        // In-memory copy of the first (up to) 256 bytes of an on-disk inode.
        struct Inode {
            uint8_t raw[inodeBufferSize];
            uint32_t number;
            uint32_t loaded;
        };

        struct GroupDesc {
            uint8_t raw[64];
            uint32_t size;
        };

        struct Name {
            char text[maxNameLength + 1];
            uint32_t length;
        };

        constexpr uint32_t noOffset = 0xFFFFFFFFu;

        struct DirEntry {
            uint32_t inode;
            uint32_t recLen;
            uint32_t offset;
            uint32_t prevOffset;
            uint64_t block;
            uint8_t nameLen;
            uint8_t type;
            char name[maxNameLength + 1];
        };

        struct DirIter {
            Inode dir;
            uint64_t blocks;
            uint64_t logical;
            uint64_t phys;
            uint32_t pos;
            uint32_t prevPos;
            bool open;
        };
    }

    // ext4 driver. No heap, no big buffers: every access goes through
    // readBytes()/writeBytes() with one 512-byte bounce buffer on the stack.
    //
    // Read support: ext2/3/4, extents and block maps, 64bit, flex_bg, meta_bg, htree dirs
    // (scanned linearly). Write support is enabled only when the volume is safe to modify
    // without a journal / checksums (see writable()).
    class Ext4Driver : public FSDriver {
    public:
        Ext4Driver() : FSDriver{Type::EXT4} {}

        bool initialize() override;
        void finalize() override;

        std::string_view cwd() override;

        Status changeDirectory(std::string_view path) override;
        Status listDirectory(std::string_view path, ListCallback callback, void* context) override;

        Status readFile(std::string_view path, ChunkCallback callback, void* context) override;
        Status writeFile(std::string_view path, std::string_view data, bool append) override;

        Status createFile(std::string_view path) override;
        Status createDirectory(std::string_view path) override;

        Status remove(std::string_view path) override;

        Status stat(std::string_view path, EntryInfo& info) override;

        // Creates a fresh ext4 volume on the whole device and mounts it.
        Status format() override;

        bool writable() const { return writable_; }

        // Which block-device resource this driver talks to (BlockdevDriver calls need a ResourceID).
        // Call before initialize()/format().
        void setResource(Kernel::ResourceSubsystem::ResourceID id) { resid_ = id; }

    private:
        // device adapter (the only place that touches the block device object)
        bool devRead(uint64_t lba, void* buffer, size_t count);
        bool devWrite(uint64_t lba, const void* buffer, size_t count);
        uint64_t devSectors();

        bool readBytes(uint64_t offset, void* dst, size_t length);
        bool writeBytes(uint64_t offset, const void* src, size_t length);
        bool zeroBytes(uint64_t offset, uint64_t length);

        // mounting
        void resetState();
        Status mountVolume();
        Status mountAt(uint64_t partitionOffset, uint64_t partitionSize, bool& recognized);

        // geometry / descriptors
        bool blockInRange(uint64_t block, uint64_t count = 1) const;
        bool groupHasSuper(uint64_t group) const;
        bool descriptorOffset(uint32_t group, uint64_t& offset);
        Status readGroupDesc(uint32_t group, EXT4::GroupDesc& gd);
        Status writeGroupDesc(uint32_t group, const EXT4::GroupDesc& gd);

        // inodes
        bool inodeOffset(uint32_t number, uint64_t& offset);
        Status readInode(uint32_t number, EXT4::Inode& inode);
        Status writeInode(const EXT4::Inode& inode);
        Status zeroInodeOnDisk(uint32_t number);
        uint64_t fileSize(const EXT4::Inode& inode) const;

        // block mapping
        Status mapBlock(const EXT4::Inode& inode, uint64_t logical, uint64_t& phys, uint64_t& run, bool* unwritten);
        Status mapExtent(const EXT4::Inode& inode, uint64_t logical, uint64_t& phys, uint64_t& run, bool* unwritten);
        Status mapLegacy(const EXT4::Inode& inode, uint64_t logical, uint64_t& phys);
        bool readPointer(uint64_t block, uint64_t index, uint32_t& value);

        // allocation
        Status markDirty();
        bool bitmapUpdate(uint64_t bitmapBlock, uint32_t first, uint32_t count, bool set);
        Status bitmapFindZero(uint64_t bitmapBlock, uint32_t from, uint32_t limit, uint32_t& bit, bool& found);
        Status adjustGroup(uint32_t group, int64_t blocks, int64_t inodes, int64_t dirs);
        Status adjustSuper(int64_t blocks, int64_t inodes);
        Status allocBlock(uint64_t goal, uint64_t& block);
        Status allocInode(uint32_t parent, bool directory, uint32_t& number);
        Status freeRun(uint64_t start, uint64_t count);
        Status freeInode(uint32_t number, bool directory);
        Status appendBlock(EXT4::Inode& inode, uint64_t logical, uint64_t phys);
        Status freeFileBlocks(const EXT4::Inode& inode);
        Status freeExtentNode(const EXT4::Inode* inode, uint64_t nodeBlock, uint32_t guard);
        Status freeLegacyTree(uint64_t block, uint32_t level);

        // directories
        Status dirOpen(uint32_t number, EXT4::DirIter& it);
        Status dirNext(EXT4::DirIter& it, EXT4::DirEntry& entry, bool& end);
        Status findEntry(uint32_t dir, const EXT4::Name& name, EXT4::DirEntry& entry);
        Status lookup(uint32_t dir, std::string_view name, uint32_t& number);
        Status dirIsEmpty(uint32_t dir, bool& empty);
        bool writeEntryAt(uint64_t block, uint32_t offset, uint32_t inode, uint32_t recLen, const EXT4::Name& name, uint8_t type);
        Status insertEntry(uint32_t parent, const EXT4::Name& name, uint32_t child, uint8_t type);
        Status removeEntry(const EXT4::DirEntry& entry);

        // paths
        Status resolve(std::string_view path, uint32_t& number);
        Status resolveParent(std::string_view path, uint32_t& parent, EXT4::Name& leaf);

        // high level helpers
        Status makeNode(uint32_t parent, const EXT4::Name& name, bool directory, uint32_t& number);
        Status writeAt(EXT4::Inode& inode, uint64_t offset, std::string_view data, uint64_t& written);
        Status adjustLinks(uint32_t number, int32_t delta);
        Status fillInfo(uint32_t number, const char* name, size_t nameLength, EntryInfo& info);

        // state
        Kernel::ResourceSubsystem::ResourceID resid_{};
        bool mounted_ = false;
        bool writable_ = false;
        bool dirty_ = false;

        uint64_t partitionOffset_ = 0;
        uint64_t volumeBytes_ = 0;
        uint64_t blockCount_ = 0;
        uint32_t blockSize_ = 0;
        uint32_t firstDataBlock_ = 0;
        uint32_t blocksPerGroup_ = 0;
        uint32_t inodesPerGroup_ = 0;
        uint32_t inodeCount_ = 0;
        uint32_t inodeSize_ = 0;
        uint32_t groupCount_ = 0;
        uint32_t descSize_ = 0;
        uint32_t descPerBlock_ = 0;
        uint32_t firstMetaBg_ = 0;
        uint32_t firstIno_ = 11;
        uint32_t backupGroups_[2] = {0, 0};
        uint32_t featCompat_ = 0;
        uint32_t featIncompat_ = 0;
        uint32_t featRoCompat_ = 0;

        uint32_t cwdInode_ = EXT4::rootInode;
        char cwdPath_[EXT4::maxPathLength] = {'/', 0};
        uint32_t cwdLength_ = 1;
    };
}