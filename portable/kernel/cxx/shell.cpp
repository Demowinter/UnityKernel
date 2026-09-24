#include <kernel/shell.hpp>
#include <kernel/console.hpp>
#include <kernel/fat32.hpp>
#include <kernel/system.hpp>
#include <libarch/api.hpp>
#include <libstd/string.hpp>
#include <libbase/string.hpp>

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
        std::string_view word = getFirstWord(cmd);
        if (word.empty()) return "";
        return ltrim(cmd.substr(word.size()));
    }

    static void printView(std::string_view text) {
        for (char ch : text) Console::putchar(ch);
    }

    static void printFsError(std::string_view command, std::string_view target, FAT32::Status status) {
        if (status == FAT32::Status::ok) return;

        Console::print(command);
        if (!target.empty()) {
            Console::print(": ");
            Console::print(target);
        }
        Console::print(": ");
        Console::println(FAT32::statusText(status));
    }

    static void printEntry(const FAT32::EntryInfo& entry, void*) {
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
        (void)args;  // Unused
        Console::println("=== System Information ===");
        Console::print("CPU Manufacturer: ");
        Console::println(Arch::CPU::manufacturer());
        Console::print("Kernel Version: ");
        Console::println("v0.1.0-alpha");
    }

    static void cmdPwd(std::string_view args) {
        (void)args;
        Console::println(FAT32::cwd());
    }

    static void cmdLs(std::string_view args) {
        FAT32::Status status = FAT32::list(args.empty() ? std::string_view(".") : args, printEntry, nullptr);
        printFsError("ls", args, status);
    }

    static void cmdCd(std::string_view args) {
        FAT32::Status status = FAT32::changeDirectory(args.empty() ? std::string_view("/") : args);
        printFsError("cd", args, status);
    }

    static void cmdCat(std::string_view args) {
        if (args.empty()) {
            Console::println("cat: missing file");
            return;
        }

        FAT32::Status status = FAT32::readFile(args, printChunk, nullptr);
        if (status == FAT32::Status::ok) Console::newline();
        else printFsError("cat", args, status);
    }

    static void cmdTouch(std::string_view args) {
        if (args.empty()) {
            Console::println("touch: missing file");
            return;
        }

        FAT32::Status status = FAT32::touch(args);
        printFsError("touch", args, status);
    }

    static void cmdWrite(std::string_view args) {
        std::string_view path = getFirstWord(args);
        std::string_view text = getArgs(args);

        if (path.empty()) {
            Console::println("write: missing file");
            return;
        }

        FAT32::Status status = FAT32::writeFile(path, text, false);
        printFsError("write", path, status);
    }

    static void cmdAppend(std::string_view args) {
        std::string_view path = getFirstWord(args);
        std::string_view text = getArgs(args);

        if (path.empty()) {
            Console::println("append: missing file");
            return;
        }

        FAT32::Status status = FAT32::writeFile(path, text, true);
        printFsError("append", path, status);
    }

    static void cmdMkdir(std::string_view args) {
        if (args.empty()) {
            Console::println("mkdir: missing directory");
            return;
        }

        FAT32::Status status = FAT32::makeDirectory(args);
        printFsError("mkdir", args, status);
    }

    static void cmdRm(std::string_view args) {
        if (args.empty()) {
            Console::println("rm: missing path");
            return;
        }

        FAT32::Status status = FAT32::remove(args);
        printFsError("rm", args, status);
    }

    static void cmdStat(std::string_view args) {
        if (args.empty()) {
            Console::println("stat: missing path");
            return;
        }

        FAT32::EntryInfo info;
        FAT32::Status status = FAT32::stat(args, info);
        if (status != FAT32::Status::ok) {
            printFsError("stat", args, status);
            return;
        }

        Console::print(info.directory ? "directory " : "file ");
        Console::println(info.name);
        Console::print("size: ");
        Console::println(STDLib::toString(info.size));
        Console::print("cluster: ");
        Console::println(STDLib::toString(info.cluster));
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
        Console::clear();
        Console::println("=== UnityKernel Shell ===");
        Console::print("Kernel command line: ");
        Console::println(kcmdline);
        Console::println("Type 'help' for available commands");
        Console::newline();

        static constexpr size_t bufferSize = 256;
        char inputBuffer[bufferSize];

        while (true) {
            Console::print(FAT32::cwd());
            Console::print(" $ ");
            
            size_t len = Console::readline(inputBuffer, bufferSize);
            
            if (len > 0) {
                executeCommand(std::string_view(inputBuffer, len));
            }
        }
    }
}
