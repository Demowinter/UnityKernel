set(LIBKERNEL_SOURCES
    cxx/memory.cpp
    cxx/system.cpp
    cxx/console.cpp
)

add_library(libkernel STATIC ${LIBKERNEL_SOURCES})

target_include_directories(libkernel PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libkernel PRIVATE ${KERNEL_COMPILE_FLAGS} ${KERNEL_COMPILE_FLAGS_EXTRA})

set_target_properties(libkernel PROPERTIES PREFIX "")