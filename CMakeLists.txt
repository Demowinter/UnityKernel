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
    -mno-red-zone
)

set(KERNEL_COMPILE_FLAGS_EXTRA)

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

# set(KERNEL64_SOURCES 
#     src/kernel64/asm/boot.s
#     src/kernel64/kernel64.cpp
# )



# set(KERNEL64_LINK_LIBS
#     "$<LINK_GROUP:RESCAN,libkstd64,libkbase64,libkrt64,libkernel64,libx86_64>"
#     gcc
# )



# add_executable(kernel64 ${KERNEL64_SOURCES})


# target_include_directories(kernel64 PRIVATE "include/")

# target_compile_options(kernel64 PRIVATE ${KERNEL_COMPILE_FLAGS} -m64)

# target_link_options(kernel64 PRIVATE ${KERNEL_LINK_FLAGS} -m64 -T ${CMAKE_SOURCE_DIR}/linker/kernel64.ld)

# target_link_libraries(kernel64 PRIVATE ${KERNEL64_LINK_LIBS})











# add_library(libx86_64 STATIC ${LIBX86_SOURCES})
# add_library(libkernel64 STATIC ${LIBKERNEL_SOURCES})
# add_library(libkbase64 STATIC ${LIBKBASE_SOURCES})
# add_library(libkrt64 STATIC ${LIBKRT_SOURCES})
# add_library(libkstd64 STATIC ${LIBKSTD_SOURCES})

# target_include_directories(libx86_64 PRIVATE "include/")
# target_include_directories(libkernel64 PRIVATE "include/")
# target_include_directories(libkbase64 PRIVATE "include/")
# target_include_directories(libkrt64 PRIVATE "include/")
# target_include_directories(libkstd64 PRIVATE "include/")

# target_compile_options(libx86_64 PRIVATE ${KERNEL_COMPILE_FLAGS} -m64)
# target_compile_options(libkernel64 PRIVATE ${KERNEL_COMPILE_FLAGS} -m64)
# target_compile_options(libkbase64 PRIVATE ${KERNEL_COMPILE_FLAGS} -m64)
# target_compile_options(libkrt64 PRIVATE ${KERNEL_COMPILE_FLAGS} -m64)
# target_compile_options(libkstd64 PRIVATE ${KERNEL_COMPILE_FLAGS} -m64)

# set_target_properties(libx86_64 PROPERTIES PREFIX "")
# set_target_properties(libkernel64 PROPERTIES PREFIX "")
# set_target_properties(libkbase64 PROPERTIES PREFIX "")
# set_target_properties(libkrt64 PROPERTIES PREFIX "")
# set_target_properties(libkstd64 PROPERTIES PREFIX "")


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