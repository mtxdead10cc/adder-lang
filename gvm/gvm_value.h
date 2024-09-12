#ifndef GVM_VALUE_H_
#define GVM_VALUE_H_

#include "gvm_types.h"
#include "gvm_memory.h"
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

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

inline static val_t* array_get_ptr(gvm_t* vm, array_t array, int index) {
    assert(ADDR_IS_NULL(array.address) == false);
    if( ADDR_IS_CONST(array.address) ) {
        return (vm->run.constants + MEM_ADDR_TO_INDEX(array.address) + index);
    } else {
        return (vm->mem.membase + MEM_ADDR_TO_INDEX(array.address) + index);
    }
}

inline static val_t array_get(gvm_t* vm, array_t array, int index) {
    return *array_get_ptr(vm, array, index);
}

inline static void array_set(gvm_t* vm, array_t array, int index, val_t value) {
    val_t* loc = array_get_ptr(vm, array, index);
    *loc = value;
}

void val_print(val_t val);
void val_print_lookup(val_t val, addr_lookup_fn lookup, void* user);
int  val_get_string(val_t val, addr_lookup_fn lookup, void* user, char* dest, int dest_len);
void val_print_lookup_val_array(val_t* lookup_buffer, val_t val);
int  val_get_string_val_array(val_t* lookup_buffer, val_t val, char* dest, int dest_len);
char* val_get_type_name(val_type_t type);

#endif // GVM_VALUE_H_