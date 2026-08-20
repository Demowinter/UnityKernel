cmake_minimum_required(VERSION 3.25)
project(UnityKernel LANGUAGES CXX ASM)

set(TARGET_ARCH x86)
set(TARGET_PLATFORM pc)
set(TARGET_BOOTLOADER grub)

set(KERNEL_COMPILE_FLAGS
    -O2
    -g
    -Wall
    -ffreestanding
    -fno-exceptions
    -fno-rtti
    -fno-pic 
    -fno-pie
    -fno-stack-protector
    -fno-builtin
    -fno-asynchronous-unwind-tables
    -fno-unwind-tables
)

set(KERNEL_LINK_FLAGS
    -no-pie
    -nostdlib
    -g
)

add_subdirectory(specific/libarch/${TARGET_ARCH}/)
add_subdirectory(specific/libplatform/${TARGET_PLATFORM}/${TARGET_ARCH}/)

add_subdirectory(portable/kernel)
add_subdirectory(portable/libkernel)
add_subdirectory(portable/libkbase)
add_subdirectory(portable/libkrt)
add_subdirectory(portable/libkstd)


add_custom_target(iso
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/iso/boot/grub

    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:kernel>
        ${CMAKE_BINARY_DIR}/iso/boot/kernel

    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:kernel>
        ${CMAKE_BINARY_DIR}/iso/boot/kernel

    # COMMAND ${CMAKE_COMMAND} -E copy
    #     $<TARGET_FILE:kernel64>
    #     ${CMAKE_BINARY_DIR}/iso/boot/kernel64

    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/grub.cfg
        ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg

    COMMAND grub-mkrescue
        -o ${CMAKE_BINARY_DIR}/kernel.iso
        ${CMAKE_BINARY_DIR}/iso

    DEPENDS
        kernel
        # kernel64
)

add_custom_target(run-bios
    COMMAND qemu-system-x86_64
    -cdrom ${CMAKE_BINARY_DIR}/kernel.iso -enable-kvm -cpu host
    -serial stdio

    DEPENDS iso
)

add_custom_target(run-bios-gdb
    COMMAND qemu-system-x86_64
    -cdrom ${CMAKE_BINARY_DIR}/kernel.iso -enable-kvm -cpu host
    -S -s
    -serial stdio

    DEPENDS iso
)

add_custom_target(run-uefi
    COMMAND qemu-system-x86_64
    -cdrom ${CMAKE_BINARY_DIR}/kernel.iso
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd
    -drive if=pflash,format=raw,file=${CMAKE_SOURCE_DIR}/utils/OVMF_VARS.fd
    -accel tcg
    -no-reboot
    -d in_asm,int,mmu,guest_errors,unimp,cpu_reset
    -D qemu.log

    DEPENDS iso
)

add_custom_target(run-uefi-gdb
    COMMAND qemu-system-x86_64
    -cdrom ${CMAKE_BINARY_DIR}/kernel.iso
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd
    -drive if=pflash,format=raw,file=${CMAKE_SOURCE_DIR}/utils/OVMF_VARS.fd
    -accel tcg
    -S -s
    -no-reboot
    -d in_asm,int,mmu,guest_errors,unimp,cpu_reset
    -D qemu.log
    

    DEPENDS iso
)

# add_custom_target(run-arm64
#     COMMAND qemu-system-arm
#     -machine virt
#     -m 128M
#     -device loader,file=${CMAKE_BINARY_DIR}/portable/kernel/kernel
#     -S
#     -gdb tcp::1234

#     DEPENDS iso
# )