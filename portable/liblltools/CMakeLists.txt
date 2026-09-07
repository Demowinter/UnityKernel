set(LIBLLTOOLS_SOURCES
    cxx/allocator.cpp
)

add_library(liblltools STATIC ${LIBLLTOOLS_SOURCES})

target_include_directories(liblltools PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(liblltools PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

set_target_properties(liblltools PROPERTIES PREFIX "")
