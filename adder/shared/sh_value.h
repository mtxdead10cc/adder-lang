#ifndef VM_VALUE_H_
#define VM_VALUE_H_

#include "sh_types.h"
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

// MEMORY

#define MEM_CONST_FLAG 1
#define MEM_PROGR_FLAG 2

#define MEM_MAX_ADDRESSABLE 0x3FFFFFFF
#define ADDR_NIL 0U

#define ADDR_IS_CONST(VAL_ADDR) (((VAL_ADDR) >> 30) == MEM_CONST_FLAG)
#define ADDR_IS_PROGR(VAL_ADDR) (((VAL_ADDR) >> 30) == MEM_PROGR_FLAG)
#define ADDR_IS_NULL(VAL_ADDR)  (((VAL_ADDR) >> 30) == 0)

#define MEM_MK_CONST_ADDR(INDEX)    ((val_addr_t)( 0x40000000 | ((INDEX) & 0x3FFFFFFF)))
#define MEM_MK_PROGR_ADDR(INDEX)    ((val_addr_t)( 0x80000000 | ((INDEX) & 0x3FFFFFFF)))

#define MEM_ADDR_TO_INDEX(VAL_ADDR) (uint32_t)((VAL_ADDR) & 0x3FFFFFFF)

// VALUE

inline static val_t val_none(void) {
    return (val_t) {
        .type = VAL_NONE,
        .u.boolean = false
    };
}

inline static val_t val_number(float value) {
    return (val_t) {
        .type = VAL_NUMBER,
        .u.number = value
    };
}

inline static val_t val_ivec2(ivec2_t value) {
    return (val_t) {
        .type = VAL_IVEC2,
        .u.ivec = value
    };
}

inline static val_t val_ivec2_from_args(int16_t x, int16_t y) {
    return (val_t) {
        .type = VAL_IVEC2,
        .u.ivec = (ivec2_t) {
            .x = x,
            .y = y
        }
    };
}

inline static val_t val_bool(bool value) {
    return (val_t) {
        .type = VAL_BOOL,
        .u.boolean = value
    };
}

inline static val_t val_char(char value) {
    return (val_t) {
        .type = VAL_CHAR,
        .u.character = value
    };
}

inline static val_t val_array(array_t value) {
    return (val_t) {
        .type = VAL_ARRAY,
        .u.array = value
    };
}

inline static val_t val_array_from_args(val_addr_t addr, int length) {
    return (val_t) {
        .type = VAL_ARRAY,
        .u.array = (array_t) {
            .address = addr,
            .length = length
        }
    };
}

inline static val_t val_frame(frame_t value) {
    return (val_t) {
        .type = VAL_FRAME,
        .u.frame = value
    };
}

inline static val_t val_frame_from_args(int return_pc, uint8_t num_args, uint8_t num_locals) {
    return (val_t) {
        .type = VAL_FRAME,
        .u.frame = (frame_t) {
            .num_args = num_args,
            .num_locals = num_locals,
            .return_pc = return_pc
        }
    };
}

inline static val_t val_iter(iter_t value) {
    return (val_t) {
        .type = VAL_ITER,
        .u.iter = value
    };
}

inline static float val_into_number(val_t value) {
    return value.u.number;
}

inline static ivec2_t val_into_ivec2(val_t value) {
    return value.u.ivec;
}

inline static bool val_into_bool(val_t value) {
    return value.u.boolean;
}

inline static char val_into_char(val_t value) {
    return value.u.character;
}

inline static array_t val_into_array(val_t value) {
    return value.u.array;
}

inline static frame_t val_into_frame(val_t value) {
    return value.u.frame;
}

inline static iter_t val_into_iter(val_t value) {
    return value.u.iter;
}

#endif // VM_VALUE_H_
