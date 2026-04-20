DEFAULT ABS
BITS 64

FLG_PRESENT   equ 0b10000000
FLG_RING0     equ 0b00000000
FLG_RING3     equ 0b01100000
FLG_TYPE_INT  equ 0b00001110
FLG_TYPE_TRAP equ 0b00001111

; id, selector, flags
%macro idt_entry 3
.entry_%1:
    dw 0
    dw (%2 & 0xffff)
    db 0
    db (%3 & 0xf)
    dw 0
    dd 0
    dd 0
%endmacro

; id
%macro write_idt_addr 1
    lea rax, [idt_int_handler_%1]
    mov word [idt.entry_%1], ax
    shr rax, 16
    mov word [idt.entry_%1 + 6], ax
    shr rax, 16
    mov dword [idt.entry_%1 + 8], eax
%endmacro

%macro idt_int 1
    idt_entry %1, 0b1000, FLG_PRESENT | FLG_RING0 | FLG_TYPE_INT
%endmacro

%macro idt_trap 1
    idt_entry %1, 0b1000, FLG_PRESENT | FLG_RING0 | FLG_TYPE_TRAP
%endmacro

%macro int_handler_snippet 1
    push %1
    jmp idt_common_handler
%endmacro

%macro int_handler_error 1
idt_int_handler_%1:
    int_handler_snippet %1
%endmacro

%macro int_handler_no_error 1
idt_int_handler_%1:
    push -1
    int_handler_snippet %1
%endmacro

section .data

idt:
    ; Exceptions
    idt_trap 0
    idt_trap 1
    idt_int  2 ; NMI is an interrupt! The rest can be interrupted.
%assign i 3
%rep (32 - 3)
    idt_trap i
%assign i (i + 1)
%endrep

    ; Actual interrupts
%assign i 32
%rep (256 - 32)
    idt_int i
%assign i (i + 1)
%endrep
.end:

idt_descriptor:
.size:
    dw (256 * 16) - 1
.offset:
    dq idt

section .text

extern idt_int_handler

global idt_load
idt_load:
%assign i 0
%rep 256
    write_idt_addr i
%assign i (i + 1)
%endrep
    lidt [idt_descriptor]
    ret

idt_common_handler:
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    mov  rdi, rsp
    call idt_int_handler
    mov  rsp, rax

    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    add rsp, 16 ; poop the interrupt number and the error code
    iret

    int_handler_no_error  0
    int_handler_error     1
    int_handler_no_error  2
    int_handler_no_error  3
    int_handler_no_error  4
    int_handler_no_error  5
    int_handler_no_error  6
    int_handler_no_error  7
    int_handler_error     8
    int_handler_no_error  9
    int_handler_error    10
    int_handler_error    11
    int_handler_error    12
    int_handler_error    13
    int_handler_error    14
    int_handler_no_error 15
    int_handler_no_error 16
    int_handler_error    17
    int_handler_no_error 18
    int_handler_no_error 19
    int_handler_no_error 20
    int_handler_error    21
%assign i 22
%rep (256 - 22)
    int_handler_no_error i
%assign i (i + 1)
%endrep
