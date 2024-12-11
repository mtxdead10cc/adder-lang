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

res_t tyctx_binsearch(tyctx_kvp_t* kvps, int low, int high, srcref_t name) {
    if ( high >= low ) {
        int mid = low + (high - low) / 2;
        if (strncmp(kvps[mid].name, srcref_ptr(name), srcref_len(name)) == 0)
            return (res_t) { true, mid };
        if (strncmp(kvps[mid].name, srcref_ptr(name), srcref_len(name)) > 0)
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

bool tyctx_insert(tyctx_t* ctx, srcref_t name, mt_forall_t* scheme) {

    if( tyctx_ensure_capacity(ctx, 1) == false )
        return false;

    res_t res = tyctx_binsearch(ctx->kvps, 0, (int) ctx->size - 1, name);
    if( res.found == false ) {
        tyctx_make_room_at(ctx, res.index);
        ctx->kvps[res.index] = (tyctx_kvp_t) {
            .name = asprint(ctx->arena, "%.*s",
                srcref_len(name), srcref_ptr(name)),
            .scheme = scheme
        };
        return true;
    } 

    return false;
}


mt_forall_t* tyctx_lookup(tyctx_t* ctx, srcref_t name) {
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

// todo: ensure that the indices
// matches ast_value_type_t
static mt_t vtypes[] = {
    { .tag = MT_CON, .u.con = { "none", 0, NULL } },
    { .tag = MT_CON, .u.con = { "int", 0, NULL } },
    { .tag = MT_CON, .u.con = { "bool", 0, NULL } },
    { .tag = MT_CON, .u.con = { "char", 0, NULL } },
    { .tag = MT_CON, .u.con = { "float", 0, NULL } }
};

mt_t* ti_value(ast_value_type_t vt) {
    return &vtypes[(int)vt];
}

static size_t freshcntr = 0;

mt_t* ti_freshvar(tyctx_t* c) {
    return mt_var(c->arena,
                asprint(c->arena,
                    "var-%lu", freshcntr++));
}

mt_t* ti_infer(tyctx_t* c, trace_t* t, ast_node_t* n);

mt_t* ti_instantiate(tyctx_t* c, trace_t* t, mt_forall_t* fa) {
    (void)(t);
    (void)(c);
    // todo: implement this
    return fa->type;
}

mt_forall_t* ti_generalize(tyctx_t* c, trace_t* t, mt_t* val_ty) {
    (void)(t);
    mt_forall_t* fa = mt_forall(c->arena, val_ty);
    // todo: implement
    return fa;
}

mt_t* ti_array(tyctx_t* c, trace_t* t, ast_array_t array) {
    return NULL;
}

mt_t* ti_if(tyctx_t* c, trace_t* t, ast_if_t ifstmt) {
    return NULL;
}

mt_t* ti_foreach(tyctx_t* c, trace_t* t, ast_foreach_t fe) {
    return NULL;
}

mt_t* ti_binop(tyctx_t* c, trace_t* t, ast_binop_t bop) {
    return NULL;
}

mt_t* ti_unop(tyctx_t* c, trace_t* t, ast_unop_t uop) {
    return NULL;
}

mt_t* ti_assign(tyctx_t* c, trace_t* t, ast_assign_t an) {
    return NULL;
}

mt_t* ti_var(tyctx_t* c, trace_t* t, srcref_t name) {
    mt_t* var = ti_freshvar(c);
    mt_forall_t* fa = tyctx_lookup(c, name);
    if( fa == NULL ) {
        trace_msg_t* m = trace_create_message(t, TM_ERROR, name);
        trace_msg_append_costr(m, "type-check: unbound variable ");
        trace_msg_append_srcref(m, name);
    }
    unify(t, var, ti_instantiate(c, t, fa));
    return var;
}

mt_t* ti_vardecl(tyctx_t* c, trace_t* t, ast_vardecl_t vd) {
    return ti_var(c, t, vd.name);
}

mt_t* ti_varref(tyctx_t* c, trace_t* t, ast_varref_t vr) {
    return ti_var(c, t, vr.name);
}

srcref_t extract_var_name(ast_node_t* n) {
    if( n->type == AST_VAR_DECL )
        return n->u.n_vardecl.name;
    if( n->type == AST_VAR_REF )
        return n->u.n_varref.name;
    assert( false && "can't extract name from non-variable" );
    return (srcref_t) { 0 };
}

mt_t* ti_funsign(tyctx_t* c, trace_t* t, ast_funsign_t fs) {
    assert( fs.argspec->type == AST_BLOCK );
    ast_block_t blk = fs.argspec->u.n_block;
    // add all args to context
    // go with the a -> b -> c -> d approach!
    return NULL;
}

mt_t* ti_fundecl(tyctx_t* c, trace_t* t, ast_fundecl_t fd) {
    mt_t* var = ti_freshvar(c);
    tyctx_t* nc = tyctx_clone(c);
    

    
    return NULL;
}

mt_t* ti_funcall(tyctx_t* c, trace_t* t, ast_funcall_t fc) {
    return NULL;
}

mt_t* ti_return(tyctx_t* c, trace_t* t, ast_return_t ret) {
    return NULL;
}

mt_t* ti_break(tyctx_t* c, trace_t* t) {
    return NULL;
}

mt_t* ti_block(tyctx_t* c, trace_t* t, ast_block_t blk) {
    return NULL;
}

mt_t* ti_infer(tyctx_t* c, trace_t* t, ast_node_t* n) {
    switch(n->type) {
        case AST_VALUE:     return ti_value(n->u.n_value.type);
        case AST_ARRAY:     return ti_array(c, t, n->u.n_array);
        case AST_IF_CHAIN:  return ti_if(c, t, n->u.n_if);
        case AST_FOREACH:   return ti_foreach(c, t, n->u.n_foreach);
        case AST_BINOP:     return ti_binop(c, t, n->u.n_binop);
        case AST_UNOP:      return ti_unop(c, t, n->u.n_unop);
        case AST_ASSIGN:    return ti_assign(c, t, n->u.n_assign);
        case AST_VAR_DECL:  return ti_vardecl(c, t, n->u.n_vardecl);
        case AST_VAR_REF:   return ti_varref(c, t, n->u.n_varref);
        case AST_FUN_SIGN:  return ti_funsign(c, t, n->u.n_funsign);
        case AST_FUN_DECL:  return ti_fundecl(c, t, n->u.n_fundecl);
        case AST_FUN_CALL:  return ti_funcall(c, t, n->u.n_funcall);
        case AST_RETURN:    return ti_return(c, t, n->u.n_return);
        case AST_BREAK:     return ti_break(c, t);
        case AST_BLOCK:     return ti_block(c, t, n->u.n_block);
        default: {
            assert(false && "unhandled type");
            return ti_value(AST_VALUE_NONE);
        }
    }
}





