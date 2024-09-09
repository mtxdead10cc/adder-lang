#ifndef GVM_VALUE_H_
#define GVM_VALUE_H_

#include "gvm_types.h"
#include <stdint.h>
#include <stdbool.h>

#define VAL_MK_TYPE_ID(T) ( ((val_t)(T) & 0xF) << 60 )
#define VAL_GET_TYPE(V)     ((val_type_t)(((val_t)(V)) >> 60))

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
                            | (  (val_t)(((val_t)(value.num_args) & 0xFFFF) << 32) )\
                            | (  (val_t)( (val_t)(value.return_pc) & 0xFFFFFFFF   ) ) );
}

inline static val_t val_frame_from_args(int return_pc, uint16_t num_args) {
    return ( VAL_MK_TYPE_ID(VAL_FRAME)\
                            | (  (val_t)(((val_t)(num_args) & 0xFFFF) << 32) )\
                            | (  (val_t)( (val_t)(return_pc) & 0xFFFFFFFF   ) ) );
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
        .num_args = (uint16_t) (((val_t)(value) >> 32) & 0xFFFF),
        .return_pc = (int)((val_t)(value) & 0xFFFFFFFF)
    };
}

void val_print(val_t val);
void val_print_lookup(val_t val, addr_lookup_fn lookup, void* user);
int  val_get_string(val_t val, addr_lookup_fn lookup, void* user, char* dest, int dest_len);
void val_print_lookup_val_array(val_t* lookup_buffer, val_t val);
int  val_get_string_val_array(val_t* lookup_buffer, val_t val, char* dest, int dest_len);

#endif // GVM_VALUE_H_