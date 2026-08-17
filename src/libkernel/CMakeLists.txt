set(LIBKERNEL_SOURCES
    memory.cpp
    system.cpp
    console.cpp
)

add_library(libkernel32 STATIC ${LIBKERNEL_SOURCES})

target_include_directories(libkernel32 PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libkernel32 PRIVATE ${KERNEL_COMPILE_FLAGS} -m32)

set_target_properties(libkernel32 PROPERTIES PREFIX "")