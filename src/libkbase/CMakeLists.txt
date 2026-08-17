set(LIBKBASE_SOURCES
    cxx/algo.cpp
    cxx/math.cpp
    cxx/memory.cpp
    cxx/string.cpp
)

add_library(libkbase32 STATIC ${LIBKBASE_SOURCES})

target_include_directories(libkbase32 PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libkbase32 PRIVATE ${KERNEL_COMPILE_FLAGS} -m32)

set_target_properties(libkbase32 PROPERTIES PREFIX "")
