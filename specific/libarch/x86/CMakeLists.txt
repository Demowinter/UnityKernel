set(LIBARCH_SOURCES
    cxx/cpu.cpp
    cxx/pmio.cpp
    cxx/mmio.cpp
    cxx/interrupt.cpp
    cxx/api.cpp
)

set(TARGET_ARCH_FLAGS
    -mno-red-zone
    -m32
)

set(TARGET_ARCH_FLAGS ${TARGET_ARCH_FLAGS} PARENT_SCOPE)

add_library(libarch STATIC ${LIBARCH_SOURCES})

target_include_directories(libarch PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libarch PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

# Disable 80387 instructions for interrupt.cpp
set_source_files_properties(cxx/interrupt.cpp PROPERTIES COMPILE_FLAGS "-mno-80387")

set_target_properties(libarch PROPERTIES PREFIX "")