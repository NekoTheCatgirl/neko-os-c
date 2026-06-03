%macro isr_no_err 1
global isr%1
isr%1:
    push 0          ; fake error code to normalize the stack
    push %1         ; vector number
    jmp isr_common
%endmacro

%macro isr_err 1
global isr%1
isr%1:
    push %1         ; error code already on stack, just push vector
    jmp isr_common
%endmacro

isr_no_err 0   ; Division By Zero
isr_no_err 1   ; Debug
isr_no_err 2   ; NMI
isr_no_err 3   ; Breakpoint
isr_no_err 4   ; Overflow
isr_no_err 5   ; Bound Range Exceeded
isr_no_err 6   ; Invalid Opcode
isr_no_err 7   ; Device Not Available
isr_err    8   ; Double Fault
isr_no_err 9   ; Coprocessor Segment Overrun
isr_err    10  ; Invalid TSS
isr_err    11  ; Segment Not Present
isr_err    12  ; Stack Fault
isr_err    13  ; General Protection Fault
isr_err    14  ; Page Fault
isr_no_err 15  ; Reserved
isr_no_err 16  ; x87 Floating Point
isr_err    17  ; Alignment Check
isr_no_err 18  ; Machine Check
isr_no_err 19  ; SIMD Floating Point
isr_no_err 20  ; Virtualization
isr_err    21  ; Control Protection
isr_no_err 22
isr_no_err 23
isr_no_err 24
isr_no_err 25
isr_no_err 26
isr_no_err 27
isr_no_err 28
isr_no_err 29
isr_err    30  ; Security
isr_no_err 31

extern exception_handler

isr_common:
    ; save all general purpose registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; at this point the stack looks like:
    ; r15, r14 ... rax, vector, error_code, rip, cs, rflags, rsp, ss

    mov rdi, [rsp + 15*8]   ; vector number
    mov rsi, [rsp + 16*8]   ; error code
    call exception_handler

    ; restore registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16     ; pop vector and error code
    iretq

extern timer_handler
global isr_timer

isr_timer:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    call timer_handler

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    iretq          ; no add rsp, 16 here

extern spurious_handler
global isr_spurious

isr_spurious:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    call spurious_handler   ; also fix: you were calling timer_handler here!

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    iretq          ; no add rsp, 16