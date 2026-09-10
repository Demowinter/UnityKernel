#include <kernel/shell.hpp>
#include <kernel/console.hpp>
#include <kernel/system.hpp>
#include <libstd/string.hpp>
#include <libbase/string.hpp>
#include <cstring>

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

    static void cmdHelp(std::string_view args) {
        (void)args;  // Unused
        Console::println("=== UnityKernel Shell Commands ===");
        Console::println("");
        Console::println("help              Show this help message");
        Console::println("clear             Clear the screen");
        Console::println("echo <text>       Print text to console");
        Console::println("reboot            Reboot the system");
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

    static void cmdReboot(std::string_view args) {
        (void)args;  // Unused
        Console::println("Rebooting...");
        System::reboot();
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
        } else if (strcmp(command, "reboot")) {
            cmdReboot(args);
        } else {
            Console::print("Unknown command: ");
            Console::println(command);
        }
    }

    void run() {
        Console::clear();
        Console::println("=== UnityKernel Shell ===");
        Console::println("Type 'help' for available commands");
        Console::println("");

        static constexpr size_t bufferSize = 256;
        char inputBuffer[bufferSize];

        while (true) {
            Console::print("> ");
            Console::newline();
            
            size_t len = Console::readline(inputBuffer, bufferSize);
            
            if (len > 0) {
                executeCommand(std::string_view(inputBuffer, len));
            }
        }
    }
}
