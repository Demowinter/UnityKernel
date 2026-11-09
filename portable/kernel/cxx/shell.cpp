#include <kernel/shell.hpp>
#include <kernel/console.hpp>
#include <kernel/filesystem.hpp>
#include <kernel/system.hpp>
#include <kernel/dformat.hpp>
#include <kernel/subsystems/driver.hpp>
#include <libarch/cpu.hpp>
#include <libarch/timer.hpp>
#include <libplatform/pci.hpp>
#include <libstd/string.hpp>
#include <libbase/string.hpp>
#include <drivers/ahci/ahci.hpp>

namespace Kernel::Shell {
    //Compare strings
    static bool strcmp(std::string_view a, std::string_view b) {
        return a.size() == b.size() && 
               __builtin_memcmp(a.data(), b.data(), a.size()) == 0;
    }

    //Trim whitespace from the left
    static std::string_view ltrim(std::string_view str) {
        while (!str.empty() && (str[0] == ' ' || str[0] == '\t')) {
            str.remove_prefix(1);
        }
        return str;
    }

    //Trim whitespace from the right
    static std::string_view rtrim(std::string_view str) {
        while (!str.empty() && (str.back() == ' ' || str.back() == '\t')) {
            str.remove_suffix(1);
        }
        return str;
    }

    //Trim both sides
    static std::string_view trim(std::string_view str) {
        return rtrim(ltrim(str));
    }

    //Extract first word from command
    static std::string_view getFirstWord(std::string_view cmd) {
        cmd = ltrim(cmd);
        size_t end = 0;
        while (end < cmd.size() && cmd[end] != ' ' && cmd[end] != '\t') {
            end++;
        }
        return cmd.substr(0, end);
    }

    //Get remaining arguments after first word
    static std::string_view getArgs(std::string_view cmd) {
        cmd = ltrim(cmd);

        size_t wordLength = 0;
        while (wordLength < cmd.size() &&
               cmd[wordLength] != ' ' && cmd[wordLength] != '\t') {
            wordLength++;
        }
        if (wordLength == cmd.size()) return "";

        return trim(cmd.substr(wordLength));
    }

    static void printView(std::string_view text) {
        Console::print(text);
    }

    static void printFsError(std::string_view command, std::string_view target, Filesystem::Status status) {
        if (status == Filesystem::Status::ok) return;

        Console::print(command);
        if (!target.empty()) {
            Console::print(": ");
            Console::print(target);
        }
        Console::print(": ");
        Console::println(Filesystem::statusText(status));
    }

    static void printEntry(const Filesystem::EntryInfo& entry, void*) {
        Console::print(entry.directory ? "d " : "- ");
        Console::print(STDLib::toString(entry.size));
        Console::print(" ");
        Console::println(entry.name);
    }

    static void printChunk(std::string_view chunk, void*) {
        printView(chunk);
    }

    static void cmdHelp(std::string_view args) {
        (void)args;  // Unused
        Console::println("=== UnityKernel Shell Commands ===");
        Console::println("");
        Console::println("help              Show this help message");
        Console::println("clear             Clear the screen");
        Console::println("echo <text>       Print text to console");
        Console::println("uname             Print kernel info");
        Console::println("cpuid             Print cpu manufacturer");
        Console::println("sysinfo           Print system information");
        Console::println("dformat           Interactively select a SATA disk and filesystem to format");
        Console::println("mnt               Mount the selected SATA filesystem");
        Console::println("unmnt             Unmount SATA and return to the ext4 RAM filesystem");
        Console::println("pwd               Print current directory");
        Console::println("ls [path]         List directory contents");
        Console::println("cd <path>         Change directory");
        Console::println("cat <file>        Print file contents");
        Console::println("touch <file>      Create an empty file");
        Console::println("write <file> <s>  Replace file with text");
        Console::println("append <file> <s> Append text to file");
        Console::println("mkdir <dir>       Create a directory");
        Console::println("rm <path>         Remove a file or empty directory");
        Console::println("stat <path>       Show file metadata");
        Console::println("lspci             Show PCIe device list");
        Console::println("reboot            Reboot the system");
        Console::println("shutdown          Halt the system");
        Console::println("");
    }

    static void cmdClear(std::string_view args) {
        (void)args;  // Unused
        Console::clear();
    }

    static void cmdEcho(std::string_view args) {
        if (args.empty()) {
            Console::println("echo: no text provided");
            return;
        }
        Console::println(args);
    }

    static void cmdUname(std::string_view args) {
        Console::println("UnityKernel v0.1.0-alpha");
    }

