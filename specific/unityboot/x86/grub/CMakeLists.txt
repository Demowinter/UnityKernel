set(UNITYBOOT_SOURCES
    asm/boot.s
    cxx/console.cpp
    cxx/system.cpp
    cxx/unityboot.cpp
    ${CMAKE_SOURCE_DIR}/portable/kernel/cxx/console.cpp
    ${CMAKE_SOURCE_DIR}/portable/kernel/cxx/memory.cpp
    ${CMAKE_SOURCE_DIR}/portable/kernel/cxx/system.cpp
    ${CMAKE_SOURCE_DIR}/portable/kernel/cxx/envapi.cpp
    ${CMAKE_SOURCE_DIR}/portable/kernel/cxx/kernel.cpp
)

set(UNITYBOOT_LINK_LIBS
    "$<LINK_GROUP:RESCAN,libstd,libbase,librt,libgrub,libplatform,libarch>"
    gcc
)

add_executable(unityboot ${UNITYBOOT_SOURCES})

target_include_directories(unityboot PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(unityboot PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

target_link_options(unityboot PRIVATE
    ${TARGET_LINK_FLAGS}
    ${TARGET_ARCH_FLAGS}
    -T ${TARGET_UNITYBOOT_LINK_SCRIPT}
)

target_link_libraries(unityboot PRIVATE ${UNITYBOOT_LINK_LIBS})