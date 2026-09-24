#include <kernel/fat32.hpp>
#include <kernel/block.hpp>
#include <libbase/memory.hpp>

namespace Kernel::FAT32 {
    namespace {
        constexpr uint8_t sectorsPerCluster = 1;
        constexpr uint32_t reservedSectors = 32;
        constexpr uint32_t fatSectors = 2;
        constexpr uint32_t clusterCount = 128;
        constexpr uint32_t rootCluster = 2;
        constexpr uint32_t totalSectors = reservedSectors + fatSectors + clusterCount * sectorsPerCluster;
        constexpr uint32_t clusterBytes = Block::sectorSize * sectorsPerCluster;
        constexpr uint32_t diskBytes = totalSectors * Block::sectorSize;

        constexpr uint32_t endOfChain = 0x0FFFFFFF;
        constexpr uint32_t endOfChainMin = 0x0FFFFFF8;
        constexpr uint8_t attrReadOnly = 0x01;
        constexpr uint8_t attrHidden = 0x02;
        constexpr uint8_t attrSystem = 0x04;
        constexpr uint8_t attrVolumeId = 0x08;
        constexpr uint8_t attrDirectory = 0x10;
        constexpr uint8_t attrArchive = 0x20;
        constexpr uint8_t attrLongName = attrReadOnly | attrHidden | attrSystem | attrVolumeId;

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

        static_assert(sizeof(DirectoryEntry) == 32);

        constexpr uint32_t entriesPerCluster = clusterBytes / sizeof(DirectoryEntry);

        struct EntryRef {
            uint32_t entryCluster = 0;
            uint32_t entryIndex = 0;
            DirectoryEntry entry = {};
            bool root = false;
            bool synthetic = false;
        };

        struct WalkResult {
            EntryRef ref = {};
            uint32_t cluster = rootCluster;
        };

        uint8_t disk[diskBytes];
        Block::MemoryDevice device;
        bool mounted = false;
        uint32_t cwdCluster = rootCluster;
        char cwdPath[128] = "/";
        size_t cwdPathLength = 1;

        bool textEquals(std::string_view lhs, const char* rhs) {
            size_t index = 0;
            while (index < lhs.size() && rhs[index] != 0) {
                if (lhs[index] != rhs[index]) return false;
                index++;
            }

            return index == lhs.size() && rhs[index] == 0;
        }

        char upper(char ch) {
            if (ch >= 'a' && ch <= 'z') return static_cast<char>(ch - 'a' + 'A');

            return ch;
        }

        char lower(char ch) {
            if (ch >= 'A' && ch <= 'Z') return static_cast<char>(ch - 'A' + 'a');

            return ch;
        }

        bool isNameChar(char ch) {
            ch = upper(ch);

            return (ch >= 'A' && ch <= 'Z') ||
                   (ch >= '0' && ch <= '9') ||
                   ch == '_' || ch == '-' || ch == '$' || ch == '~';
        }

