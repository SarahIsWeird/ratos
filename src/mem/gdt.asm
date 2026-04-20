DEFAULT ABS
BITS 64

section .text

global gdt_reload_segments
gdt_reload_segments:
    push 0x8
    lea  rax, [rel .reload_cs]
    push rax
    retfq
.reload_cs:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret
