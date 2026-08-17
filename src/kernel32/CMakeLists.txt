set(KERNEL32_SOURCES
    cxx/grub.cpp
    cxx/kernel32.cpp
)

set(KERNEL32_LINK_LIBS
    "$<LINK_GROUP:RESCAN,libkstd32,libkbase32,libkrt32,libkernel32,libarch>"
    gcc
)

add_executable(kernel32 ${KERNEL32_SOURCES})

target_include_directories(kernel32 PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(kernel32 PRIVATE ${KERNEL_COMPILE_FLAGS} -m32)

target_link_options(kernel32 PRIVATE ${KERNEL_LINK_FLAGS} -m32 -T ${CMAKE_SOURCE_DIR}/linker/kernel32.ld)

target_link_libraries(kernel32 PRIVATE ${KERNEL32_LINK_LIBS})