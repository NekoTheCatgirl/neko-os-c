bits 32
section .multiboot
align 8
header_start:
    ; Magic
    dd 0xE85250D6
    ; Architecture: i386 protected mode
    dd 0
    ; Header length
    dd header_end - header_start
    ; Checksum
    dd 0x100000000 - (0xE85250D6 + 0 + (header_end - header_start))

    ; Framebuffer Request (Type 5)
    align 8                       ; CRITICAL: Force 8-byte alignment
    .fb_start:
        dw 5                      ; Type
        dw 1                      ; Flags (1=Optional)
        dd .fb_end - .fb_start    ; Size (Auto-calculated)
        dd 480                    ; Width (0=Any)
        dd 270                    ; Height (0=Any)
        dd 0                      ; Depth (0=Any)
    .fb_end:

    ; End Tag
    align 8                       ; CRITICAL: Force 8-byte alignment
    .end_start:
        dw 0                      ; Type
        dw 0                      ; Flags
        dd .end_end - .end_start  ; Size (Must be 8)
    .end_end:
header_end: