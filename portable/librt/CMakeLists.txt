set(LIBRT_SOURCES
    cxx/runtime.cpp
)

add_library(librt STATIC ${LIBRT_SOURCES})

target_include_directories(librt PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(librt PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

set_target_properties(librt PROPERTIES PREFIX "")
