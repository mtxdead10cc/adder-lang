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

typedef struct list_t {
    int len;
    val_t* ptr;
} list_t;

typedef struct val_t {
    val_type_t type;
    union {
        int16_t n;
        char    c;
        bool    b;
        list_t  l;
    } data;
} val_t;

typedef struct gvm_chunk_t {
    int len;
    uint8_t* instructions;
} gvm_chunk_t;

typedef enum token_type_t {
    TT_UNKNOWN,
    TT_COLON,
    TT_NUMBER,
    TT_STRING,
    TT_SYMBOL,
    TT_SEPARATOR,
    TT_END
} token_type_t;

typedef struct token_t {
    token_type_t type;
    int src_index;
    int src_line;
    int src_column;
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
    RES_OUT_OF_MEMORY,
    RES_ERROR
} gvm_result_t;

#endif // GVM_TYPES_H_