add_library(libkernel32 STATIC src/console.cpp src/cpu.cpp)
add_library(libkernel64 STATIC src/console.cpp src/cpu.cpp)

target_include_directories(libkernel32 PUBLIC "include/")
target_include_directories(libkernel64 PUBLIC "include/")

target_compile_options(libkernel32 PRIVATE ${KERNEL_FLAGS} -m32)
target_compile_options(libkernel64 PRIVATE ${KERNEL_FLAGS} -m64)

set_target_properties(libkernel32 PROPERTIES PREFIX "")
set_target_properties(libkernel64 PROPERTIES PREFIX "")