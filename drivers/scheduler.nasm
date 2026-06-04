global scheduler_context_switch
; scheduler_context_switch(task_t *old, task_t *new)
; rdi = old task, rsi = new task
scheduler_context_switch:
    ; Save callee-saved registers onto current stack
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    ; Save rsp into old task (rsp field is at offset 0)
    mov [rdi], rsp

    ; Load rsp from new task
    mov rsp, [rsi]

    ; Restore callee-saved registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp

    ret ; Return into new task