#include "co_infer.h"
#include "co_ast.h"
#include "co_trace.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

tyctx_t* tyctx_create(arena_t* a, int capacity) {
    tyctx_t* ctx = (tyctx_t*) aalloc(a, sizeof(tyctx_t));
    ctx->arena = a;
    ctx->capacity = capacity;
    ctx->size = 0;
    ctx->kvps = (tyctx_kvp_t*) aalloc(a, sizeof(tyctx_kvp_t) * ctx->capacity);
    return ctx;
}

tyctx_t* tyctx_clone(tyctx_t* src) {
    tyctx_t* ctx = (tyctx_t*) aalloc(src->arena, sizeof(tyctx_t));
    ctx->arena = src->arena;
    ctx->capacity = src->capacity;
    ctx->size = src->size;
    ctx->kvps = (tyctx_kvp_t*) aalloc(src->arena, sizeof(tyctx_kvp_t) * ctx->capacity);
    memcpy(ctx->kvps, src->kvps, sizeof(tyctx_kvp_t) * src->size);
    return ctx;
}

typedef struct res_t {
    bool found;
    int index;
} res_t;

res_t tyctx_binsearch(tyctx_kvp_t* kvps, int low, int high, char* name) {
    if ( high >= low ) {
        int mid = low + (high - low) / 2;
        if (strcmp(kvps[mid].name, name) == 0)
            return (res_t) { true, mid };
        if (strcmp(kvps[mid].name, name) > 0)
            return tyctx_binsearch(kvps, low, mid - 1, name);
        return tyctx_binsearch(kvps, mid + 1, high, name);
    }
    return (res_t) { false, low };
}

bool tyctx_ensure_capacity(tyctx_t* ctx, int extra) {
    int required = (ctx->size + extra);
    if( required >= ctx->capacity ) {
        int new_cap = required * 2;
        tyctx_kvp_t* kvps = arealloc(ctx->arena, ctx->kvps, new_cap * sizeof(tyctx_kvp_t));
        if( kvps == NULL ) {
            return false;
        }
        ctx->capacity = new_cap;
        ctx->kvps = kvps;
    }
    return true;
}

void tyctx_make_room_at(tyctx_t* ctx, int index) {
    if( (int) ctx->size > index ) {
        memmove(&ctx->kvps[index+1],
            &ctx->kvps[index],
            sizeof(tyctx_kvp_t) * (ctx->size - index));
    }
    ctx->size++;
}

bool tyctx_insert(tyctx_t* ctx, char* name, mt_forall_t* scheme) {

    if( tyctx_ensure_capacity(ctx, 1) == false )
        return false;

    res_t res = tyctx_binsearch(ctx->kvps, 0, (int) ctx->size - 1, name);
    if( res.found == false ) {
        tyctx_make_room_at(ctx, res.index);
        ctx->kvps[res.index] = (tyctx_kvp_t) {
            .name = name,
            .scheme = scheme
        };
        return true;
    } 

    return false;
}

mt_forall_t* tyctx_lookup(tyctx_t* ctx, char* name) {
    res_t res = tyctx_binsearch(ctx->kvps, 0, (int) ctx->size - 1, name);
    if( res.found ) {
        return ctx->kvps[res.index].scheme;
    }
    return NULL;
}

void tyctx_dump(tyctx_t* ctx) {
    printf("ctx_dump (size=%d, capacity=%d)\n",
        ctx->size, ctx->capacity);
    for(int i = 0; i < ctx->size; i++) {
        printf("  \"%s\": \"%s\"\n", 
            ctx->kvps[i].name,
            "<todo>");
    }
}

mt_t* find(mt_t* ty) {
    if( ty->tag != MT_VAR )
        return ty;
    while ( ty->tag == MT_VAR) {
        mt_t* linked = ty->u.var.link;
        if( linked == NULL )
            break;
        ty = linked;
    }
    return ty;
}

