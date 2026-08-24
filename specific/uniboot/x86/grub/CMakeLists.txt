set(UNIBOOT_SOURCES
    asm/boot.s
    cxx/uniboot.cpp
)

set(UNIBOOT_LINK_LIBS
    "$<LINK_GROUP:RESCAN,libkstd,libkbase,libkrt,libkernel,libgrub,libplatform,libarch>"
    gcc
)

add_executable(uniboot ${UNIBOOT_SOURCES})

target_include_directories(uniboot PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(uniboot PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

target_link_options(uniboot PRIVATE
    ${TARGET_LINK_FLAGS}
    ${TARGET_ARCH_FLAGS}
    -T ${TARGET_LINK_SCRIPT}
)

target_link_libraries(uniboot PRIVATE ${UNIBOOT_LINK_LIBS})