    static void cmdCPUID(std::string_view args) {
        Console::println(Arch::CPU::manufacturer());
    }

    static void sysinfo(std::string_view args) {
    (void)args;

    static const char* art[] = {
    "  _    _ ",
    " | |  | |",
    " | |  | |",
    " | |  | |",
    " | |__| |",
    "  \\____/ ",
};
    constexpr size_t lines    = sizeof(art) / sizeof(art[0]);
    constexpr size_t artWidth = 12;

    for (size_t i = 0; i < lines; ++i) {
        const char* line = art[i];
        size_t len = 0;
        while (line[len]) ++len;

        Console::print(line);
        for (size_t j = len; j < artWidth; ++j) {
            Console::print(" ");
        }

        switch (i) {
            case 0: Console::println("user@unitykernel");          break;
            case 1: Console::println("------------------");        break;
            case 2: Console::println("OS:        UnityKernel");    break;
            case 3: Console::println("Version:   v0.1.0-alpha");   break;
            case 4: Console::println("Kernel:    v0.1.0-alpha");   break;
            case 5: Console::print  ("CPU:       ");
                    Console::println(Arch::CPU::manufacturer());   break;
            default: Console::newline();                           break;
        }
    }
}

    static void cmdPwd(std::string_view args) {
        (void)args;
        Console::println(Filesystem::cwd());
    }

        static void dformat(std::string_view args) {
            if (!trim(args).empty()) {
                Console::println("Usage: dformat");
                return;
            }

            auto* device = static_cast<Driver::AHCI::AHCIDriver*>(
                DriverSubsystem::loadDriver(Driver::Category::BLK, Driver::Type::AHCI));
            if (device == nullptr) {
                Console::println("dformat: no supported SATA devices detected");
                return;
            }

            Console::println("Detected storage devices:");
            Console::print("  1) SATA disk on AHCI port ");
            Console::print(STDLib::toString(device->portNumber()));
            Console::print(" (");
            Console::print(STDLib::toString(device->sectors(0)));
            Console::println(" sectors)");
            Console::println("Select a device number, or press Enter to cancel:");

            char input[256];
            size_t inputLength = Console::readline(input, sizeof(input));
            std::string_view response = trim(std::string_view(input, inputLength));
            if (response.empty()) {
                Console::println("dformat: cancelled");
                return;
            }
            if (!strcmp(response, "1")) {
                Console::println("dformat: invalid device selection");
                return;
            }

            Console::println("Select a filesystem:");
            Console::println("  1) FAT32");
            Console::println("  2) ext4");
            Console::println("Enter 1 or 2, or press Enter to cancel:");
            inputLength = Console::readline(input, sizeof(input));
            response = trim(std::string_view(input, inputLength));
            const bool fat32 = strcmp(response, "1");
            const bool ext4 = strcmp(response, "2");
            if (response.empty()) {
                Console::println("dformat: cancelled");
                return;
            }
            if (!fat32 && !ext4) {
                Console::println("dformat: invalid filesystem selection");
                return;
            }

            Console::warn("Formatting permanently destroys data on the selected disk.");
            Console::warn("Back up any files you need before continuing.");
            Console::print("Selected: AHCI port ");
            Console::print(STDLib::toString(device->portNumber()));
            Console::print(", ");
            Console::println(fat32 ? "FAT32" : "ext4");
            Console::println("Type FORMAT exactly and press Enter to proceed; anything else cancels:");
            inputLength = Console::readline(input, sizeof(input));
            response = trim(std::string_view(input, inputLength));
            if (!strcmp(response, "FORMAT")) {
                Console::println("dformat: cancelled");
                return;
            }

            Console::println("Formatting selected SATA disk...");
            const Filesystem::Status switchStatus = Filesystem::prepareDiskFormat();
            if (switchStatus != Filesystem::Status::ok) {
                printFsError("dformat", "SATA", switchStatus);
                return;
            }

            const DFORMAT::Status status = fat32
                ? DFORMAT::partitionAndFormatFat32(*device)
                : DFORMAT::formatExt4(*device);
            if (status == DFORMAT::Status::ok) {
                Filesystem::setPreferredType(
                    fat32 ? Filesystem::Type::fat32 : Filesystem::Type::ext4);
                Console::print("dformat: ");
                Console::print(fat32 ? "FAT32 partition" : "ext4 volume");
                Console::println(" created successfully");
            } else {
                Console::print("dformat: ");
                Console::println(DFORMAT::statusText(status));
            }
        }

