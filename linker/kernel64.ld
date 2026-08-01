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

    .bss ALIGN(16) : {
        *(COMMON)
        *(.bss)
    }

    .stack ALIGN(16) : {
        stack_bottom = .;

        . += 32K;
        .  = ALIGN(16); /* stack top ALWAYS must be aligned of 16 bytes! */

        stack_top = .;
    }
}