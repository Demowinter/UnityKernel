OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)

SECTIONS
{
    . = 0x40200000;

    .text : ALIGN(16) {
        KEEP(*(.text.boot))
        *(.text*)
    }

    .rodata : ALIGN(16) {*(.rodata)}

    .data : ALIGN(16) {*(.data)}

    .dso : ALIGN(16) {__dso_handle = .;}

    .init_array : ALIGN(16) {
        __init_array_start = .;

        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))

        __init_array_end = .;
    }

    .bss : ALIGN(16) {
        *(COMMON)
        *(.bss*)
    }

    .heap (NOLOAD) : ALIGN(16) {
        __heap_start = .;

        . += 1M;
        .  = ALIGN(16);

        __heap_end = .;
    }

    .stack (NOLOAD) : ALIGN(16) {
        __stack_bottom = .;

        . += 32K;
        .  = ALIGN(16); /* stack top ALWAYS must be aligned of 16 bytes! */

        __stack_top = .;
    }
}