    static void cmdMnt(std::string_view args) {
        if (!args.empty()) {
            Console::println("Usage: mnt");
            return;
        }

        auto* device = static_cast<Driver::AHCI::AHCIDriver*>(
            DriverSubsystem::loadDriver(Driver::Category::BLK, Driver::Type::AHCI));
        if (device == nullptr) {
            Console::println("mnt: SATA disk is unavailable");
            return;
        }

        Filesystem::Status status = Filesystem::mountSata(
            Filesystem::preferredType(), *device);
        if (status == Filesystem::Status::ok) {
            Console::println(Filesystem::preferredType() == Filesystem::Type::ext4
                ? "mnt: SATA ext4 filesystem mounted"
                : "mnt: SATA FAT32 filesystem mounted");
        } else {
            printFsError("mnt", "SATA", status);
        }
    }

    static void cmdUnmnt(std::string_view args) {
        if (!args.empty()) {
            Console::println("Usage: unmnt");
            return;
        }

        Filesystem::Status status = Filesystem::unmountSata();
        if (status == Filesystem::Status::ok) {
            Console::println("unmnt: returned to the ext4 RAM filesystem");
        } else {
            printFsError("unmnt", "SATA", status);
        }
    }

    static void cmdLs(std::string_view args) {
        Filesystem::Status status = Filesystem::list(args.empty() ? std::string_view(".") : args, printEntry, nullptr);
        printFsError("ls", args, status);
    }

    static void cmdCd(std::string_view args) {
        Filesystem::Status status = Filesystem::changeDirectory(args.empty() ? std::string_view("/") : args);
        printFsError("cd", args, status);
    }

    static void cmdCat(std::string_view args) {
        if (args.empty()) {
            Console::println("cat: missing file");
            return;
        }

        Filesystem::Status status = Filesystem::readFile(args, printChunk, nullptr);
        if (status == Filesystem::Status::ok) Console::newline();
        else printFsError("cat", args, status);
    }

    static void cmdTouch(std::string_view args) {
        if (args.empty()) {
            Console::println("touch: missing file");
            return;
        }

        Filesystem::Status status = Filesystem::touch(args);
        printFsError("touch", args, status);
    }

    static void cmdWrite(std::string_view args) {
        std::string_view path = getFirstWord(args);
        std::string_view text = getArgs(args);

        if (path.empty()) {
            Console::println("write: missing file");
            return;
        }

        Filesystem::Status status = Filesystem::writeFile(path, text, false);
        printFsError("write", path, status);
    }

    static void cmdAppend(std::string_view args) {
        std::string_view path = getFirstWord(args);
        std::string_view text = getArgs(args);

        if (path.empty()) {
            Console::println("append: missing file");
            return;
        }

        Filesystem::Status status = Filesystem::writeFile(path, text, true);
        printFsError("append", path, status);
    }

    static void cmdMkdir(std::string_view args) {
        if (args.empty()) {
            Console::println("mkdir: missing directory");
            return;
        }

        Filesystem::Status status = Filesystem::makeDirectory(args);
        printFsError("mkdir", args, status);
    }

    static void cmdRm(std::string_view args) {
        if (args.empty()) {
            Console::println("rm: missing path");
            return;
        }

        Filesystem::Status status = Filesystem::remove(args);
        printFsError("rm", args, status);
    }

    static void cmdStat(std::string_view args) {
        if (args.empty()) {
            Console::println("stat: missing path");
            return;
        }

        Filesystem::EntryInfo info;
        Filesystem::Status status = Filesystem::stat(args, info);
        if (status != Filesystem::Status::ok) {
            printFsError("stat", args, status);
            return;
        }

        Console::print(info.directory ? "directory " : "file ");
        Console::println(info.name);
        Console::print("size: ");
        Console::println(STDLib::toString(info.size));
        Console::print("id: ");
        Console::println(STDLib::toString(info.cluster));
    }

    static void cmdLspci([[maybe_unused]] std::string_view args) {
        Console::println("PCIe devices on bus 0:");

        for (uint32_t device = 0; device < 32; device++) {
            auto vendor = Platform::PCI::readConfig<uint16_t>(0, device, 0, 0x00);
            auto deviceid = Platform::PCI::readConfig<uint16_t>(0, device, 0, 0x02);

            if (!vendor || *vendor == 0xFFFF) continue;
            if (!deviceid) continue;

            Console::print("  ");
            Console::print("00:0");
            Console::print(STDLib::toString(device));
            Console::print(".0");
            Console::print(" Vendor=");
            Console::print(STDLib::hex(*vendor));
            Console::print(" DeviceID=");
            Console::println(STDLib::hex(*deviceid));
        }
    }

