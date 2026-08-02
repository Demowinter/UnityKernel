OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

SECTIONS {
    . = 1M;

    .text ALIGN(16) : {
        *(.multiboot)
        *(.text)
    }

    .rodata ALIGN(16) : {*(.rodata)}

    .data ALIGN(16) : {*(.data)}

    .init_array ALIGN(16) : {
        __init_array_start = .;

        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))

        __init_array_end = .;
    }

    .bss ALIGN(16) : {
        *(COMMON)
        *(.bss)
    }

    .heap ALIGN(16) : {
        __heap_bottom = .;

        . += 1M;
        .  = ALIGN(16);

        __heap_top = .;
    }

    .stack ALIGN(16) : {
        __stack_bottom = .;

        . += 32K;
        .  = ALIGN(16); /* stack top ALWAYS must be aligned of 16 bytes! */

        __stack_top = .;
    }
}