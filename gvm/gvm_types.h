#ifndef GVM_TYPES_H_
#define GVM_TYPES_H_

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t type_id_t;

typedef struct grid_t {
    int width;
    int height;
    type_id_t* data;
} grid_t;

typedef enum val_type_t {
    VAL_NUMBER,
    VAL_BOOL,
    VAL_CHAR,
    VAL_LIST
} val_type_t;

typedef struct val_t val_t;
typedef struct val_buffer_t val_buffer_t;

typedef uint32_t list_t;

typedef enum gvm_mem_location_t {
    MEM_LOC_CONST,
    MEM_LOC_STACK,
    MEM_LOC_HEAP
} gvm_mem_location_t;

#define GET_LIST_MEM_LOC(L) ((L) >> 28)
#define GET_LIST_LENGTH(L) (((L) & 0x0FFFFFFFFF) >> 14)
#define GET_LIST_OFFSET(L) (((L) & 0x3FFF))

typedef struct val_t {
    val_type_t type;
    union {
        int      n;
        char     c;
        bool     b;
        list_t   l;
    } data;
} val_t;

typedef enum token_type_t {
    TT_UNKNOWN,
    TT_COLON,
    TT_NUMBER,
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
    OP_DUP,
    OP_CMP_EQUAL,
    OP_CMP_LESS_THAN,
    OP_CMP_MORE_THAN,
    OP_PUSH,
    OP_POP,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_EXIT,
    OP_CALL_NATIVE,
    OP_RETURN,
    OP_OPCODE_COUNT
} gvm_op_t;

typedef struct u8buffer_t {
    int size;
    int capacity;
    uint8_t* data;
} u8buffer_t;

typedef struct val_buffer_t {
    int storage;
    int size;
    int capacity;
    val_t* values;
} val_buffer_t;

typedef struct byte_code_block_t {
    int size;
    uint8_t* data;
} byte_code_block_t;

typedef struct byte_code_header_t {
    uint16_t header_size;
    uint16_t const_bytes;
    uint16_t code_bytes;
} byte_code_header_t;

typedef struct env_t env_t;

typedef void (*func_t)(env_t* env);

typedef struct env_t {
    val_buffer_t constants;
    val_buffer_t heap;
    val_buffer_t stack;
    struct {
        int count;
        func_t funcs[16];
        char* names[16];
    } native;
} env_t;

#endif // GVM_TYPES_H_