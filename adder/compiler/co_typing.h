#ifndef CO_TYPING_H_
#define CO_TYPING_H_

#include "sh_utils.h"
#include "co_types.h"
#include "co_utils.h"
#include "sh_arena.h"
#include "co_trace.h"

typedef struct ctx_kvp_t {
    char* key;
    char* val;
} ctx_kvp_t;

typedef struct ctx_t {
    arena_t* arena;
    size_t size;
    size_t capacity;
    ctx_kvp_t* kvps;
} ctx_t;

typedef enum ctx_res_t {
    CTX_RES_OK,
    CTX_RES_OUT_OF_MEMORY,
    CTX_RES_FORBIDDEN
} ctx_res_t;

ctx_t* ctx_create(arena_t* a, size_t capacity);
ctx_t* ctx_clone(ctx_t* ctx);
ctx_res_t ctx_insert(ctx_t* ctx, char* key, char* val);
char* ctx_lookup(ctx_t* ctx, char* key);
char* ctx_infer(ctx_t* ctx, char* key);
void ctx_dump(ctx_t* ctx);

char* make_signature(arena_t* a, ast_node_t* n);

char* typecheck(arena_t* arena, trace_t* trace, ast_node_t* root);


#endif // CO_TYPING_H_