        void write16(uint8_t* buffer, uint32_t offset, uint16_t value) {
            buffer[offset] = static_cast<uint8_t>(value & 0xFF);
            buffer[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        }

        void write32(uint8_t* buffer, uint32_t offset, uint32_t value) {
            buffer[offset] = static_cast<uint8_t>(value & 0xFF);
            buffer[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
            buffer[offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
            buffer[offset + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
        }

        uint32_t readFat(uint32_t cluster) {
            uint32_t offset = reservedSectors * Block::sectorSize + cluster * 4;
            uint32_t value = 0;

            value |= static_cast<uint32_t>(disk[offset]);
            value |= static_cast<uint32_t>(disk[offset + 1]) << 8;
            value |= static_cast<uint32_t>(disk[offset + 2]) << 16;
            value |= static_cast<uint32_t>(disk[offset + 3]) << 24;

            return value & 0x0FFFFFFF;
        }

        void writeFat(uint32_t cluster, uint32_t value) {
            uint32_t offset = reservedSectors * Block::sectorSize + cluster * 4;
            value &= 0x0FFFFFFF;

            disk[offset] = static_cast<uint8_t>(value & 0xFF);
            disk[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
            disk[offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
            disk[offset + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
        }

        uint32_t clusterToLba(uint32_t cluster) {
            return reservedSectors + fatSectors + (cluster - rootCluster) * sectorsPerCluster;
        }

        bool validCluster(uint32_t cluster) {
            return cluster >= rootCluster && cluster < rootCluster + clusterCount;
        }

        Status readCluster(uint32_t cluster, void* buffer) {
            if (!validCluster(cluster)) return Status::ioError;
            if (!device.read(clusterToLba(cluster), buffer, sectorsPerCluster)) return Status::ioError;

            return Status::ok;
        }

        Status writeCluster(uint32_t cluster, const void* buffer) {
            if (!validCluster(cluster)) return Status::ioError;
            if (!device.write(clusterToLba(cluster), buffer, sectorsPerCluster)) return Status::ioError;

            return Status::ok;
        }

        Status zeroCluster(uint32_t cluster) {
            uint8_t buffer[clusterBytes];
            memset(buffer, 0, sizeof(buffer));

            return writeCluster(cluster, buffer);
        }

        uint32_t nextCluster(uint32_t cluster) {
            uint32_t next = readFat(cluster);
            if (next >= endOfChainMin) return 0;

            return next;
        }

        Status allocateCluster(uint32_t& cluster) {
            for (uint32_t candidate = rootCluster + 1; candidate < rootCluster + clusterCount; candidate++) {
                if (readFat(candidate) == 0) {
                    writeFat(candidate, endOfChain);
                    cluster = candidate;
                    return zeroCluster(candidate);
                }
            }

            return Status::noSpace;
        }

        void freeChain(uint32_t firstCluster) {
            uint32_t cluster = firstCluster;

            while (validCluster(cluster)) {
                uint32_t next = nextCluster(cluster);
                writeFat(cluster, 0);
                if (next == 0) return;
                cluster = next;
            }
        }

        uint32_t entryFirstCluster(const DirectoryEntry& entry) {
            return (static_cast<uint32_t>(entry.firstClusterHigh) << 16) | entry.firstClusterLow;
        }

        void setEntryFirstCluster(DirectoryEntry& entry, uint32_t cluster) {
            entry.firstClusterLow = static_cast<uint16_t>(cluster & 0xFFFF);
            entry.firstClusterHigh = static_cast<uint16_t>((cluster >> 16) & 0xFFFF);
        }

        bool isDirectory(const DirectoryEntry& entry) {
            return (entry.attr & attrDirectory) != 0;
        }

        void fillShortName(char* name, std::string_view text) {
            memset(name, ' ', 11);
            for (size_t i = 0; i < text.size() && i < 11; i++) name[i] = text[i];
        }

        bool makeShortName(std::string_view component, char shortName[11]) {
            if (component.empty()) return false;
            if (textEquals(component, ".") || textEquals(component, "..")) return false;

            memset(shortName, ' ', 11);

            size_t dot = component.size();
            for (size_t i = 0; i < component.size(); i++) {
                if (component[i] == '.') {
                    if (dot != component.size()) return false;
                    dot = i;
                }
            }

            size_t baseLength = dot;
            size_t extLength = dot == component.size() ? 0 : component.size() - dot - 1;

            if (baseLength == 0 || baseLength > 8 || extLength > 3) return false;

            for (size_t i = 0; i < baseLength; i++) {
                if (!isNameChar(component[i])) return false;
                shortName[i] = upper(component[i]);
            }

            for (size_t i = 0; i < extLength; i++) {
                char ch = component[dot + 1 + i];
                if (!isNameChar(ch)) return false;
                shortName[8 + i] = upper(ch);
            }

            return true;
        }

        void displayName(const char shortName[11], char output[13]) {
            size_t pos = 0;

            for (size_t i = 0; i < 8 && shortName[i] != ' '; i++) output[pos++] = lower(shortName[i]);

            if (shortName[8] != ' ') {
                output[pos++] = '.';
                for (size_t i = 8; i < 11 && shortName[i] != ' '; i++) output[pos++] = lower(shortName[i]);
            }

            output[pos] = 0;
        }

        bool nextPathComponent(std::string_view path, size_t& pos, std::string_view& component) {
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

        bool shortNameEquals(const char lhs[11], const char rhs[11]) {
            for (size_t i = 0; i < 11; i++) {
                if (lhs[i] != rhs[i]) return false;
            }

            return true;
        }

        Status writeEntry(const EntryRef& ref, const DirectoryEntry& entry) {
            if (ref.root || ref.synthetic) return Status::ioError;

            uint8_t buffer[clusterBytes];
            Status status = readCluster(ref.entryCluster, buffer);
            if (status != Status::ok) return status;

            auto* entries = reinterpret_cast<DirectoryEntry*>(buffer);
            entries[ref.entryIndex] = entry;

            return writeCluster(ref.entryCluster, buffer);
        }

        Status findEntry(uint32_t dirCluster, const char shortName[11], EntryRef& ref) {
            uint32_t cluster = dirCluster;
            uint8_t buffer[clusterBytes];

            while (validCluster(cluster)) {
                Status status = readCluster(cluster, buffer);
                if (status != Status::ok) return status;

                auto* entries = reinterpret_cast<DirectoryEntry*>(buffer);
                for (uint32_t index = 0; index < entriesPerCluster; index++) {
                    DirectoryEntry& entry = entries[index];

                    if (static_cast<uint8_t>(entry.name[0]) == 0x00) return Status::notFound;
                    if (static_cast<uint8_t>(entry.name[0]) == 0xE5) continue;
                    if ((entry.attr & attrLongName) == attrLongName) continue;

                    if (shortNameEquals(entry.name, shortName)) {
                        ref.entryCluster = cluster;
                        ref.entryIndex = index;
                        ref.entry = entry;
                        ref.root = false;
                        ref.synthetic = false;
                        return Status::ok;
                    }
                }

                cluster = nextCluster(cluster);
            }

            return Status::notFound;
        }

        Status findFreeEntry(uint32_t dirCluster, EntryRef& ref) {
            uint32_t cluster = dirCluster;
            uint32_t previous = 0;
            uint8_t buffer[clusterBytes];

            while (validCluster(cluster)) {
                Status status = readCluster(cluster, buffer);
                if (status != Status::ok) return status;

                auto* entries = reinterpret_cast<DirectoryEntry*>(buffer);
                for (uint32_t index = 0; index < entriesPerCluster; index++) {
                    uint8_t first = static_cast<uint8_t>(entries[index].name[0]);
                    if (first == 0x00 || first == 0xE5) {
                        ref.entryCluster = cluster;
                        ref.entryIndex = index;
                        ref.entry = entries[index];
                        ref.root = false;
                        ref.synthetic = false;
                        return Status::ok;
                    }
                }

                previous = cluster;
                cluster = nextCluster(cluster);
            }

            uint32_t newCluster = 0;
            Status status = allocateCluster(newCluster);
            if (status != Status::ok) return status;

            writeFat(previous, newCluster);
            writeFat(newCluster, endOfChain);

            ref.entryCluster = newCluster;
            ref.entryIndex = 0;
            ref.entry = {};
            ref.root = false;
            ref.synthetic = false;

            return Status::ok;
        }

        Status resolveExisting(std::string_view path, WalkResult& result) {
            if (!mounted) return Status::notReady;

            uint32_t current = (!path.empty() && path[0] == '/') ? rootCluster : cwdCluster;
            result.cluster = current;
            result.ref.root = current == rootCluster;
            result.ref.synthetic = true;
            result.ref.entry = {};
            result.ref.entry.attr = attrDirectory;
            setEntryFirstCluster(result.ref.entry, current);

            size_t pos = 0;
            std::string_view component;
            bool sawComponent = false;

            while (nextPathComponent(path, pos, component)) {
                sawComponent = true;

                if (textEquals(component, ".")) {
                    result.cluster = current;
                    result.ref.synthetic = true;
                    result.ref.root = current == rootCluster;
                    result.ref.entry = {};
                    result.ref.entry.attr = attrDirectory;
                    setEntryFirstCluster(result.ref.entry, current);
                    continue;
                }

                char shortName[11];
                if (textEquals(component, "..")) {
                    fillShortName(shortName, "..");
                } else if (!makeShortName(component, shortName)) {
                    return Status::invalidName;
                }

                EntryRef ref;
                Status status = findEntry(current, shortName, ref);

                if (textEquals(component, "..") && current == rootCluster && status == Status::notFound) {
                    result.cluster = rootCluster;
                    result.ref.root = true;
                    result.ref.synthetic = true;
                    result.ref.entry = {};
                    result.ref.entry.attr = attrDirectory;
                    setEntryFirstCluster(result.ref.entry, rootCluster);
                    continue;
                }

                if (status != Status::ok) return status;

                result.ref = ref;
                result.cluster = entryFirstCluster(ref.entry);

                if (hasMoreComponents(path, pos)) {
                    if (!isDirectory(ref.entry)) return Status::notDirectory;
                    current = result.cluster;
                }
            }

            if (!sawComponent) {
                result.cluster = current;
                result.ref.root = current == rootCluster;
                result.ref.synthetic = true;
                result.ref.entry = {};
                result.ref.entry.attr = attrDirectory;
                setEntryFirstCluster(result.ref.entry, current);
            }

            return Status::ok;
        }

        Status resolveParent(std::string_view path, uint32_t& parentCluster, char leafName[11]) {
            if (!mounted) return Status::notReady;

            uint32_t current = (!path.empty() && path[0] == '/') ? rootCluster : cwdCluster;
            size_t pos = 0;
            std::string_view component;
            bool sawComponent = false;

            while (nextPathComponent(path, pos, component)) {
                sawComponent = true;
                bool last = !hasMoreComponents(path, pos);

                if (last) {
                    if (!makeShortName(component, leafName)) return Status::invalidName;

                    parentCluster = current;
                    return Status::ok;
                }

                if (textEquals(component, ".")) continue;

                char shortName[11];
                if (textEquals(component, "..")) {
                    if (current == rootCluster) continue;
                    fillShortName(shortName, "..");
                } else if (!makeShortName(component, shortName)) {
                    return Status::invalidName;
                }

                EntryRef ref;
                Status status = findEntry(current, shortName, ref);
                if (status != Status::ok) return status;
                if (!isDirectory(ref.entry)) return Status::notDirectory;

                current = entryFirstCluster(ref.entry);
            }

            (void)sawComponent;
            return Status::invalidName;
        }

        Status createEntry(uint32_t parentCluster, const char shortName[11], uint8_t attr, uint32_t firstCluster, uint32_t size) {
            EntryRef existing;
            Status status = findEntry(parentCluster, shortName, existing);
            if (status == Status::ok) return Status::alreadyExists;
            if (status != Status::notFound) return status;

            EntryRef freeRef;
            status = findFreeEntry(parentCluster, freeRef);
            if (status != Status::ok) return status;

            DirectoryEntry entry = {};
            for (size_t i = 0; i < 11; i++) entry.name[i] = shortName[i];
            entry.attr = attr;
            entry.fileSize = size;
            setEntryFirstCluster(entry, firstCluster);

            return writeEntry(freeRef, entry);
        }

        Status initializeDirectory(uint32_t cluster, uint32_t parentCluster) {
            uint8_t buffer[clusterBytes];
            memset(buffer, 0, sizeof(buffer));

            auto* entries = reinterpret_cast<DirectoryEntry*>(buffer);

            fillShortName(entries[0].name, ".");
            entries[0].attr = attrDirectory;
            setEntryFirstCluster(entries[0], cluster);

            fillShortName(entries[1].name, "..");
            entries[1].attr = attrDirectory;
            setEntryFirstCluster(entries[1], parentCluster);

            return writeCluster(cluster, buffer);
        }

        bool directoryEmpty(uint32_t cluster) {
            uint8_t buffer[clusterBytes];

            while (validCluster(cluster)) {
                if (readCluster(cluster, buffer) != Status::ok) return false;

                auto* entries = reinterpret_cast<DirectoryEntry*>(buffer);
                for (uint32_t index = 0; index < entriesPerCluster; index++) {
                    DirectoryEntry& entry = entries[index];
                    uint8_t first = static_cast<uint8_t>(entry.name[0]);

                    if (first == 0x00) return true;
                    if (first == 0xE5) continue;
                    if ((entry.attr & attrLongName) == attrLongName) continue;
                    if (entry.name[0] == '.' && entry.name[1] == ' ') continue;
                    if (entry.name[0] == '.' && entry.name[1] == '.') continue;

                    return false;
                }

                cluster = nextCluster(cluster);
            }

            return true;
        }

        Status ensureChainSize(uint32_t& firstCluster, uint32_t byteSize) {
            if (byteSize == 0) return Status::ok;

            uint32_t needed = (byteSize + clusterBytes - 1) / clusterBytes;

            if (firstCluster == 0) {
                Status status = allocateCluster(firstCluster);
                if (status != Status::ok) return status;
                needed--;
            }

            uint32_t cluster = firstCluster;
            while (needed > 1) {
                uint32_t next = nextCluster(cluster);
                if (next == 0) {
                    Status status = allocateCluster(next);
                    if (status != Status::ok) return status;
                    writeFat(cluster, next);
                    writeFat(next, endOfChain);
                }

                cluster = next;
                needed--;
            }

            return Status::ok;
        }

        Status writeData(uint32_t firstCluster, uint32_t offset, std::string_view data) {
            if (data.empty()) return Status::ok;
            if (firstCluster == 0) return Status::ioError;

            uint32_t cluster = firstCluster;
            uint32_t clusterOffset = offset;

            while (clusterOffset >= clusterBytes) {
                cluster = nextCluster(cluster);
                if (cluster == 0) return Status::ioError;
                clusterOffset -= clusterBytes;
            }

            size_t dataPos = 0;
            uint8_t buffer[clusterBytes];

            while (dataPos < data.size()) {
                Status status = readCluster(cluster, buffer);
                if (status != Status::ok) return status;

                size_t count = clusterBytes - clusterOffset;
                if (count > data.size() - dataPos) count = data.size() - dataPos;

                memcpy(buffer + clusterOffset, data.data() + dataPos, count);

                status = writeCluster(cluster, buffer);
                if (status != Status::ok) return status;

                dataPos += count;
                clusterOffset = 0;

                if (dataPos < data.size()) {
                    cluster = nextCluster(cluster);
                    if (cluster == 0) return Status::ioError;
                }
            }

            return Status::ok;
        }

        Status fillEntryInfo(const DirectoryEntry& entry, EntryInfo& info) {
            displayName(entry.name, info.name);
            info.directory = isDirectory(entry);
            info.size = entry.fileSize;
            info.cluster = entryFirstCluster(entry);

            return Status::ok;
        }

        void writeBootSector() {
            uint8_t boot[Block::sectorSize];
            memset(boot, 0, sizeof(boot));

            boot[0] = 0xEB;
            boot[1] = 0x58;
            boot[2] = 0x90;

            const char oem[] = "UNITYKRN";
            memcpy(boot + 3, oem, 8);

            write16(boot, 11, Block::sectorSize);
            boot[13] = sectorsPerCluster;
            write16(boot, 14, reservedSectors);
            boot[16] = 1;
            write16(boot, 17, 0);
            write16(boot, 19, 0);
            boot[21] = 0xF8;
            write16(boot, 22, 0);
            write16(boot, 24, 63);
            write16(boot, 26, 255);
            write32(boot, 28, 0);
            write32(boot, 32, totalSectors);
            write32(boot, 36, fatSectors);
            write16(boot, 40, 0);
            write16(boot, 42, 0);
            write32(boot, 44, rootCluster);
            write16(boot, 48, 1);
            write16(boot, 50, 6);
            boot[64] = 0x80;
            boot[66] = 0x29;
            write32(boot, 67, 0x554B3332);

            const char label[] = "UNITYKERNEL";
            memcpy(boot + 71, label, 11);

            const char fsType[] = "FAT32   ";
            memcpy(boot + 82, fsType, 8);

            boot[510] = 0x55;
            boot[511] = 0xAA;

            device.write(0, boot);
        }
    }

    void initialize() {
        if (mounted) return;

        memset(disk, 0, sizeof(disk));
        device.initialize(disk, totalSectors);

        writeBootSector();

        writeFat(0, 0x0FFFFFF8);
        writeFat(1, 0x0FFFFFFF);
        writeFat(rootCluster, endOfChain);
        zeroCluster(rootCluster);

        cwdCluster = rootCluster;
        cwdPath[0] = '/';
        cwdPath[1] = 0;
        cwdPathLength = 1;
        mounted = true;

        writeFile("/welcome.txt", "UnityKernel FAT32 ram disk is online.", false);
        makeDirectory("/tmp");
    }

    bool ready() {
        return mounted;
    }

    const char* statusText(Status status) {
        switch (status) {
            case Status::ok: return "ok";
            case Status::notReady: return "filesystem is not ready";
            case Status::invalidName: return "invalid FAT32 8.3 name";
            case Status::notFound: return "not found";
            case Status::alreadyExists: return "already exists";
            case Status::notDirectory: return "not a directory";
            case Status::isDirectory: return "is a directory";
            case Status::directoryNotEmpty: return "directory not empty";
            case Status::noSpace: return "no space left";
            case Status::ioError: return "I/O error";
        }

        return "unknown error";
    }

    std::string_view cwd() {
        return std::string_view(cwdPath, cwdPathLength);
    }

    Status changeDirectory(std::string_view path) {
        WalkResult result;
        Status status = resolveExisting(path.empty() ? std::string_view("/") : path, result);
        if (status != Status::ok) return status;
        if (!isDirectory(result.ref.entry)) return Status::notDirectory;

        uint32_t current = (!path.empty() && path[0] == '/') ? rootCluster : cwdCluster;
        char nextPath[sizeof(cwdPath)];
        size_t nextLength = 1;
        nextPath[0] = '/';
        nextPath[1] = 0;

        if (path.empty()) path = "/";

        if (path[0] != '/') {
            memcpy(nextPath, cwdPath, cwdPathLength + 1);
            nextLength = cwdPathLength;
        }

        size_t pos = 0;
        std::string_view component;

        while (nextPathComponent(path, pos, component)) {
            if (textEquals(component, ".")) continue;

            if (textEquals(component, "..")) {
                if (current == rootCluster) {
                    nextLength = 1;
                    nextPath[0] = '/';
                    nextPath[1] = 0;
                    continue;
                }

                char parentName[11];
                fillShortName(parentName, "..");

                EntryRef parentRef;
                status = findEntry(current, parentName, parentRef);
                if (status != Status::ok) return status;
                current = entryFirstCluster(parentRef.entry);

                if (nextLength > 1) {
                    if (nextPath[nextLength - 1] == '/') nextLength--;
                    while (nextLength > 1 && nextPath[nextLength - 1] != '/') nextLength--;
                    if (nextLength > 1) nextLength--;
                    nextPath[nextLength] = 0;
                }

                continue;
            }

            char shortName[11];
            if (!makeShortName(component, shortName)) return Status::invalidName;

            EntryRef ref;
            status = findEntry(current, shortName, ref);
            if (status != Status::ok) return status;
            if (!isDirectory(ref.entry)) return Status::notDirectory;

            char display[13];
            displayName(ref.entry.name, display);

            if (nextLength > 1 && nextPath[nextLength - 1] != '/') {
                if (nextLength + 1 >= sizeof(nextPath)) return Status::invalidName;
                nextPath[nextLength++] = '/';
            }

            for (size_t i = 0; display[i] != 0; i++) {
                if (nextLength + 1 >= sizeof(nextPath)) return Status::invalidName;
                nextPath[nextLength++] = display[i];
            }

            nextPath[nextLength] = 0;
            current = entryFirstCluster(ref.entry);
        }

        cwdCluster = result.cluster;
        memcpy(cwdPath, nextPath, nextLength + 1);
        cwdPathLength = nextLength;

        return Status::ok;
    }

    Status list(std::string_view path, ListCallback callback, void* context) {
        if (!callback) return Status::ioError;

        WalkResult result;
        Status status = resolveExisting(path.empty() ? std::string_view(".") : path, result);
        if (status != Status::ok) return status;

        if (!isDirectory(result.ref.entry)) {
            EntryInfo info;
            fillEntryInfo(result.ref.entry, info);
            callback(info, context);
            return Status::ok;
        }

        uint32_t cluster = result.cluster;
        uint8_t buffer[clusterBytes];

        while (validCluster(cluster)) {
            status = readCluster(cluster, buffer);
            if (status != Status::ok) return status;

            auto* entries = reinterpret_cast<DirectoryEntry*>(buffer);
            for (uint32_t index = 0; index < entriesPerCluster; index++) {
                DirectoryEntry& entry = entries[index];
                uint8_t first = static_cast<uint8_t>(entry.name[0]);

                if (first == 0x00) return Status::ok;
                if (first == 0xE5) continue;
                if ((entry.attr & attrLongName) == attrLongName) continue;
                if (entry.name[0] == '.' && entry.name[1] == ' ') continue;
                if (entry.name[0] == '.' && entry.name[1] == '.') continue;

                EntryInfo info;
                fillEntryInfo(entry, info);
                callback(info, context);
            }

            cluster = nextCluster(cluster);
        }

        return Status::ok;
    }

    Status readFile(std::string_view path, ChunkCallback callback, void* context) {
        if (!callback) return Status::ioError;

        WalkResult result;
        Status status = resolveExisting(path, result);
        if (status != Status::ok) return status;
        if (isDirectory(result.ref.entry)) return Status::isDirectory;

        uint32_t remaining = result.ref.entry.fileSize;
        uint32_t cluster = entryFirstCluster(result.ref.entry);
        uint8_t buffer[clusterBytes];

        while (remaining > 0) {
            if (cluster == 0) return Status::ioError;

            status = readCluster(cluster, buffer);
            if (status != Status::ok) return status;

            uint32_t count = remaining > clusterBytes ? clusterBytes : remaining;
            callback(std::string_view(reinterpret_cast<const char*>(buffer), count), context);

            remaining -= count;
            cluster = nextCluster(cluster);
        }

        return Status::ok;
    }

    Status writeFile(std::string_view path, std::string_view data, bool append) {
        uint32_t parentCluster = 0;
        char leafName[11];

        Status status = resolveParent(path, parentCluster, leafName);
        if (status != Status::ok) return status;

        EntryRef ref;
        status = findEntry(parentCluster, leafName, ref);

        if (status == Status::notFound) {
            uint32_t firstCluster = 0;
            status = ensureChainSize(firstCluster, static_cast<uint32_t>(data.size()));
            if (status != Status::ok) return status;

            status = writeData(firstCluster, 0, data);
            if (status != Status::ok) {
                if (firstCluster != 0) freeChain(firstCluster);
                return status;
            }

            return createEntry(parentCluster, leafName, attrArchive, firstCluster, static_cast<uint32_t>(data.size()));
        }

        if (status != Status::ok) return status;
        if (isDirectory(ref.entry)) return Status::isDirectory;

        uint32_t firstCluster = entryFirstCluster(ref.entry);
        uint32_t oldSize = append ? ref.entry.fileSize : 0;
        uint32_t newSize = oldSize + static_cast<uint32_t>(data.size());

        if (!append && firstCluster != 0) {
            freeChain(firstCluster);
            firstCluster = 0;
        }

        status = ensureChainSize(firstCluster, newSize);
        if (status != Status::ok) return status;

        status = writeData(firstCluster, oldSize, data);
        if (status != Status::ok) return status;

        setEntryFirstCluster(ref.entry, firstCluster);
        ref.entry.fileSize = newSize;

        return writeEntry(ref, ref.entry);
    }

    Status touch(std::string_view path) {
        uint32_t parentCluster = 0;
        char leafName[11];

        Status status = resolveParent(path, parentCluster, leafName);
        if (status != Status::ok) return status;

        EntryRef ref;
        status = findEntry(parentCluster, leafName, ref);
        if (status == Status::ok) {
            if (isDirectory(ref.entry)) return Status::isDirectory;
            return Status::ok;
        }
        if (status != Status::notFound) return status;

        return createEntry(parentCluster, leafName, attrArchive, 0, 0);
    }

    Status makeDirectory(std::string_view path) {
        uint32_t parentCluster = 0;
        char leafName[11];

        Status status = resolveParent(path, parentCluster, leafName);
        if (status != Status::ok) return status;

        EntryRef existing;
        status = findEntry(parentCluster, leafName, existing);
        if (status == Status::ok) return Status::alreadyExists;
        if (status != Status::notFound) return status;

        uint32_t newCluster = 0;
        status = allocateCluster(newCluster);
        if (status != Status::ok) return status;

        status = initializeDirectory(newCluster, parentCluster);
        if (status != Status::ok) {
            freeChain(newCluster);
            return status;
        }

        status = createEntry(parentCluster, leafName, attrDirectory, newCluster, 0);
        if (status != Status::ok) freeChain(newCluster);

        return status;
    }

    Status remove(std::string_view path) {
        if (path.empty() || textEquals(path, ".") || textEquals(path, "..")) return Status::invalidName;

        size_t pos = 0;
        std::string_view component;
        std::string_view leaf;

        while (nextPathComponent(path, pos, component)) leaf = component;
        if (textEquals(leaf, ".") || textEquals(leaf, "..")) return Status::invalidName;

        WalkResult result;
        Status status = resolveExisting(path, result);
        if (status != Status::ok) return status;
        if (result.ref.root || result.ref.synthetic) return Status::invalidName;

        if (isDirectory(result.ref.entry)) {
            uint32_t cluster = entryFirstCluster(result.ref.entry);
            if (!directoryEmpty(cluster)) return Status::directoryNotEmpty;
            freeChain(cluster);
        } else {
            uint32_t cluster = entryFirstCluster(result.ref.entry);
            if (cluster != 0) freeChain(cluster);
        }

        result.ref.entry.name[0] = static_cast<char>(0xE5);
        return writeEntry(result.ref, result.ref.entry);
    }

    Status stat(std::string_view path, EntryInfo& info) {
        WalkResult result;
        Status status = resolveExisting(path.empty() ? std::string_view(".") : path, result);
        if (status != Status::ok) return status;

        if (result.ref.root || result.ref.synthetic) {
            info.name[0] = result.cluster == rootCluster ? '/' : '.';
            info.name[1] = 0;
            info.directory = true;
            info.size = 0;
            info.cluster = result.cluster;
            return Status::ok;
        }

        return fillEntryInfo(result.ref.entry, info);
    }
}
