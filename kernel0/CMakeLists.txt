add_executable(kernel0 boot/boot.s src/kernel0.cpp)

target_compile_options(kernel0 PRIVATE ${KERNEL_FLAGS} -m32)

target_link_options(kernel0 PRIVATE
    -no-pie
    -T${CMAKE_SOURCE_DIR}/linker/kernel0.ld
    -nostdlib
)

target_link_libraries(kernel0 PRIVATE libkernel32 libkstd32)
