cmake_minimum_required(VERSION 3.25)
project(kernel LANGUAGES CXX ASM)

set(KERNEL_FLAGS
    -O2
    -Wall
    -ffreestanding
    -fno-exceptions
    -fno-rtti
    -fno-pic 
    -fno-pie
    -fno-stack-protector
    -mno-red-zone
)

add_subdirectory(libkrt)
add_subdirectory(libkstd)
add_subdirectory(libkernel)
add_subdirectory(kernel0)
add_subdirectory(kernel64)

add_custom_target(iso
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/iso/boot/grub

    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:kernel0>
        ${CMAKE_BINARY_DIR}/iso/boot/kernel0

    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:kernel0>
        ${CMAKE_BINARY_DIR}/iso/boot/kernel0

    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:kernel64>
        ${CMAKE_BINARY_DIR}/iso/boot/kernel64

    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/grub.cfg
        ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg

    COMMAND grub-mkrescue
        -o ${CMAKE_BINARY_DIR}/kernel.iso
        ${CMAKE_BINARY_DIR}/iso

    DEPENDS
        kernel0
        kernel64
)

add_custom_target(run-bios
    COMMAND qemu-system-x86_64
    -cdrom ${CMAKE_BINARY_DIR}/kernel.iso -enable-kvm -cpu host

    DEPENDS iso
)

add_custom_target(run-uefi
    COMMAND qemu-system-x86_64
    -cdrom ${CMAKE_BINARY_DIR}/kernel.iso -enable-kvm -cpu host
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd
    -drive if=pflash,format=raw,file=${CMAKE_SOURCE_DIR}/utils/OVMF_VARS.fd

    DEPENDS iso
)