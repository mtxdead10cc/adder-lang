#include "co_hm.h"
#include "sh_utils.h"
#include <stdarg.h>

hm_type_t* hm_type_find(hm_type_t* ty) {
    if( ty->tag != HM_VAR )
        return ty;
    hm_type_t* res = ty;
    while ( res->tag == HM_VAR ) {
        if( res->u.var.next == NULL )
            return res;
        res = res->u.var.next;
    }
    return res;
}

void hm_type_make_equal_to(hm_type_t* ty, hm_type_t* other_ty) {
    hm_type_t* chain_end = hm_type_find(ty);
    assert(chain_end->tag == HM_VAR && "already resolved to something else");
    chain_end->u.var.next = other_ty;
}

bool hm_unify(hm_type_t* a, hm_type_t* b) {
    a = hm_type_find(a);
    b = hm_type_find(b);
    if( a->tag == HM_VAR ) {
        // todo: add occurs check
        hm_type_make_equal_to(a, b);
        return true;
    } else if ( b->tag == HM_VAR ) {
        return hm_unify(b, a); // swap arg order
    } else if ( a->tag == HM_CON && b->tag == HM_CON ) {
        hm_constant_t ca = a->u.con;
        hm_constant_t cb = b->u.con;
        if( ca.args.count != cb.args.count )
            return false;   // todo: report error
        if( sstr_equal(&ca.name, &cb.name) == false )
            return false;   // todo: report error
        size_t count = ca.args.count;
        hm_type_t** aargs = ca.args.array;
        hm_type_t** bargs = cb.args.array;
        for(size_t i = 0; i < count; i++) {
            if( hm_unify(aargs[i], bargs[i]) == false )
                return false;
        }
        return true;
    }
    assert(false && "unexpected type");
    return false;
}

hm_type_t* hm_make(arena_t* arena, hm_tag_t tag) {
    hm_type_t* ty = (hm_type_t*) aalloc(arena, sizeof(hm_type_t));
    ty->tag = tag;
    return ty;
}

hm_type_t* hm_fresh_var(arena_t* arena, char* name) {
    static size_t counter = 0;
    hm_type_t* ty = hm_make(arena, HM_VAR);
    ty->u.var.next = NULL;
    if( name != NULL ) {
        ty->u.var.name = sstr(name);
    } else {
        sstr_format(&ty->u.var.name, "var%lu", counter++);
    }
    return ty;
}

hm_type_t* hm_forall(arena_t* arena, hm_type_t* into) {
    hm_type_t* ty = hm_make(arena, HM_FORALL);
    ty->u.forall.type = into;
    ty->u.forall.type_vars.count = 0;
    return ty;
}

static hm_type_t hm_bool = {
    .tag = HM_CON,
    .u.con = {
        .args = { 0 },
        .name.str = "bool"
    }
};

static hm_type_t hm_int = {
    .tag = HM_CON,
    .u.con = {
        .args = { 0 },
        .name.str = "int"
    }
};

static hm_type_t hm_float = {
    .tag = HM_CON,
    .u.con = {
        .args = { 0 },
        .name.str = "float"
    }
};

static hm_type_t hm_char = {
    .tag = HM_CON,
    .u.con = {
        .args = { 0 },
        .name.str = "char"
    }
};

// https://bernsteinbear.com/blog/type-inference/
hm_type_t* hm_infer(arena_t* a, tyexp_t* e, hm_map_t* ctx) {
    hm_type_t* res = hm_fresh_var(a, NULL);
    switch (e->kind) {
        case TYEXP_BOOL:    return &hm_bool;
        case TYEXP_CHAR:    return &hm_char;
        case TYEXP_INT:     return &hm_int;
        case TYEXP_FLOAT:   return &hm_float;
        case TYEXP_VAR: {
            hm_type_t* scheme = hm_map_get(ctx, &e->var.name);
            if( scheme == NULL ) {
                assert(false && "unbound variable expr.name");
                return NULL;
            }
            if( hm_unify(res, scheme->u.forall.type) == false ) {
                assert(false && "unify failed.");
                return NULL;
            }
        } break;
        case TYEXP_FUNCTION: {
            hm_type_t* arg_tyvar = hm_fresh_var(a, "a");
            assert(e->fun.arg->kind == TYEXP_VAR);
            hm_map_t* tmp_ctx = hm_map_copy(ctx);
            hm_map_set(tmp_ctx, &e->fun.arg->var.name, hm_forall(a, arg_tyvar));
            hm_type_t* body_type = hm_infer(a, e->fun.body, tmp_ctx);
            hm_map_destroy(tmp_ctx);

            hm_type_t* ty = hm_make(a, HM_CON);
            ty->u.con.name = sstr("->");
            ty->u.con.args.count = 2;
            ty->u.con.args.array[0] = arg_tyvar;
            ty->u.con.args.array[1] = body_type;
            if( hm_unify(res, ty) == false ) {
                assert(false && "unify failed.");
                return NULL;
            }
        } break;
        case TYEXP_APPLY: {
            hm_type_t* func_ty = hm_infer(a, e->app.func, ctx);
            hm_type_t* arg_ty = hm_infer(a, e->app.arg, ctx);

            hm_type_t* ty = hm_make(a, HM_CON);
            ty->u.con.name = sstr("->");
            ty->u.con.args.count = 2;
            ty->u.con.args.array[0] = arg_ty;
            ty->u.con.args.array[1] = res;
            if( hm_unify(func_ty, ty) == false ) {
                assert(false && "unify failed.");
                return NULL;
            }
        } break;
        case TYEXP_DEFINE: {
            sstr_t* name = &e->def.bind_name->var.name;
            tyexp_t* value = e->def.bind_value;
            tyexp_t* body = e->def.body;
            hm_type_t* value_ty = hm_infer(a, value, ctx);
            hm_map_t* tmp_ctx = hm_map_copy(ctx);
            hm_map_set(tmp_ctx, name, hm_forall(a, value_ty));
            hm_type_t* body_ty = hm_infer(a, body, tmp_ctx);
            hm_map_destroy(tmp_ctx);
            if( hm_unify(res, body_ty) == false ) {
                assert(false && "unify failed.");
                return NULL;
            }
        } break;
        default: {
            assert(false && "unexpected type");
        } break;
    }

    return res;
}
