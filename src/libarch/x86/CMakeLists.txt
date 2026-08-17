set(LIBARCH_SOURCES
    asm/boot.s
    cpu.cpp
    io.cpp
    api.cpp
)

add_library(libarch STATIC ${LIBARCH_SOURCES})

target_include_directories(libarch PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libarch PRIVATE ${KERNEL_COMPILE_FLAGS} -m32)

