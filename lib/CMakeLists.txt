add_library(lib_objs OBJECT
            string.c
            sprintf.c
            klogf.c
            alloc.c
            console.c
)

target_include_directories(lib_objs PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_options(lib_objs PRIVATE ${KERNEL_C_FLAGS})