set(LIBKBASE_SOURCES
    cxx/algo.cpp
    cxx/math.cpp
    cxx/memory.cpp
    cxx/string.cpp
)

add_library(libkbase STATIC ${LIBKBASE_SOURCES})

target_include_directories(libkbase PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libkbase PRIVATE ${KERNEL_COMPILE_FLAGS} ${KERNEL_COMPILE_FLAGS_EXTRA})

set_target_properties(libkbase PROPERTIES PREFIX "")