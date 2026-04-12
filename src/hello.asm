section .data
    msg db "Hello from ASM!", 10
    msg_len equ $ - msg

section .text
    global hello_asm

hello_asm:
    mov rax, 1 ; SYS_write
    mov rdi, 1 ; STDOUT
    lea rsi, [rel msg]
    mov rdx, msg_len
    syscall
    ret