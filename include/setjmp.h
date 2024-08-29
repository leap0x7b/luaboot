#pragma once
#include <stdint.h>

typedef struct {
    uint64_t rbx;
    uint64_t rsp;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
    uint64_t mxcsr;
    uint8_t xmm_buf[160];
} __attribute__((aligned(8))) jmp_buf[1];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

#define _setjmp(env) setjmp(env)
#define _longjmp(env, val) longjmp(env, val)