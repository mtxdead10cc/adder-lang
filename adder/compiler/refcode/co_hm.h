#ifndef HM_H_
#define HM_H_

/* Hinly-Milner
    Guide
    https://bernsteinbear.com/blog/type-inference/
*/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#include "co_types.h"
#include "sh_arena.h"
#include "sh_types.h"
#include "sh_utils.h"
#include "co_utils.h"

typedef enum tyexp_kind_t {
    TYEXP_VAR,
    TYEXP_INT,
    TYEXP_FLOAT,
    TYEXP_BOOL,
    TYEXP_CHAR,
    TYEXP_LIST,
    TYEXP_FUNCTION,
    TYEXP_APPLY,
    TYEXP_WHERE,
    TYEXP_ASSIGN
} tyexp_kind_t;

typedef struct tyexp_t tyexp_t;
typedef struct tyexp_t {
    tyexp_kind_t kind;
    union  {
        struct {
            sstr_t name;
        } var;
        struct {
            tyexp_t* arg;
            tyexp_t* body;
        } fun;
        struct {
            tyexp_t* func;
            tyexp_t* arg;
        } app;
        struct {
            tyexp_t* binding;
            tyexp_t* body;
        } where;
        struct {
            tyexp_t* name;
            tyexp_t* value;
        } assign;
    };
} tyexpr_t;


inline static tyexp_t* tyexp_var(arena_t* a, char* name) {
    tyexp_t* ty = (tyexp_t*) aalloc(a, sizeof(tyexp_t));
    ty->kind = TYEXP_VAR;
    ty->var.name = sstr(name);
    return ty;
}

inline static tyexp_t* tyexp_int(arena_t* a) {
    tyexp_t* ty = (tyexp_t*) aalloc(a, sizeof(tyexp_t));
    ty->kind = TYEXP_INT;
    return ty;
}

inline static tyexp_t* tyexp_bool(arena_t* a) {
    tyexp_t* ty = (tyexp_t*) aalloc(a, sizeof(tyexp_t));
    ty->kind = TYEXP_BOOL;
    return ty;
}

inline static tyexp_t* tyexp_function(arena_t* a, tyexp_t* arg, tyexp_t* body) {
    tyexp_t* ty = (tyexp_t*) aalloc(a, sizeof(tyexp_t));
    ty->kind = TYEXP_FUNCTION;
    ty->fun.arg = arg;
    ty->fun.body = body;
    return ty;
}

inline static tyexp_t* tyexp_application(arena_t* a, tyexp_t* arg, tyexp_t* func) {
    tyexp_t* ty = (tyexp_t*) aalloc(a, sizeof(tyexp_t));
    ty->kind = TYEXP_APPLY;
    ty->app.arg = arg;
    ty->app.func = func;
    return ty;
}

inline static tyexp_t* tyexp_where(arena_t* a, tyexp_t* body, tyexp_t* binding) {
    assert(binding->kind == TYEXP_ASSIGN);
    tyexp_t* ty = (tyexp_t*) aalloc(a, sizeof(tyexp_t));
    ty->kind = TYEXP_WHERE;
    ty->where.binding = binding;
    ty->where.body = body;
    return ty;
}

inline static tyexp_t* tyexp_assign(arena_t* a, tyexp_t* var, tyexp_t* value) {
    assert(var->kind == TYEXP_VAR);
    tyexp_t* ty = (tyexp_t*) aalloc(a, sizeof(tyexp_t));
    ty->kind = TYEXP_ASSIGN;
    ty->assign.name = var;
    ty->assign.value = value;
    return ty;
}

typedef enum hm_tag_t {
    HM_TYPE,
    HM_VAR,
    HM_CON,
    HM_FORALL
} hm_tag_t;

typedef struct hm_type_t hm_type_t;

typedef struct hm_var_t {
    sstr_t      name;
    hm_type_t*  forward; // union find
} hm_var_t;

typedef struct hm_constant_t {
    sstr_t      name;
    struct {
        size_t count;
        hm_type_t* array[2];
    } args;
} hm_constant_t;

typedef struct hm_forall_t {
    struct {
        size_t count;
        hm_var_t** array;
    } type_vars;
    hm_type_t* type;
} hm_forall_t;

typedef struct hm_type_t {
    hm_tag_t tag;
    union {
        hm_var_t       var;
        hm_constant_t  con;
        hm_forall_t    forall;
    } u;
} hm_type_t;

typedef struct hm_map_kvp_t {
    sstr_t*      key;
    hm_forall_t* val;
} hm_map_kvp_t;

typedef struct hm_map_t {
    hm_map_kvp_t* kvps; 
    size_t        count;
    size_t        capacity;
} hm_map_t;

hm_map_t* hm_map_copy(arena_t* a, hm_map_t* other);
hm_forall_t* hm_map_get(hm_map_t* m, sstr_t* name);
void hm_map_add(arena_t* a, hm_map_t* m, sstr_t* name, hm_forall_t* scheme);

hm_type_t* hm_type_find(hm_type_t* ty);
void hm_type_make_equal_to(hm_type_t* ty, hm_type_t* other_ty);

void hm_print(hm_type_t* ty);

bool hm_unify(hm_type_t* a, hm_type_t* b);

hm_forall_t* hm_forall(arena_t* arena, hm_type_t* into);
hm_type_t* hm_make(arena_t* arena, hm_tag_t tag);

inline static hm_type_t* hm_var(arena_t* arena, char* name) {
    hm_type_t* vt = hm_make(arena, HM_VAR);
    vt->u.var.name = sstr(name);
    return vt;
}

hm_type_t* hm_infer(arena_t* a, tyexp_t* e);
hm_type_t* hm_infer_rec(arena_t* a, tyexp_t* e, hm_map_t* ctx);

#endif // HM_H_