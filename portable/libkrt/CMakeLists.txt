set(LIBKRT_SOURCES
    cxx/krt.cpp
)

add_library(libkrt STATIC ${LIBKRT_SOURCES})

target_include_directories(libkrt PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libkrt PRIVATE ${KERNEL_COMPILE_FLAGS} ${KERNEL_COMPILE_FLAGS_EXTRA})

set_target_properties(libkrt PROPERTIES PREFIX "")
