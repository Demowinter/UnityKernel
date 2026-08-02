.code64
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

.extern kernel64

.extern __stack_top

_start:
    mov $__stack_top, %rsp

    mov %rax, %rdi
    mov %rbx, %rsi

    call kernel64

    cli

hang:
    hlt
    jmp hang
