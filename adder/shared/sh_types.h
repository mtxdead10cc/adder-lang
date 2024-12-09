#ifndef GVM_SHARED_TYPES_H_
#define GVM_SHARED_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "sh_config.h"

typedef struct arena_t arena_t;
typedef struct arena_t {
    arena_t* next;
    ptrdiff_t size;
    ptrdiff_t capacity;
    uint8_t* data;
} arena_t;

typedef uint8_t type_id_t;

typedef uint64_t val_t;
typedef uint32_t val_addr_t;

typedef struct ivec2_t {
    int16_t x;
    int16_t y;
} ivec2_t;

typedef struct array_t {
    val_addr_t address; // the address of the first value
    int length;         // the length of the array
} array_t;

typedef struct frame_t {
    int return_pc;      // the instruction to resume 
    uint8_t num_args;   // the number of args 
    uint8_t num_locals; // the number of locals 
} frame_t;

typedef struct iter_t {
    val_addr_t current; // the address of the current value
    int remaining;      // the number of iterations remaining 
} iter_t;

typedef enum val_type_t {
    VAL_NONE,
    VAL_NUMBER,
    VAL_IVEC2,
    VAL_BOOL,
    VAL_CHAR,
    VAL_ARRAY,
    VAL_FRAME,
    VAL_ITER,
    VAL_TYPE_COUNT
} val_type_t;

typedef enum gvm_op_t {
    OP_HALT = 0x00,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_ADD,
    OP_SUB,
    OP_NEG,
    OP_DUP_1,
    OP_DUP_2,
    OP_ROT_2,
    OP_CMP_EQUAL,
    OP_CMP_NOT_EQUAL,
    OP_CMP_LESS_THAN,
    OP_CMP_MORE_THAN,
    OP_CMP_LESS_THAN_OR_EQUAL,
    OP_CMP_MORE_THAN_OR_EQUAL,
    OP_PUSH_VALUE,
    OP_POP_1,
    OP_POP_2,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_EXIT,
    OP_CALL,
    OP_ENTRY_POINT,
    OP_MAKE_FRAME,
    OP_RETURN_NOTHING,
    OP_RETURN_VALUE,
    OP_STORE_LOCAL,
    OP_LOAD_LOCAL,
    OP_PRINT,
    OP_MAKE_ARRAY,
    OP_ARRAY_LENGTH,
    OP_MAKE_ITER,
    OP_ITER_NEXT,
    OP_CALL_NATIVE,
    OP_OPCODE_COUNT
} gvm_op_t;

typedef struct sstr_t {
    char str[GVM_DEFAULT_STRLEN];
} sstr_t;

typedef struct ffi_bundle_t ffi_bundle_t;
typedef struct gvm_program_t {
    struct {
        uint32_t    size;   // size in bytes
        uint8_t*    buffer; // instructions
    } inst;
    struct {
        uint32_t    count;  // number of constants (values)
        val_t*      buffer; // values
    } cons;
    ffi_bundle_t*   ffi;
} gvm_program_t;

#endif // GVM_SHARED_TYPES_H_