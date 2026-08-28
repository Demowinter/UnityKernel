set(LIBPLATFORM_SOURCES
    cxx/api.cpp
)

set(TARGET_KERNEL_LINK_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/kernel/link.ld" PARENT_SCOPE)
set(TARGET_UNITYBOOT_LINK_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/unityboot/link.ld" PARENT_SCOPE)

add_library(libplatform STATIC ${LIBPLATFORM_SOURCES})

target_include_directories(libplatform PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libplatform PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

set_target_properties(libplatform PROPERTIES PREFIX "")