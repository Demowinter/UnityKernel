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
#include <libkstd/vector.hpp>
#include <libkstd/list.hpp>

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

        Console::ok("Started kernel32");

        Console::newline();

        Console::print("Welcome to ");
        Console::println("UnityKernel! v0.1.0-alpha", 5);

        KernelSTD::String str = "Hello from KernelSTD::String!";

        Console::ok(str);

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

        {

            KernelSTD::Vector<KernelSTD::String> vect = {"Hello", "World", "Slava", "Ukraini"};
            vect.push_back("putin");
            vect.push_back("huilo");

            Console::info("Trying to print vector...");

            for (auto str : vect) Console::ok(str);
        }

        KernelSTD::LinkedList<KernelSTD::String> list = {"Unity", "Kernel"};

        for (auto str : list) Console::ok(str);

        std::string_view view = "Hello from string in std::string_view";
        KernelSTD::String str3 = view;

        Console::info(str3);

        str3.at(-1); // invalid index

        KernelRT::finalize();

        CPU::interrupts(false);
        CPU::halt();
    }
}