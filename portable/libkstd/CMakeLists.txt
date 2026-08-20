set(LIBKSTD_SOURCES
    cxx/assert.cpp
    cxx/string.cpp
)

add_library(libkstd STATIC ${LIBKSTD_SOURCES})

target_include_directories(libkstd PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libkstd PRIVATE ${KERNEL_COMPILE_FLAGS} ${KERNEL_COMPILE_FLAGS_EXTRA})

set_target_properties(libkstd PROPERTIES PREFIX "")