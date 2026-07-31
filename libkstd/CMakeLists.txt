add_library(libkstd32 STATIC src/memory.cpp src/string.cpp)

target_include_directories(libkstd32 PUBLIC "include/")

target_compile_options(libkstd32 PRIVATE ${KERNEL_FLAGS} -m32)

set_target_properties(libkstd32 PROPERTIES PREFIX "")