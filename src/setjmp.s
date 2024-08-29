section .text

global setjmp
global longjmp

setjmp:
    pop rsi
    mov [0 + rdi], rbx
    mov [8 + rdi], rsp
    push rsi
    mov [16 + rdi], rbp
    mov [24 + rdi], r12
    mov [32 + rdi], r13
    mov [40 + rdi], r14
    mov [48 + rdi], r15
    mov [56 + rdi], rsi
    xor rax, rax
    ret

longjmp:
    mov eax, esi
    mov rbx, [0 + rdi]
    mov rsp, [8 + rdi]
    mov rbp, [16 + rdi]
    mov r12, [24 + rdi]
    mov r13, [32 + rdi]
    mov r14, [40 + rdi]
    mov r15, [48 + rdi]
    xor rdx, rdx
    mov rcx, 1
    cmp rdx, rax
    cmovz rax, rcx
    jmp [56 + rdx]