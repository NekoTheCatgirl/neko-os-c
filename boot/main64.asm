global long_mode_start
extern kernel_main
extern stack_top

section .text
bits 64
long_mode_start:
    ; Write directly to VGA memory as a canary
    mov rax, 0x4F4B4F4F        ; 'OK' in white on red
    mov rbx, 0xB8000
    mov [rbx], rax

    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rsp, stack_top

    call kernel_main

    cli
.hang: hlt
    jmp .hang
.end: