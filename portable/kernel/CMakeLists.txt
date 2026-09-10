set(KERNEL_SOURCES
    cxx/memory.cpp
    cxx/system.cpp
    cxx/console.cpp
    cxx/shell.cpp
    cxx/envapi.cpp
    cxx/kernel.cpp
)

set(KERNEL_LINK_LIBS
    "$<LINK_GROUP:RESCAN,libstd,libbase,librt,libgrub,liblltools,libplatform,libarch>"
    gcc
)

add_executable(kernel ${KERNEL_SOURCES})

target_include_directories(kernel PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(kernel PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

target_link_options(kernel PRIVATE
    ${TARGET_LINK_FLAGS}
    ${TARGET_ARCH_FLAGS}
    -T ${TARGET_KERNEL_LINK_SCRIPT}
)

target_link_libraries(kernel PRIVATE ${KERNEL_LINK_LIBS})