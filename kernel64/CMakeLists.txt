add_executable(kernel64 boot/boot.s src/kernel64.cpp)

target_compile_options(kernel64 PRIVATE ${KERNEL_FLAGS} -m64)

target_link_options(kernel64 PRIVATE
    -no-pie
    -T${CMAKE_SOURCE_DIR}/linker/kernel64.ld
    -nostdlib
)

target_link_libraries(kernel64 PRIVATE libkernel64)