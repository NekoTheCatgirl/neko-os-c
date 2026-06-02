add_library(kernel_objs OBJECT
            main.c
            multiboot2.c
            mem.c
            panic.c
)

target_include_directories(kernel_objs PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_options(kernel_objs PRIVATE ${KERNEL_C_FLAGS})