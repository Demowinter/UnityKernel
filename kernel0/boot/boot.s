.section .multiboot
.align 8 #  заголовок multiboot повинен бути завжди вирівняний по 8 байтах!

header_start:

.long 0xE85250D6
.long 0
.long header_end - header_start
.long -(0xE85250D6 + 0 + (header_end - header_start))

.short 0
.short 0
.long 8

header_end:

.section .text

.global _start

.extern kernel0

.extern stack_top
.extern stack_bottom

_start:
    mov $stack_top, %esp

    push %ebx
    push %eax

    call kernel0

    cli

hang:
    hlt
    jmp hang
