set(LIBKSTD_SOURCES
    cxx/assert.cpp
    cxx/string.cpp
)

add_library(libkstd32 STATIC ${LIBKSTD_SOURCES})

target_include_directories(libkstd32 PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libkstd32 PRIVATE ${KERNEL_COMPILE_FLAGS} -m32)

set_target_properties(libkstd32 PROPERTIES PREFIX "")

set_target_properties(libarch PROPERTIES PREFIX "")