set(LIBDRI_SOURCES
    cxx/ahci/ahci.cpp
    cxx/blockdev/ramdisk.cpp
    # cxx/fs/fat32.cpp
)

add_library(libdri STATIC ${LIBDRI_SOURCES})

target_include_directories(libdri PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libdri PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

set_target_properties(libdri PROPERTIES PREFIX "")