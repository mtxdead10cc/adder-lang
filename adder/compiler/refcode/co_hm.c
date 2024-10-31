#include "co_hm.h"
#include "sh_utils.h"
#include <stdarg.h>


hm_map_t* hm_map_copy(arena_t* a, hm_map_t* other) {
    hm_map_t* m = (hm_map_t*) aalloc(a, sizeof(hm_map_t));
    assert(m != NULL);
    m->kvps = (hm_map_kvp_t*) aalloc(a, sizeof(hm_map_kvp_t) * other->capacity);
    assert(m->kvps != NULL);
    m->capacity = other->capacity;
    m->count = other->count;
    memcpy(m->kvps, other->kvps, sizeof(hm_map_kvp_t) * other->count);
    return m;
}

hm_forall_t* hm_map_get(hm_map_t* m, sstr_t* name) {
    for(size_t i = 0; i < m->count; i++) {
        if( sstr_equal(m->kvps[i].key, name) )
            return m->kvps[i].val;
    }
    return NULL;
}

void hm_map_add(arena_t* a, hm_map_t* m, sstr_t* name, hm_forall_t* scheme) {
    if( m->kvps == NULL && m->capacity == 0 ) {
        m->kvps = aalloc(a, sizeof(hm_forall_t) * 4);
        m->capacity = 4;
    }
    if( m->count >= m->capacity ) {
        ptrdiff_t new_cap = (m->count + 1) * 2;
        m->kvps = arealloc(a, m->kvps, sizeof(hm_forall_t) * new_cap);
        m->capacity = new_cap;
        assert(m->kvps != NULL);
    }
    m->kvps[m->count++] = (hm_map_kvp_t) {
        .key = name,
        .val = scheme
    };
}

hm_type_t* hm_type_find(hm_type_t* ty) {
    if( ty->tag != HM_VAR )
        return ty;
    hm_type_t* res = ty;
    while ( res->tag == HM_VAR ) {
        if( res->u.var.forward == NULL )
            break;
        res = res->u.var.forward;
    }
    return res;
}

void hm_type_make_equal_to(hm_type_t* ty, hm_type_t* other_ty) {
    hm_type_t* chain_end = hm_type_find(ty);
    assert(chain_end->tag == HM_VAR && "already resolved to something else");
    chain_end->u.var.forward = other_ty;
}

void hm_print(hm_type_t* ty) {
    switch(ty->tag) {
        case HM_CON: {
            printf("[");
            printf("con ");
            sstr_printf(&ty->u.con.name);
            printf(" [");
            for(size_t i = 0; i < ty->u.con.args.count; i++) {
                hm_print(ty->u.con.args.array[i]); printf(" ");
            }
            printf("]");
            printf("]");
        } break;
        case HM_VAR: {
            //printf("[");
            while ( ty->tag == HM_VAR ) {
                //printf("var ");
                //sstr_printf(&ty->u.var.name);
                if( ty->u.var.forward == NULL ) {
                    //printf("]");
                    printf("var ");
                    sstr_printf(&ty->u.var.name);
                    return;
                }
                //printf(" => ");
                ty = ty->u.var.forward;
            }
            hm_print(ty);
            //printf("]");
        } break;
        case HM_FORALL: {
            printf("[");
            printf("forall ");
            printf(" [");
            for(size_t i = 0; i < ty->u.forall.type_vars.count; i++) {
                hm_var_t* var = ty->u.forall.type_vars.array[i];
                printf("var "); sstr_printf(&var->name); printf(" ");
            }
            printf("] ");
            hm_print(ty->u.forall.type);
            printf("]");
        } break;
        default: {
            printf("<unk>");
        } break;
    }
}

bool hm_unify(hm_type_t* a, hm_type_t* b) {

    a = hm_type_find(a);
    b = hm_type_find(b);

    printf("unify "); hm_print(a); printf(" :: "); hm_print(b); printf("\n"); 

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
    ty->u.var.forward = NULL;
    if( name != NULL ) {
        ty->u.var.name = sstr(name);
    } else {
        sstr_format(&ty->u.var.name, "$%lu", counter++);
    }
    return ty;
}

hm_forall_t* hm_forall(arena_t* arena, hm_type_t* into) {
    hm_forall_t* fa = (hm_forall_t*) aalloc(arena, sizeof(hm_forall_t));
    fa->type = into;
    fa->type_vars.count = 0;
    fa->type_vars.array = NULL;
    return fa;
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

/*static hm_type_t hm_float = {
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
};*/

// https://bernsteinbear.com/blog/type-inference/
hm_type_t* hm_infer_rec(arena_t* a, tyexp_t* e, hm_map_t* ctx) {
    hm_type_t* res = hm_fresh_var(a, NULL);
    switch (e->kind) {
        case TYEXP_BOOL: {
            if( hm_unify(res, &hm_bool) == false ) {
                assert(false && "unify bool failed.");
                return NULL;
            }
        } break;
        case TYEXP_INT: {
            if( hm_unify(res, &hm_int) == false ) {
                assert(false && "unify int failed.");
                return NULL;
            }
        } break;
        case TYEXP_VAR: {
            hm_forall_t* scheme = hm_map_get(ctx, &e->var.name);
            if( scheme == NULL ) {
                assert(false && "unbound variable expr.name");
                return NULL;
            }
            if( hm_unify(res, scheme->type) == false ) {
                assert(false && "unify var failed.");
                return NULL;
            }
        } break;
        case TYEXP_FUNCTION: {
            hm_type_t* arg_tyvar = hm_fresh_var(a, "a");
            assert(e->fun.arg->kind == TYEXP_VAR);
            hm_map_t* tmp_ctx = hm_map_copy(a, ctx);
            hm_map_add(a, tmp_ctx, &e->fun.arg->var.name, hm_forall(a, arg_tyvar));
            hm_type_t* body_type = hm_infer_rec(a, e->fun.body, tmp_ctx);

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
            hm_type_t* func_ty = hm_infer_rec(a, e->app.func, ctx);
            hm_type_t* arg_ty = hm_infer_rec(a, e->app.arg, ctx);
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
        case TYEXP_WHERE: {
            assert(e->where.binding->kind == TYEXP_ASSIGN);
            sstr_t* name = &e->where.binding->assign.name->var.name;
            tyexp_t* value = e->where.binding->assign.value;
            tyexp_t* body = e->where.body;
            hm_type_t* value_ty = hm_infer_rec(a, value, ctx);
            hm_map_t* tmp_ctx = hm_map_copy(a, ctx);
            hm_map_add(a, tmp_ctx, name, hm_forall(a, value_ty));
            hm_type_t* body_ty = hm_infer_rec(a, body, tmp_ctx);
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

hm_type_t* hm_infer(arena_t* a, tyexp_t* e) {
    hm_map_t m = { 0 };
    return hm_infer_rec(a, e, &m);
}
