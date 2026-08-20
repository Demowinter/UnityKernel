set(LIBPLATFORM_SOURCES
    cxx/api.cpp
)

add_library(libplatform STATIC ${LIBPLATFORM_SOURCES})

target_include_directories(libplatform PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libplatform PRIVATE ${KERNEL_COMPILE_FLAGS} ${KERNEL_ARCH_FLAGS})

set_target_properties(libplatform PROPERTIES PREFIX "")