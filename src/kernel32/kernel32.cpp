#include <memory>
#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <kernel32/grub.hpp>
#include <libkrt/krt.hpp>
#include <libkernel/cpu.hpp>
#include <libkernel/memory.hpp>
#include <libkernel/console.hpp>
#include <libkstd/string.hpp>

KernelSTD::String string = "Global init string!";

void testStatic() {
        static KernelSTD::String staticString = "Hello from static string!";

        Kernel::Console::ok(staticString);
}

namespace Kernel {
    extern "C" [[noreturn]] void kernel32(uint32_t mbMagic, GRUB::MultibootInfo* mbInfo) {
        // Console::clear();
        // Console::info("Starting kernel32...");

        Memory::initialize();
        KernelRT::initialize();

        if (!GRUB::checkMultiboot(mbMagic)) Console::info("Multiboot structure is corrupted");
        else Console::info("Multiboot structure is OK");

        CPU::Info info = CPU::cpuid(0);

        Console::info("CPU manufacturer: ", false);
        Console::println(CPU::manufacturer(info), 0x05);
        // Console::newline();

        // Console::ok("Started kernel32");
        // delete[] name;

        // Console::newline();

        // Console::print("Welcome to ");
        // Console::println("UnityKernel! v0.1.0-alpha", 5);

        // KernelSTD::String str = "Hello from KernelSTD::String!";
        // KernelSTD::ForwardStringHandler handler(str.data());

        // KernelSTD::ForwardIterator<char, KernelSTD::ForwardStringHandler, KernelSTD::ForwardIterator> it{handler};
        // std::string str2 = "Hello from std";

        // Console::ok(str2);

        // str2.begin();
        // str2.end();
        // Console::println({str.c_str(), str.size()});

        // std::vector<int> v;
        // std::map<int, int> m;

        // v.begin();
        // m.begin();

        testStatic();
        testStatic();

        {
            KernelSTD::String str = "Hello, World! And hello, Wordle! :D";
            KernelSTD::String str2 = str.substr(str.find("Wordle").value(), 7);

            Console::info(str);
            Console::info("What is the best game ever? ", false);
            Console::println(str2);

            str2.insert(str2.end(), " Hi!");
            Console::info(str2);

            std::reverse(str2.begin(), str2.end());
            Console::info(str2);
        }

        Console::info(string);

        KernelRT::finalize();

        CPU::interrupts(false);
        CPU::halt();
    }
}