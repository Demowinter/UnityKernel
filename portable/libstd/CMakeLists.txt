set(LIBSTD_SOURCES
    cxx/assert.cpp
    cxx/string.cpp
)

add_library(libstd STATIC ${LIBSTD_SOURCES})

target_include_directories(libstd PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libstd PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

set_target_properties(libstd PROPERTIES PREFIX "")