set(LIBBASE_SOURCES
    cxx/algo.cpp
    cxx/math.cpp
    cxx/memory.cpp
    cxx/string.cpp
)

add_library(libbase STATIC ${LIBBASE_SOURCES})

target_include_directories(libbase PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libbase PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

set_target_properties(libbase PROPERTIES PREFIX "")