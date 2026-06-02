add_library(lib_objs OBJECT
            string.c
            printf.c
            sprintf.c
            klogf.c
            hpet.c
            alloc.c
            madt.c
)

target_include_directories(lib_objs PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_options(lib_objs PRIVATE ${KERNEL_C_FLAGS})