bool connect(trace_t* trace, mt_t* var, mt_t* other) {
    assert(var->tag == MT_VAR);
    mt_t* end = find(var);
    if( end->tag != MT_VAR ) {
        trace_msg_t* msg = trace_create_message(trace, TM_ERROR, trace_no_ref());
        trace_msg_append_costr(msg, "type-check: variable already resolved (");
        trace_msg_append(msg, var->u.var.name, strlen(var->u.var.name));
        trace_msg_append_costr(msg, ")");
        return false;
    }
    end->u.var.link = other;
    return true;
}

void unification_failed(trace_t* trace, mt_t* a, mt_t* b) {
    (void)(a);
    (void)(b);
    trace_msg_t* msg = trace_create_message(trace, TM_ERROR, trace_no_ref());
    trace_msg_append_costr(msg, "type-check: unification failed.");
}

bool unify(trace_t* trace, mt_t* a, mt_t* b) {
    a = find(a);
    b = find(b);
    if( a->tag == MT_VAR ) {
        return connect(trace, a, b); // todo: occurs check
    } else if ( b->tag == MT_VAR ) {
        return connect(trace, b, a); // todo: occurs check
    } else if ( a->tag == MT_CON && b->tag == MT_CON ) {
        if( strcmp(a->u.con.name, b->u.con.name) != 0 ) {
            unification_failed(trace, a, b);
            return false;
        }
        if( a->u.con.argcnt != b->u.con.argcnt ) {
            unification_failed(trace, a, b);
            return false;
        }
        int cnt = a->u.con.argcnt;
        for(int i = 0; i < cnt; i++) {
            bool ok = unify(trace,
                a->u.con.args[i],
                b->u.con.args[i]);
            if( ok == false )
                return false;
        }
        return true;
    } else {
        trace_msg_t* msg = trace_create_message(trace, TM_ERROR, trace_no_ref());
        trace_msg_append_costr(msg, "type-check: unexpected type.");
        return false;
    }
}

mt_t* mt_var(arena_t* a, char* name) {
    mt_t* t = (mt_t*) aalloc(a, sizeof(mt_t));
    t->tag = MT_VAR;
    t->u.var.name = name;
    t->u.var.link = NULL;
    return t;
}

mt_t* mt_con(arena_t* a, char* name) {
    mt_t* t = (mt_t*) aalloc(a, sizeof(mt_t));
    t->tag = MT_CON;
    t->u.con.name = name;
    t->u.con.argcnt = 0;
    t->u.con.args = NULL;
    return t;
}

void mt_con_add_arg(arena_t* a, mt_t* con, mt_t* arg) {
    assert(con->tag == MT_CON);
    if( con->u.con.argcnt == 0 ) {
        assert(con->u.con.args == NULL);
        con->u.con.argcnt = 1;
        con->u.con.args = (mt_t**) aalloc(a,
            sizeof(mt_t*) * con->u.con.argcnt);
        assert(con->u.con.args != NULL);
    } else {
        con->u.con.argcnt += 1;
        con->u.con.args = (mt_t**) arealloc(a,
            con->u.con.args,
            sizeof(mt_t*) * con->u.con.argcnt);
    }
    con->u.con.args[con->u.con.argcnt - 1] = arg;
}

mt_forall_t* mt_forall(arena_t* a, mt_t* type) {
    mt_forall_t* fa = (mt_forall_t*) aalloc(a, sizeof(mt_forall_t));
    fa->type = type;
    fa->tyvarcnt = 0;
    fa->tyvars = NULL;
    return fa;
}

void mt_forall_add_var(arena_t* a, mt_forall_t* fa, mt_t* var) {
    // not sure if this actually needs to be a var
    assert(var->tag == MT_VAR);
    if( fa->tyvarcnt == 0 ) {
        assert(fa->tyvars == NULL);
        fa->tyvarcnt = 1;
        fa->tyvars = (mt_t**) aalloc(a,
            sizeof(mt_t*) * fa->tyvarcnt);
        assert(fa->tyvars != NULL);
    } else {
        fa->tyvarcnt += 1;
        fa->tyvars = (mt_t**) arealloc(a,
            fa->tyvars,
            sizeof(mt_t*) * fa->tyvarcnt);
    }
    fa->tyvars[fa->tyvarcnt - 1] = var;
}





