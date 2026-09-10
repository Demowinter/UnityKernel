set(LIBELF_SOURCES
    cxx/loader.cpp
)

add_library(libelf STATIC ${LIBELF_SOURCES})

target_include_directories(libelf PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libelf PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

set_target_properties(libelf PROPERTIES PREFIX "")