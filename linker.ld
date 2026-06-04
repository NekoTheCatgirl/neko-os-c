ENTRY(start)

SECTIONS
{
    . = 1M;

    trampoline_load_addr = 0x8000;

    ld_kernel_start = .;

    .text ALIGN(4K) :
    {
        KEEP(*(.multiboot))
        *(.text)
    }

    .heap BLOCK(4K) : ALIGN(4K)
    {
        ld_heap_start = .;
        . += 256M;
        ld_heap_end = .;
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }

    .data ALIGN(4K) :
    {
        *(.data)
    }

    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    }

    ld_kernel_end = .;
}