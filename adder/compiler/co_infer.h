#ifndef CO_INFER_H_
#define CO_INFER_H_

#include <stdbool.h>
#include <stdint.h>
#include "sh_arena.h"

typedef enum mt_tag_t {
    MT_UNK,
    MT_VAR,
    MT_CON
} mt_tag_t;

typedef struct mt_t mt_t;

typedef struct mt_var_t {
    char* name;
    mt_t* link;
} mt_var_t;

typedef struct mt_con_t {
    char* name;
    int argcnt;
    mt_t** args;
} mt_con_t;

typedef struct mt_t {
    mt_tag_t tag;
    union {
        mt_var_t var;
        mt_con_t con;
    } u;
} mt_t;

typedef struct mt_forall_t {
    mt_t*       type;
    int         tyvarcnt;
    mt_t**      tyvars;
} mt_forall_t;

typedef struct tyctx_kvp_t {
    char*           name;
    mt_forall_t*    scheme;
} tyctx_kvp_t;

typedef struct tyctx_t {
    arena_t* arena;
    int size;
    int capacity;
    tyctx_kvp_t* kvps;
} tyctx_t;

tyctx_t*        tyctx_create(arena_t* a, int capacity);
tyctx_t*        tyctx_clone(tyctx_t* ctx);
bool            tyctx_insert(tyctx_t* ctx, char* name, mt_forall_t* scheme);
mt_forall_t*    tyctx_lookup(tyctx_t* ctx, char* name);
void            tyctx_dump(tyctx_t* ctx);

mt_forall_t* mt_forall(arena_t* a, mt_t* type);
void         mt_forall_add_var(arena_t* a, mt_forall_t* fa, mt_t* var);
mt_t*        mt_con(arena_t* a, char* name);
void         mt_con_add_arg(arena_t* a, mt_t* con, mt_t* arg);
mt_t*        mt_var(arena_t* a, char* name);

typedef struct trace_t trace_t;

mt_t* find(mt_t* ty); // union-find-find
bool connect(trace_t* trace, mt_t* var, mt_t* other);
bool unify(trace_t* trace, mt_t* a, mt_t* b);

#endif // CO_INFER_H_