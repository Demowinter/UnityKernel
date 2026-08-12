cmake_minimum_required(VERSION 3.25)
project(UnityKernel LANGUAGES CXX ASM)

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

set(KERNEL_LINK_FLAGS
    -no-pie
    -nostdlib
    -g
)

set(KERNEL64_SOURCES 
    src/kernel64/asm/boot.s
    src/kernel64/kernel64.cpp
)

set(KERNEL32_SOURCES 
    src/kernel32/asm/boot.s
    src/kernel32/grub.cpp
    src/kernel32/kernel32.cpp
)

set(KERNEL64_LINK_LIBS
    "$<LINK_GROUP:RESCAN,libkstd64,libkbase64,libkrt64,libkernel64>"
    gcc
)

set(KERNEL32_LINK_LIBS
    "$<LINK_GROUP:RESCAN,libkstd32,libkbase32,libkrt32,libkernel32>"
    gcc
)

add_executable(kernel64 ${KERNEL64_SOURCES})
add_executable(kernel32 ${KERNEL32_SOURCES})

target_include_directories(kernel64 PRIVATE "include/")
target_include_directories(kernel32 PRIVATE "include/")

target_compile_options(kernel64 PRIVATE ${KERNEL_COMPILE_FLAGS} -m64)
target_compile_options(kernel32 PRIVATE ${KERNEL_COMPILE_FLAGS} -m32)

target_link_options(kernel64 PRIVATE ${KERNEL_LINK_FLAGS} -m64 -T ${CMAKE_SOURCE_DIR}/linker/kernel64.ld)
target_link_options(kernel32 PRIVATE ${KERNEL_LINK_FLAGS} -m32 -T ${CMAKE_SOURCE_DIR}/linker/kernel32.ld)

target_link_libraries(kernel64 PRIVATE ${KERNEL64_LINK_LIBS})
target_link_libraries(kernel32 PRIVATE ${KERNEL32_LINK_LIBS})

set(LIBKERNEL_SOURCES
    src/libkernel/cpu.cpp
    src/libkernel/memory.cpp
    src/libkernel/io.cpp
    src/libkernel/system.cpp
    src/libkernel/console.cpp
)

set(LIBKBASE_SOURCES
    src/libkbase/algo.cpp
    src/libkbase/math.cpp
    src/libkbase/memory.cpp
    src/libkbase/string.cpp
)

set(LIBKRT_SOURCES
    src/libkrt/krt.cpp
)

set(LIBKSTD_SOURCES
    src/libkstd/assert.cpp
    src/libkstd/string.cpp
)

add_library(libkernel64 STATIC ${LIBKERNEL_SOURCES})
add_library(libkernel32 STATIC ${LIBKERNEL_SOURCES})
add_library(libkbase64 STATIC ${LIBKBASE_SOURCES})
add_library(libkbase32 STATIC ${LIBKBASE_SOURCES})
add_library(libkrt64 STATIC ${LIBKRT_SOURCES})
add_library(libkrt32 STATIC ${LIBKRT_SOURCES})
add_library(libkstd64 STATIC ${LIBKSTD_SOURCES})
add_library(libkstd32 STATIC ${LIBKSTD_SOURCES})

target_include_directories(libkernel64 PRIVATE "include/")
target_include_directories(libkernel32 PRIVATE "include/")
target_include_directories(libkbase64 PRIVATE "include/")
target_include_directories(libkbase32 PRIVATE "include/")
target_include_directories(libkrt64 PRIVATE "include/")
target_include_directories(libkrt32 PRIVATE "include/")
target_include_directories(libkstd64 PRIVATE "include/")
target_include_directories(libkstd32 PRIVATE "include/")

target_compile_options(libkernel64 PRIVATE ${KERNEL_COMPILE_FLAGS} -m64)
target_compile_options(libkernel32 PRIVATE ${KERNEL_COMPILE_FLAGS} -m32)
target_compile_options(libkbase64 PRIVATE ${KERNEL_COMPILE_FLAGS} -m64)
target_compile_options(libkbase32 PRIVATE ${KERNEL_COMPILE_FLAGS} -m32)
target_compile_options(libkrt64 PRIVATE ${KERNEL_COMPILE_FLAGS} -m64)
target_compile_options(libkrt32 PRIVATE ${KERNEL_COMPILE_FLAGS} -m32)
target_compile_options(libkstd64 PRIVATE ${KERNEL_COMPILE_FLAGS} -m64)
target_compile_options(libkstd32 PRIVATE ${KERNEL_COMPILE_FLAGS} -m32)

set_target_properties(libkernel64 PROPERTIES PREFIX "")
set_target_properties(libkernel32 PROPERTIES PREFIX "")
set_target_properties(libkbase64 PROPERTIES PREFIX "")
set_target_properties(libkbase32 PROPERTIES PREFIX "")
set_target_properties(libkrt64 PROPERTIES PREFIX "")
set_target_properties(libkrt32 PROPERTIES PREFIX "")
set_target_properties(libkstd64 PROPERTIES PREFIX "")
set_target_properties(libkstd32 PROPERTIES PREFIX "")


add_custom_target(iso
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/iso/boot/grub

    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:kernel32>
        ${CMAKE_BINARY_DIR}/iso/boot/kernel32

    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:kernel32>
        ${CMAKE_BINARY_DIR}/iso/boot/kernel32

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
        kernel32
        kernel64
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