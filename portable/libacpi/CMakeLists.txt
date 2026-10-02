set(LIBACPI_SOURCES
    cxx/acpi.cpp
)

add_library(libacpi STATIC ${LIBACPI_SOURCES})

target_include_directories(libacpi PRIVATE ${CMAKE_SOURCE_DIR}/include/)

target_compile_options(libacpi PRIVATE ${TARGET_COMPILE_FLAGS} ${TARGET_ARCH_FLAGS})

set_target_properties(libacpi PROPERTIES PREFIX "")