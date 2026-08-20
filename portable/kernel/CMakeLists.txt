set(KERNEL_SOURCES
    cxx/grub.cpp
    cxx/kernel.cpp
)

set(KERNEL_LINK_LIBS
    "$<LINK_GROUP:RESCAN,libkstd,libkbase,libkrt,libkernel,libplatform,libarch>"
    gcc
)

add_executable(kernel ${KERNEL_SOURCES})

target_include_directories(kernel PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(kernel PRIVATE ${KERNEL_COMPILE_FLAGS} ${KERNEL_ARCH_FLAGS})

target_link_options(kernel PRIVATE
    ${KERNEL_LINK_FLAGS}
    ${KERNEL_ARCH_FLAGS}
    -T ${CMAKE_SOURCE_DIR}/specific/libplatform/${TARGET_PLATFORM}/${TARGET_ARCH}/linker/link.ld
)

target_link_libraries(kernel PRIVATE ${KERNEL_LINK_LIBS})