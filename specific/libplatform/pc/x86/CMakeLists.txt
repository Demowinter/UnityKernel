add_subdirectory(boot/${TARGET_BOOTLOADER}/)

set(LIBPLATFORM_SOURCES
    ${BOOTLOADER_SOURCES};
)

add_library(libplatform STATIC ${LIBPLATFORM_SOURCES})

target_include_directories(libplatform PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libplatform PRIVATE ${KERNEL_COMPILE_FLAGS} ${KERNEL_COMPILE_FLAGS_EXTRA})

set_target_properties(libplatform PROPERTIES PREFIX "")