    static void cmdReboot(std::string_view args) {
        (void)args;  // Unused
        Console::println("Rebooting...");
        System::reboot();
    }

   static void cmdShutdown(std::string_view args) {
        (void)args;  // Unused
        Console::println("Are you sure you want to shutdown? (y/n)");

        char ch = Console::read();

        Console::putchar(ch);
        Console::newline();

        if (ch == 'y' || ch == 'Y') {
            Console::println("Shutting down...");
            System::shutdown();
        } else {
            Console::println("Shutdown cancelled.");
        }
    }

    void executeCommand(std::string_view cmd) {
        cmd = trim(cmd);
        
        if (cmd.empty()) {
            return;
        }

        std::string_view command = getFirstWord(cmd);
        std::string_view args = getArgs(cmd);

        if (strcmp(command, "help")) {
            cmdHelp(args);
        } else if (strcmp(command, "clear")) {
            cmdClear(args);
        } else if (strcmp(command, "echo")) {
            cmdEcho(args);
        } else if (strcmp(command, "uname")) {
            cmdUname(args);
        } else if (strcmp(command, "cpuid")) {
            cmdCPUID(args);
        } else if (strcmp(command, "sysinfo")) {
            sysinfo(args);
        } else if (strcmp(command, "dformat")) {
            dformat(args);
        } else if (strcmp(command, "mnt")) {
            cmdMnt(args);
        } else if (strcmp(command, "unmnt")) {
            cmdUnmnt(args);
        } else if (strcmp(command, "pwd")) {
            cmdPwd(args);
        } else if (strcmp(command, "ls")) {
            cmdLs(args);
        } else if (strcmp(command, "cd")) {
            cmdCd(args);
        } else if (strcmp(command, "cat")) {
            cmdCat(args);
        } else if (strcmp(command, "touch")) {
            cmdTouch(args);
        } else if (strcmp(command, "write")) {
            cmdWrite(args);
        } else if (strcmp(command, "append")) {
            cmdAppend(args);
        } else if (strcmp(command, "mkdir")) {
            cmdMkdir(args);
        } else if (strcmp(command, "rm")) {
            cmdRm(args);
        } else if (strcmp(command, "stat")) {
            cmdStat(args);
        } else if (strcmp(command, "lspci")) {
            cmdLspci(args);
        } else if (strcmp(command, "reboot")) {
            cmdReboot(args);
        } else if (strcmp(command, "shutdown")) {
            cmdShutdown(args);
        } else {
            Console::print("Unknown command: ");
            Console::println(command);
        }
    }

    void run(const char* kcmdline) {
    Arch::Timer::sleep(1000);

    Console::clear();
    Console::println("UnityKernel v0.1.0-alpha");
    Console::println(" ");

    //ASCII art banner for UnityKernel Shell
    static const char* banner[] = {
        " ###    ### ######      ### ### ############# ###     ###",
        " ###    ### ### ###     ### ### #############  ###   ###",
        " ###    ### ###  ###    ### ###      ###         #####",
        " ###    ### ###   ###   ### ###      ###          ###",
        " ###    ### ###    ###  ### ###      ###          ###",
        "  ##    ##  ###     ### ### ###      ###          ###",
        "    ####    ###      ###### ###      ###          ###",
        " ",
        "   ######   ###   ### ######### ###       ###",
        " ###    ### ###   ### ######### ###       ###",
        "  ###       ###   ### ###       ###       ###",
        "   ###      ######### ######### ###       ###",
        "     ###    ######### ######### ###       ###",
        "       ###  ###   ### ###       ###       ###",
        " ###    ### ###   ### ######### ######### #########",
        "   ######   ###   ### ######### ######### #########",
    };

    for (const char* line : banner) {
        Console::println(line);
        Arch::Timer::sleep(30);
    }
    Console::println(" ");
    Console::println("Welcome to UnityKernel Shell!");
    Arch::Timer::sleep(1000);

    Console::clear();
    Console::print("Welcome to UnityKernel Shell! ");
    Console::println(kcmdline);
    Console::println("Type 'help' for available commands");
    Console::newline();

    static constexpr size_t bufferSize = 256;
    char inputBuffer[bufferSize];

    while (true) {
        Console::print(Filesystem::cwd());
        Console::print(" $ ");

        size_t len = Console::readline(inputBuffer, bufferSize);

        if (len > 0) {
            executeCommand(std::string_view(inputBuffer, len));
        }
    }
}
}
