#pragma once

#define DIV_ROUNDUP(a, b) ((a + (b - 1)) / b)
#define ALIGN_UP(x, a) (DIV_ROUNDUP(x, a) * a)
#define ALIGN_DOWN(x, a) ((x / a) * a)
#define SIZEOF_ARRAY(array) (sizeof(array) / sizeof(array[0]))
