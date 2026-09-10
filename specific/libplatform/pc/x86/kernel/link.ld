OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)

ENTRY(kernelMain)

SECTIONS {
    . = 8M;

    .text : ALIGN(16) {
        *(.multiboot)
        *(.text)
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

    .endsect : {__sections_end = .;}
}