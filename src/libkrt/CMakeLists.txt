set(LIBKRT_SOURCES
    krt.cpp
)

add_library(libkrt32 STATIC ${LIBKRT_SOURCES})

target_include_directories(libkrt32 PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libkrt32 PRIVATE ${KERNEL_COMPILE_FLAGS} -m32)

set_target_properties(libkrt32 PROPERTIES PREFIX "")
