OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)

ENTRY(_start)

SECTIONS {
    . = 1M;

    .text : {
        *(.multiboot)
        *(.text)
    }

    .rodata : {*(.rodata)}

    .data : {*(.data)}

    .bss : {
        *(COMMON)
        *(.bss)
    }

    .stack ALIGN(16) : {
        stack_bottom = .;
        . += 16K;
        stack_top = .;
    }
}