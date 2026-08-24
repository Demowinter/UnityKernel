set(LIBGRUB_SOURCES
    cxx/multiboot.cpp
)

add_library(libgrub STATIC ${LIBGRUB_SOURCES})

target_include_directories(libgrub PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libgrub PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

set_target_properties(libgrub PROPERTIES PREFIX "")