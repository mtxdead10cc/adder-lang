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

#define VAL_MK_TYPE_ID(T) ( ((val_t)(T) & 0xF) << 60 )
#define VAL_GET_TYPE(V)     ((val_type_t)(((val_t)(V)) >> 60))

inline static val_t val_none() {
    return VAL_MK_TYPE_ID(VAL_NONE);
}

inline static val_t val_number(float value) {
    uint32_t tmp = *((uint32_t*)&value);
    return (VAL_MK_TYPE_ID(VAL_NUMBER) | tmp);
}

inline static val_t val_ivec2(ivec2_t value) {
    return (VAL_MK_TYPE_ID(VAL_IVEC2) | (value.x << 16) | value.y);
}

inline static val_t val_ivec2_from_args(int16_t x, int16_t y) {
    return (VAL_MK_TYPE_ID(VAL_IVEC2) | (x << 16) | y);
}

inline static val_t val_bool(bool value) {
    return (VAL_MK_TYPE_ID(VAL_BOOL) | (value ? 0xFF : 0x00));
}

inline static val_t val_char(char value) {
    return VAL_MK_TYPE_ID(VAL_CHAR) | (((val_t)(value)) & 0xFFFF);
}

inline static val_t val_array(array_t value) {
    return ( VAL_MK_TYPE_ID(VAL_ARRAY)\
                            | (  (val_t)(((val_t)(value.length) & 0xFFFFFF) << 32) )\
                            | (  (val_t)( (val_t)(value.address) & 0xFFFFFFFF    ) ) );
}

inline static val_t val_array_from_args(val_addr_t addr, int length) {
    return ( VAL_MK_TYPE_ID(VAL_ARRAY)\
                            | (  (val_t)(((val_t)(length) & 0xFFFFFF) << 32) )\
                            | (  (val_t)( (val_t)(addr) & 0xFFFFFFFF       ) ) );
}

inline static val_t val_frame(frame_t value) {
    return ( VAL_MK_TYPE_ID(VAL_FRAME)\
                            | (  (val_t)(((val_t)(value.num_args) & 0xFF) << 40) )\
                            | (  (val_t)(((val_t)(value.num_locals) & 0xFF) << 32) )\
                            | (  (val_t)( (val_t)(value.return_pc) & 0xFFFFFFFF   ) ) );
}

inline static val_t val_frame_from_args(int return_pc, uint8_t num_args, uint8_t num_locals) {
    return ( VAL_MK_TYPE_ID(VAL_FRAME)\
                            | (  (val_t)(((val_t)(num_args) & 0xFF) << 40) )\
                            | (  (val_t)(((val_t)(num_locals) & 0xFF) << 32) )\
                            | (  (val_t)( (val_t)(return_pc) & 0xFFFFFFFF   ) ) );
}

inline static val_t val_iter(iter_t value) {
    return ( VAL_MK_TYPE_ID(VAL_ITER)\
                        | (  (val_t)(((val_t)(value.remaining) & 0xFFFFFF) << 32) )\
                        | (  (val_t)( (val_t)(value.current) & 0xFFFFFFFF    ) ) );
}

inline static float val_into_number(val_t value) {
    uint32_t tmp = (uint32_t)(value & 0xFFFFFFFF);
    return *(float*)&tmp;
}

inline static ivec2_t val_into_ivec2(val_t value) {
    return (ivec2_t) {
        .x = ((value >> 16) & 0xFFFF),
        .y = ( value & 0xFFFF )
    };
}

inline static bool val_into_bool(val_t value) {
    return ((((val_t)(value)) & 0xFF) > 0x80);
}

inline static char val_into_char(val_t value) {
    return ((char)(((val_t)(value)) & 0xFFFF));
}

inline static array_t val_into_array(val_t value) {
    return (array_t) {
        .length = (int) (((val_t)(value) >> 32) & 0xFFFFFF),
        .address = (val_addr_t)((val_t)(value) & 0xFFFFFFFF)
    };
}

inline static frame_t val_into_frame(val_t value) {
    return (frame_t) {
        .num_args = (uint8_t) (((val_t)(value) >> 40) & 0xFF),
        .num_locals = (uint8_t) (((val_t)(value) >> 32) & 0xFF),
        .return_pc = (int)((val_t)(value) & 0xFFFFFFFF)
    };
}

inline static iter_t val_into_iter(val_t value) {
    return (iter_t) {
        .remaining = (int) (((val_t)(value) >> 32) & 0xFFFFFF),
        .current = (val_addr_t)((val_t)(value) & 0xFFFFFFFF)
    };
}

#endif // VM_VALUE_H_
