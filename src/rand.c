#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define STATE_SIZE  312
#define MIDDLE      156
#define INIT_SHIFT  62
#define TWIST_MASK  0xb5026f5aa96619e9
#define INIT_FACT   6364136223846793005
#define SHIFT1      29
#define MASK1       0x5555555555555555
#define SHIFT2      17
#define MASK2       0x71d67fffeda60000
#define SHIFT3      37
#define MASK3       0xfff7eee000000000
#define SHIFT4      43
#define LOWER_MASK  0x7fffffff
#define UPPER_MASK  (~(uint64_t)LOWER_MASK)

static uint64_t state[STATE_SIZE];
static size_t index = STATE_SIZE + 1;

void srand(uint64_t seed) {
    index = STATE_SIZE;
    state[0] = seed;
    for (size_t i = 1; i < STATE_SIZE; i++)
        state[i] = (INIT_FACT * (state[i - 1] ^ (state[i - 1] >> INIT_SHIFT))) + i;
}

uint64_t rand(void) {
    if (index >= STATE_SIZE) {
        for (size_t i = 0; i < STATE_SIZE; i++) {
            uint64_t x = (state[i] & UPPER_MASK) | (state[(i + 1) % STATE_SIZE] & LOWER_MASK);
            x = (x >> 1) ^ (x & 1? TWIST_MASK : 0);
            state[i] = state[(i + MIDDLE) % STATE_SIZE] ^ x;
        }
        index = 0;
    }

    uint64_t y = state[index];
    y ^= (y >> SHIFT1) & MASK1;
    y ^= (y << SHIFT2) & MASK2;
    y ^= (y << SHIFT3) & MASK3;
    y ^= y >> SHIFT4;

    index++;
    return y;
}