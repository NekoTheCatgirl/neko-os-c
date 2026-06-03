enable_language(ASM_NASM)

set(CMAKE_ASM_NASM_FLAGS "-f elf64")

add_library(boot_objs OBJECT
            boot.nasm
            main.nasm
            main64.nasm
)