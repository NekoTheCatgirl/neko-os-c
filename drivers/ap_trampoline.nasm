bits 16

global trampoline_start
global trampoline_end
global trampoline_pm
global trampoline_pm_jmp
global trampoline_gdt
global trampoline_gdt_ptr
global trampoline_gdt64
global trampoline_gdt64_ptr
global trampoline_pml4
global trampoline_stack
global trampoline_entry

trampoline_start:
    cli
    cld

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    lgdt [trampoline_gdt_ptr - trampoline_start + 0x8000]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

trampoline_pm_jmp:
    jmp 0x08:0x0000             ; BSP patches offset to trampoline_pm physical addr

bits 32
trampoline_pm:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov eax, [trampoline_pml4 - trampoline_start + 0x8000]
    mov cr3, eax

    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    lgdt [trampoline_gdt64_ptr - trampoline_start + 0x8000]
    jmp 0x08:(trampoline_lm - trampoline_start + 0x8000)

bits 64
trampoline_lm:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rsp, [trampoline_stack - trampoline_start + 0x8000]
    mov rax, [trampoline_entry - trampoline_start + 0x8000]
    call rax

.hang:
    cli
    hlt
    jmp .hang

align 8
trampoline_gdt:
    dq 0
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
trampoline_gdt_ptr:
    dw trampoline_gdt_ptr - trampoline_gdt - 1
    dd trampoline_gdt - trampoline_start + 0x8000   ; hardcoded physical addr

align 8
trampoline_gdt64:
    dq 0
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
trampoline_gdt64_ptr:
    dw trampoline_gdt64_ptr - trampoline_gdt64 - 1
    dq trampoline_gdt64 - trampoline_start + 0x8000  ; hardcoded physical addr

align 8
trampoline_pml4:  dd 0
trampoline_stack: dq 0
trampoline_entry: dq 0

trampoline_end: