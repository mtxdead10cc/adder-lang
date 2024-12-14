#ifndef CO_BTY_H_
#define CO_BTY_H_

#include "sh_arena.h"
#include "co_types.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum bty_errcode_t {
    BTY_ERR_TYPECHECK,
    BTY_ERR_INTERNAL
} bty_errcode_t;

// Bidirectional Type Checker

typedef enum bty_tag_t {
    BTY_VOID,       // 'unusable' return value
    BTY_FLOAT,
    BTY_INT,
    BTY_CHAR,
    BTY_BOOL,
    BTY_SOMETIMES,
    BTY_ALWAYS,
    BTY_LIST,
    BTY_FUNC,
    BTY_RETURN,
    BTY_ERROR
} bty_tag_t;

typedef struct trace_t trace_t;
typedef struct bty_type_t bty_type_t;

typedef struct bty_error_t {
    uint32_t erc; // error code
} bty_error_t;

typedef struct bty_fun_t {
    int          argc;
    bool         exported;
    bty_type_t** args;
    bty_type_t*  ret;
} bty_fun_t;

typedef struct bty_type_t {
    bty_tag_t           tag;
    union {
        bty_fun_t       fun;    // function type
        bty_error_t     err;    // error
        bty_type_t*     con;    // wrapper types
    } u;
} bty_type_t;

typedef struct bty_ctx_kvp_t {
    char*           name;
    bty_type_t*     type;
} bty_ctx_kvp_t;

typedef struct bty_ctx_t {
    arena_t* arena;
    trace_t* trace;
    int size;
    int capacity;
    bty_ctx_kvp_t* kvps;
} bty_ctx_t;


bty_type_t* bty_void(void);
bty_type_t* bty_float(void);
bty_type_t* bty_int(void);
bty_type_t* bty_char(void);
bty_type_t* bty_bool(void);

bty_type_t* bty_func(arena_t* a);
void bty_func_add_arg(arena_t* a, bty_type_t* fun, bty_type_t* arg);
void bty_func_set_return_type(bty_type_t* fun, bty_type_t* arg);
bty_type_t* bty_list(arena_t* a, bty_type_t* content_type);

bty_type_t* bty_error(arena_t* a, uint32_t erc);

bool bty_is_error(bty_type_t* ty);
bool bty_is_void(bty_type_t* ty);
bool bty_is_float(bty_type_t* ty);
bool bty_is_int(bty_type_t* ty);
bool bty_is_char(bty_type_t* ty);
bool bty_is_bool(bty_type_t* ty);
bool bty_is_func(bty_type_t* ty);
bool bty_is_list(bty_type_t* ty);

char* sprint_bty_type(arena_t* a, bty_type_t* ty);

bty_ctx_t* bty_ctx_create(arena_t* a, trace_t* t, int capacity);
bool bty_ctx_insert(bty_ctx_t* ctx, srcref_t name, bty_type_t* type);
bty_type_t* bty_ctx_lookup(bty_ctx_t* ctx, srcref_t name);
bty_ctx_t* bty_ctx_clone(bty_ctx_t* src);

void bty_ctx_dump(bty_ctx_t* ctx);
bty_type_t* bty_synthesize(bty_ctx_t* c, ast_node_t* n);
bool bty_typecheck(bty_ctx_t* ctx, ast_node_t* program);


#endif // CO_BTY_H_
