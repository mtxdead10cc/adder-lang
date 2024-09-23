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

typedef enum gvm_op_t {
    OP_HALT = 0x00,
    OP_AND,
    OP_OR,
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
    OP_EXIT,
    OP_CALL,
    OP_ENTRY_POINT,
    OP_MAKE_FRAME,
    OP_RETURN,
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

typedef struct u8buffer_t {
    uint32_t size;
    uint32_t capacity;
    uint8_t* data;
} u8buffer_t;

typedef struct valbuffer_t {
    uint32_t size;
    uint32_t capacity;
    val_t* values;
} valbuffer_t;

typedef struct gvm_program_t {
    struct {
        uint32_t         size;   // size in bytes
        uint8_t*    buffer; // instructions
    } inst;
    struct {
        uint32_t         count;  // number of constants (values)
        val_t*      buffer; // values
    } cons;
} gvm_program_t;

typedef struct gvm_exec_args_t {
    struct {
        uint32_t    count; 
        val_t*      buffer;
    } args;
    uint32_t  cycle_limit;
} gvm_exec_args_t;

typedef struct gvm_stack_t {
    val_t* values;  // pointer to the stack
    int top;        // the index of the top element on the stack
    int frame;      // the index of the current stack (call) frame
    int size;       // size of the stack (in val_t count)
} gvm_stack_t;

typedef struct gvm_heap_t {
    uint64_t*   gc_marks; // garbage collector (marking region)
    val_t*      values;   // pointer to heap memory region
    int         size;     // size of the heap memory (in val_t count)
} gvm_heap_t;

typedef struct gvm_mem_t {
    val_t*      membase;   // base pointer to the memory region (stack + heap)
    int         memsize;   // total size of stack + heap (in val_t count)
    gvm_stack_t stack;
    gvm_heap_t  heap;
} gvm_mem_t;

typedef struct gvm_t gvm_t;

typedef struct func_result_t {
    int arg_count;
    val_t value;
} func_result_t;

typedef val_t* (*addr_lookup_fn)(void* user, val_addr_t addr);
typedef val_t (*func_t)(gvm_t* gvm, val_t* args);

typedef struct tabval_t {
    int     argc;
    func_t  func;
} tabval_t;

typedef struct func_table_t {
    char      key[GVM_ENV_NFUNC_NAME_MAX_LEN][GVM_ENV_NFUNC_TABLE_SIZE];
    bool      is_in_use[GVM_ENV_NFUNC_TABLE_SIZE];
    tabval_t  value[GVM_ENV_NFUNC_TABLE_SIZE];
} func_table_t;

typedef struct env_t {
    gvm_t*       vm;
    func_table_t table;
} env_t;

typedef struct gvm_runtime_t {
    val_t*   constants;
    uint8_t* instructions;
    uint32_t pc;
} gvm_runtime_t;

typedef struct gvm_t {
    env_t           env;
    gvm_mem_t       mem;
    gvm_runtime_t   run;
    void*           validation; // validation data (NULL if no validation)
} gvm_t;

#endif // GVM_TYPES_H_