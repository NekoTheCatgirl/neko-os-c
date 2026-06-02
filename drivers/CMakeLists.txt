enable_language(ASM_NASM)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

add_library(drivers_asm_objs OBJECT
            idt.asm
)

add_library(drivers_objs OBJECT
            framebuffer.c
            acpi.c
            idt.c
            paging.c
            gdt.c
            serial.c
            font.c
            pic.c
            lapic.c
            ioapic.c
)

target_include_directories(drivers_objs PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_options(drivers_objs PRIVATE ${KERNEL_C_FLAGS})