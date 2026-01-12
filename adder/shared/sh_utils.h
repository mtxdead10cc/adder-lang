#ifndef VM_UTILS_H_
#define VM_UTILS_H_

#include "shared/sh_types.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#define READ_U32(D, AT) ((uint32_t)((0xFF & (D)[(AT) + 3]) << (8*3)) |\
                         (uint32_t)((0xFF & (D)[(AT) + 2]) << (8*2)) |\
                         (uint32_t)((0xFF & (D)[(AT) + 1]) << (8*1)) |\
                         (uint32_t)((0xFF & (D)[(AT) + 0]) << (8*0)))

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define clamp(v, a, b) (max(min((v), b), a))

#endif // VM_UTILS_H_
