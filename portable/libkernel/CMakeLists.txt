set(LIBKERNEL_SOURCES
    cxx/memory.cpp
    cxx/system.cpp
    cxx/console.cpp
)

add_library(libkernel STATIC ${LIBKERNEL_SOURCES})

target_include_directories(libkernel PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libkernel PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

set_target_properties(libkernel PROPERTIES PREFIX "")