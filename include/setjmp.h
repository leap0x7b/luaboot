#pragma once
#include <stdint.h>

typedef uint64_t jmp_buf[8];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

#define _setjmp(env) setjmp(env)
#define _longjmp(env, val) longjmp(env, val)