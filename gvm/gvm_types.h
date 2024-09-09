#ifndef GVM_TYPES_H_
#define GVM_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "gvm_config.h"

typedef uint8_t type_id_t;

typedef struct grid_t {
    int width;
    int height;
    type_id_t* data;
} grid_t;

typedef struct valbuffer_t valbuffer_t;
typedef uint64_t val_t;
typedef uint32_t val_addr_t;

typedef struct ivec2_t {
    int16_t x;
    int16_t y;
} ivec2_t;

typedef struct array_t {
    val_addr_t address;
    int length;
} array_t;

typedef struct frame_t {
    int return_pc;      // the instruction to resume 
    uint16_t num_args;   // the number of args preceeding the frame
} frame_t;

typedef enum val_type_t {
    VAL_NUMBER,
    VAL_IVEC2,
    VAL_BOOL,
    VAL_CHAR,
    VAL_ARRAY,
    VAL_FRAME
} val_type_t;

typedef enum token_type_t {
    TT_UNKNOWN,
    TT_COLON,
    TT_NUMBER,
    TT_VEC2,
    TT_STRING,
    TT_SYMBOL,
    TT_SEPARATOR,
    TT_COMMENT,
    TT_END
} token_type_t;

typedef struct token_t {
    token_type_t type;
    int src_index;  // the start index in the source text (parser_text_t)
    int src_line;   // line in the source text (parser_text_t)
    int src_column; // column int he source text (parser_text_t)
    int index;      // index of the token (parser_tokens_t)
} token_t;

typedef struct parser_text_t {
    char* array;
    int size;
} parser_text_t;

typedef struct parser_tokens_t {
    token_t* array;
    int size;
    int capacity;
} parser_tokens_t;

typedef struct parser_t {
    parser_text_t text;
    parser_tokens_t tokens;
    int current;
} parser_t;

typedef enum gvm_result_t {
    RES_OK,
    RES_INVALID_INPUT,
    RES_NOT_SUPPORTED,
    RES_OUT_OF_MEMORY,
    RES_ERROR
} gvm_result_t;

typedef enum gvm_op_t {
    OP_HALT = 0x00,
    OP_AND,
    OP_OR,
    OP_NOR,
    OP_NOT,
    OP_MUL,
    OP_ADD,
    OP_SUB,
    OP_NEG,
    OP_DUP_1,
    OP_DUP_2,
    OP_ROT_2,
    OP_CMP_EQUAL,
    OP_CMP_LESS_THAN,
    OP_CMP_MORE_THAN,
    OP_PUSH_VALUE,
    OP_POP_1,
    OP_POP_2,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_EXIT_IMMEDIATE,
    OP_EXIT_WITH_VALUE,
    OP_CALL,
    OP_MAKE_FRAME,
    OP_RETURN,
    OP_STORE,
    OP_LOAD,
    OP_PRINT,
    OP_OPCODE_COUNT
} gvm_op_t;

typedef struct u8buffer_t {
    int size;
    int capacity;
    uint8_t* data;
} u8buffer_t;

typedef struct valbuffer_t {
    int storage;
    int size;
    int capacity;
    val_t* values;
} valbuffer_t;

typedef struct byte_code_block_t {
    int size;
    uint8_t* data;
} byte_code_block_t;

typedef struct byte_code_header_t {
    uint16_t header_size;
    uint16_t const_bytes;
    uint16_t code_bytes;
} byte_code_header_t;

typedef struct gvm_value_stack_t {
    val_t* values;  // pointer to the stack
    int top;
    int size;       // size of the stack (in val_t count)
} gvm_value_stack_t;

typedef struct gvm_heap_t {
    uint64_t*   gc_marks; // garbage collector (marking region)
    val_t*      values;   // pointer to heap memory region
    int         size;     // size of the heap memory (in val_t count)
} gvm_heap_t;

typedef struct gvm_mem_t {
    val_t*  membase;   // base pointer to the memory region (stack + heap)
    int     memsize;   // total size of stack + heap (in val_t count)
    gvm_value_stack_t stack;
    gvm_heap_t heap;
} gvm_mem_t;

typedef struct gvm_t gvm_t;

typedef val_t* (*addr_lookup_fn)(void* user, val_addr_t addr);
typedef void (*func_t)(gvm_t* vm);

typedef struct env_t {
    gvm_t* vm;
    struct {
        int count;
        func_t funcs[16];
        char* names[16];
    } native;
} env_t;

typedef struct gvm_runtime_t {
    val_t    registers[GVM_ASM_MAX_REGISTERS];
    val_t*   constants;
    uint8_t* instructions;
    int pc;
} gvm_runtime_t;

typedef struct gvm_t {
    env_t           env;
    gvm_mem_t       mem;
    gvm_runtime_t   run;
} gvm_t;

#endif // GVM_TYPES_H_