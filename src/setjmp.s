section .text

global setjmp
global longjmp

setjmp:
    mov [0 + rdi], rbx
    mov [8 + rdi], rbp
    mov [16 + rdi], r12
    mov [24 + rdi], r13
    mov [32 + rdi], r14
    mov [40 + rdi], r15
    lea rdx, [8 + rsp]
    mov [48 + rdi], rdx
    mov rdx, [rsp]
    mov [56 + rdi], rdx
    xor rax, rax
    ret

longjmp:
    mov rbx, rsi
    mov rbp, [8 + rdi]
    mov rbx, [0 + rdi]
    mov r12, [16 + rdi]
    mov r13, [24 + rdi]
    mov r14, [32 + rdi]
    mov r15, [40 + rdi]
    mov rsp, [48 + rdi]
    mov rdx, [56 + rdi]
    mov eax, 1
    test esi, esi
    cmovne eax, esi
    jmp rdx