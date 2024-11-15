#include "co_bty.h"
#include <assert.h>
#include <string.h>
#include "co_types.h"
#include "co_utils.h"
#include "co_trace.h"
#include "co_ast.h"

static bty_type_t bty_base_types[] = {
    { BTY_UNKNOWN,      {{0}} },
    { BTY_VOID,         {{0}} },
    { BTY_FLOAT,        {{0}} },
    { BTY_INT,          {{0}} },
    { BTY_CHAR,         {{0}} },
    { BTY_BOOL,         {{0}} }
};

#define verify_base_type_array_member(I) \
    assert(bty_base_types[I].tag == I && "error: base type array ordering");

bool bty_is_subtype(bty_type_t* child, bty_type_t* parent);
bool bty_is_equal(bty_type_t* a, bty_type_t* b);
bty_type_t* bty_from_const_expr(arena_t* a, ast_node_t* e);
bty_type_t* bty_synthesize(bty_ctx_t* c, ast_node_t* n);
void bty_check(bty_ctx_t* c, ast_node_t* n, bty_type_t* t);


bty_type_t* bty_unknown(void) {
    verify_base_type_array_member(BTY_UNKNOWN);
    return &bty_base_types[BTY_UNKNOWN];
}

bty_type_t* bty_void(void) {
    verify_base_type_array_member(BTY_VOID);
    return &bty_base_types[BTY_VOID];
}

bty_type_t* bty_float(void) {
    verify_base_type_array_member(BTY_FLOAT);
    return &bty_base_types[BTY_FLOAT];
}

bty_type_t* bty_int(void) {
    verify_base_type_array_member(BTY_INT);
    return &bty_base_types[BTY_INT];
}

bty_type_t* bty_char(void) {
    verify_base_type_array_member(BTY_CHAR);
    return &bty_base_types[BTY_CHAR];
}

bty_type_t* bty_bool(void) {
    verify_base_type_array_member(BTY_BOOL);
    return &bty_base_types[BTY_BOOL];
}

bty_type_t* bty_from_const_array(arena_t* a, ast_array_t ar) {
    if( ar.count == 0 )
        return bty_list(a, bty_unknown());

    bty_type_t* inner_type = bty_from_const_expr(a, ar.content[0]);
    if( inner_type == NULL )
        return NULL;

    for(size_t i = 1; i < ar.count; i++) {
        bty_type_t* tmp = bty_from_const_expr(a, ar.content[i]);
        if( tmp == NULL )
            return NULL;
        if( bty_is_equal(inner_type, tmp) == false )
            return NULL;
    }

    return bty_list(a, inner_type);
}

bty_type_t* bty_from_const_expr(arena_t* a, ast_node_t* e) {
    switch(e->type) {
    case AST_VALUE: {
        ast_value_t v = e->u.n_value;
        switch(v.type) {
            case AST_VALUE_INT:     return bty_int();
            case AST_VALUE_BOOL:    return bty_bool();
            case AST_VALUE_CHAR:    return bty_char();
            case AST_VALUE_FLOAT:   return bty_float();
            default:                return NULL;
        }
    }
    case AST_ARRAY:                 return bty_from_const_array(a, e->u.n_array);
    default:                        return NULL;
    }
}

bty_type_t* bty_func(arena_t* a, bty_type_t* ret) {
    bty_type_t* fun = (bty_type_t*) aalloc(a, sizeof(bty_type_t));
    fun->tag = BTY_FUNC;
    fun->u.fun.argc = 0;
    fun->u.fun.args = NULL;
    fun->u.fun.ret = ret;
    return fun;
}

void bty_func_add_arg(arena_t* a, bty_type_t* fun, bty_type_t* arg) {
    assert(fun->tag == BTY_FUNC);
    if( fun->u.fun.args == NULL ) {
        assert(fun->u.fun.argc == 0);
        fun->u.fun.argc = 1;
        fun->u.fun.args = (bty_type_t**) aalloc(a,
            sizeof(bty_type_t*) * fun->u.fun.argc);
    } else {
        assert(fun->u.fun.argc > 0);
        fun->u.fun.argc += 1;
        fun->u.fun.args = (bty_type_t**) arealloc(a,
            fun->u.fun.args,
            sizeof(bty_type_t*) * fun->u.fun.argc);
    }
    fun->u.fun.args[fun->u.fun.argc - 1] = arg;
}

bty_type_t* bty_list(arena_t* a, bty_type_t* content_type) {
    bty_type_t* lst = (bty_type_t*) aalloc(a, sizeof(bty_type_t));
    lst->tag = BTY_LIST;
    lst->u.lst.ctype = content_type;
    return lst;
}

bool bty_is_unknown(bty_type_t* ty) {
    return ty->tag == BTY_UNKNOWN;
}

bool bty_is_float(bty_type_t* ty) {
    return ty->tag == BTY_FLOAT;
}

bool bty_is_int(bty_type_t* ty) {
    return ty->tag == BTY_INT;
}

bool bty_is_char(bty_type_t* ty) {
    return ty->tag == BTY_CHAR;
}

bool bty_is_bool(bty_type_t* ty) {
    return ty->tag == BTY_BOOL;
}

bool bty_is_func(bty_type_t* ty) {
    return ty->tag == BTY_FUNC;
}

bool bty_is_list(bty_type_t* ty) {
    return ty->tag == BTY_LIST;
}

char* sprint_bty_type(arena_t* a, bty_type_t* ty) {
    switch(ty->tag) {
        case BTY_UNKNOWN: {
            return asprintf(a, "unknown");
        } break;
        case BTY_VOID: {
            return asprintf(a, "void");
        } break;
        case BTY_FLOAT: {
            return asprintf(a, "float");
        } break;
        case BTY_INT: {
            return asprintf(a, "int");
        } break;
        case BTY_CHAR: {
            return asprintf(a, "char");
        } break;
        case BTY_BOOL: {
            return asprintf(a, "bool");
        } break;
        case BTY_LIST: {
            return asprintf(a,
                "list of %s", 
                sprint_bty_type(a, 
                    ty->u.lst.ctype));
        } break;
        case BTY_FUNC: {
            char* text = "";
            for(int i = 0; i < ty->u.fun.argc; i++) {
                if( i > 0 )
                    text = asprintf(a, "%s,", text);
                text = asprintf(a, 
                    "%s %s", text,
                    sprint_bty_type(a,
                        ty->u.fun.args[i]));
            }
            return asprintf(a, 
                "func (%s) -> %s", text,
                sprint_bty_type(a, ty->u.fun.ret));
        } break;
        default: {
            return "";
        }
    }
}

/* SUBTYPES 
    FLOAT > INT
    INT > CHAR
    CHAR > BOOL
    STRING == ARRAY OF CHAR
    FUN 
        ARGS APP <= DEF
        APP RESULT >= DEF RETURN  
*/



bool bty_is_func_subtype(bty_type_t* child, bty_type_t* parent) {
    assert(child->tag == BTY_FUNC);
    if( parent->tag != BTY_FUNC )
        return false;
    if( child->u.fun.argc != parent->u.fun.argc )
        return false;
    int len = child->u.fun.argc;
    for(int i = 0; i < len; i++) {
        bty_type_t* ca = child->u.fun.args[i];
        bty_type_t* pa = child->u.fun.args[i];
        if( bty_is_subtype(ca, pa) == false )
            return false;
    }
    return bty_is_subtype(child->u.fun.ret,
        parent->u.fun.ret);
}

bool bty_is_list_subtype(bty_type_t* child, bty_type_t* parent) {
    assert(child->tag == BTY_LIST);
    if( parent->tag != BTY_LIST )
        return false;
    return bty_is_subtype(child->u.lst.ctype, parent->u.lst.ctype);
}


bool bty_is_subtype(bty_type_t* child, bty_type_t* parent) {
    switch(child->tag) {
        case BTY_BOOL:      return parent->tag == BTY_BOOL
                                || parent->tag == BTY_INT
                                || parent->tag == BTY_CHAR
                                || parent->tag == BTY_FLOAT;
        case BTY_CHAR:      return parent->tag == BTY_INT
                                || parent->tag == BTY_CHAR
                                || parent->tag == BTY_FLOAT;
        case BTY_INT:       return parent->tag == BTY_INT
                                || parent->tag == BTY_FLOAT;
        case BTY_FLOAT:     return parent->tag == BTY_FLOAT;
        case BTY_UNKNOWN:   return false;
        case BTY_VOID:      return false;
        case BTY_LIST:      return bty_is_list_subtype(child, parent);
        case BTY_FUNC:      return bty_is_func_subtype(child, parent);
        default: {
            printf("bty_is_subtype: unknown type\n");
            return false;
        }
    }
}

bool bty_is_equal(bty_type_t* a, bty_type_t* b) {
    if( a->tag != b->tag )
        return false;
    if( a->tag == BTY_LIST )
        return bty_is_equal(a->u.lst.ctype, b->u.lst.ctype);
    if( a->tag == BTY_FUNC ) {
        bty_fun_t af = a->u.fun;
        bty_fun_t bf = b->u.fun;
        if( af.argc != bf.argc )
            return false;
        for(int i = 0; i < af.argc; i++) {
            if( bty_is_equal(af.args[i], bf.args[i]) == false )
                return false;
        }
        return bty_is_equal(af.ret, bf.ret);
    }
    return true;
}

/////// CONTEXT ////////////

bty_ctx_t* bty_ctx_create(arena_t* a, trace_t* t, int capacity) {
    bty_ctx_t* ctx = (bty_ctx_t*) aalloc(a, sizeof(bty_ctx_t));
    ctx->arena = a;
    ctx->trace = t;
    ctx->capacity = capacity;
    ctx->size = 0;
    ctx->kvps = (bty_ctx_kvp_t*) aalloc(a, sizeof(bty_ctx_kvp_t) * ctx->capacity);
    return ctx;
}

bty_ctx_t* bty_ctx_clone(bty_ctx_t* src) {
    bty_ctx_t* ctx = (bty_ctx_t*) aalloc(src->arena, sizeof(bty_ctx_t));
    ctx->arena = src->arena;
    ctx->trace = src->trace;
    ctx->capacity = src->capacity;
    ctx->size = src->size;
    ctx->kvps = (bty_ctx_kvp_t*) aalloc(src->arena, sizeof(bty_ctx_kvp_t) * ctx->capacity);
    memcpy(ctx->kvps, src->kvps, sizeof(bty_ctx_kvp_t) * src->size);
    return ctx;
}

typedef struct res_t {
    bool found;
    int index;
} res_t;

res_t bty_ctx_binsearch(bty_ctx_kvp_t* kvps, int low, int high, srcref_t name) {
    if ( high >= low ) {
        int mid = low + (high - low) / 2;
        if (strncmp(kvps[mid].name, srcref_ptr(name), srcref_len(name)) == 0)
            return (res_t) { true, mid };
        if (strncmp(kvps[mid].name, srcref_ptr(name), srcref_len(name)) > 0)
            return bty_ctx_binsearch(kvps, low, mid - 1, name);
        return bty_ctx_binsearch(kvps, mid + 1, high, name);
    }
    return (res_t) { false, low };
}

bool bty_ctx_ensure_capacity(bty_ctx_t* ctx, int extra) {
    int required = (ctx->size + extra);
    if( required >= ctx->capacity ) {
        int new_cap = required * 2;
        bty_ctx_kvp_t* kvps = arealloc(ctx->arena, ctx->kvps, new_cap * sizeof(bty_ctx_kvp_t));
        if( kvps == NULL ) {
            return false;
        }
        ctx->capacity = new_cap;
        ctx->kvps = kvps;
    }
    return true;
}

void bty_ctx_make_room_at(bty_ctx_t* ctx, int index) {
    if( (int) ctx->size > index ) {
        memmove(&ctx->kvps[index+1],
            &ctx->kvps[index],
            sizeof(bty_ctx_kvp_t) * (ctx->size - index));
    }
    ctx->size++;
}

bool bty_ctx_insert(bty_ctx_t* ctx, srcref_t name, bty_type_t* type) {

    if( bty_ctx_ensure_capacity(ctx, 1) == false )
        return false;

    res_t res = bty_ctx_binsearch(ctx->kvps, 0, (int) ctx->size - 1, name);
    if( res.found == false ) {
        bty_ctx_make_room_at(ctx, res.index);
        ctx->kvps[res.index] = (bty_ctx_kvp_t) {
            .name = asprintf(ctx->arena, "%.*s",
                srcref_len(name), srcref_ptr(name)),
            .type = type
        };
        return true;
    } 

    return false;
}

bty_type_t* bty_ctx_lookup(bty_ctx_t* ctx, srcref_t name) {
    res_t res = bty_ctx_binsearch(ctx->kvps, 0, (int) ctx->size - 1, name);
    if( res.found ) {
        return ctx->kvps[res.index].type;
    }
    return NULL;
}

void bty_ctx_dump(bty_ctx_t* ctx) {
    printf("ctx_dump (size=%d, capacity=%d)\n",
        ctx->size, ctx->capacity);
    for(int i = 0; i < ctx->size; i++) {
        printf("  \"%s\": %s\n",
            ctx->kvps[i].name,
            sprint_bty_type(ctx->arena,
                ctx->kvps[i].type));
    }
}


/* DESCRIPTION
 *  The checking function calls the inference function when it 
 *  reaches a term whose type can be inferred, comparing the 
 *  inferred type with the type being checked against.
 *  The inference function calls the checking function when it 
 *  encounters a type annotation.
 */

bty_type_t* bty_from_annotation(bty_ctx_t* c, ast_annot_t* a) {
    if(srcref_equals_string(a->name, LANG_TYPENAME_ARRAY)) {
        if( a->childcount != 1 ) {
            trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, a->name);
            trace_msg_append_costr(m, "invalid type annotation");
            return NULL;
        }
        bty_type_t* inner = bty_from_annotation(c, a->children[0]);
        if( inner == NULL ) {
            trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, a->name);
            trace_msg_append_costr(m, "invalid type annotation (list content)");
            return NULL;
        }
        return bty_list(c->arena, inner);
    } else if(srcref_equals_string(a->name, LANG_TYPENAME_STRING)) {
        return bty_list(c->arena, bty_char());
    } else if(srcref_equals_string(a->name, LANG_TYPENAME_VOID)) {
        return bty_void();
    } else if(srcref_equals_string(a->name, LANG_TYPENAME_INT)) {
        return bty_int();
    } else if(srcref_equals_string(a->name, LANG_TYPENAME_FLOAT)) {
        return bty_float();
    } else if(srcref_equals_string(a->name, LANG_TYPENAME_BOOL)) {
        return bty_bool();
    } else if(srcref_equals_string(a->name, LANG_TYPENAME_CHAR)) {
        return bty_char();
    } else {
        trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, a->name);
        trace_msg_append_costr(m, "unhandled type signature");
        return NULL;
    }
}

bty_type_t* bty_synth_var_reference(bty_ctx_t* c, ast_varref_t v) {
    bty_type_t* ty = bty_ctx_lookup(c, v.name);
    if( ty == NULL ) {
        trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, v.name);
        trace_msg_append_costr(m, "reference to undefined variable: ");
        trace_msg_append_srcref(m, v.name);
    }
    return ty; // maybe return some error type instead of NULL?
}

bty_type_t* bty_synth_all_same_or_null(bty_ctx_t* c, ast_node_t** coll, size_t len) {
    if( len == 0 )
        return bty_unknown();
    bty_type_t* ty = bty_synthesize(c, coll[0]);
    for(size_t i = 1; i < len; i++) {
        bty_type_t* tmp = bty_synthesize(c, coll[i]);
        assert( tmp != NULL );
        if( bty_is_equal(ty, tmp) == false )
            return NULL;
    }
    return ty;
}

// note: it might be possible to call bty_check
// here instead of 'custom' checking subtype
bty_type_t* bty_synth_unop(bty_ctx_t* c, ast_node_t* n) {
    ast_unop_t op = n->u.n_unop;
    bty_type_t* ty = bty_synthesize(c, op.inner);
    switch(op.type) {
        case AST_UN_NOT: {
            if( bty_is_subtype(ty, bty_bool()) == false ) {
                trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, n->ref);
                trace_msg_append_fmt(m,
                    "type-error: can't interpret %s as a boolean value",
                    sprint_bty_type(c->arena, ty));
                return bty_unknown(); // todo: perhaps return some kind of error? 
            }
        } break;
        case AST_UN_NEG: {
            if( bty_is_subtype(ty, bty_float()) == false ) {
                trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, n->ref);
                trace_msg_append_fmt(m,
                    "type-error: can't interpret %s as a numeric value",
                    sprint_bty_type(c->arena, ty));
                return bty_unknown(); // todo: perhaps return some kind of error? 
            }
        } break;
        default: {
            assert(false && "not implemented");
        }
    }
    return ty;
}

bool is_allowed_binop_operand_type(ast_binop_type_t op, bty_type_t* ty) {
    switch(op) {
        case AST_BIN_MUL:   return bty_is_float(ty) || bty_is_int(ty);
        case AST_BIN_DIV:   return bty_is_float(ty) || bty_is_int(ty);
        case AST_BIN_MOD:   return bty_is_float(ty) || bty_is_int(ty);
        case AST_BIN_ADD:   return bty_is_float(ty) || bty_is_int(ty);
        case AST_BIN_SUB:   return bty_is_float(ty) || bty_is_int(ty);
        case AST_BIN_LT:    return bty_is_float(ty) || bty_is_int(ty) || bty_is_char(ty);
        case AST_BIN_GT:    return bty_is_float(ty) || bty_is_int(ty) || bty_is_char(ty);
        case AST_BIN_LT_EQ: return bty_is_float(ty) || bty_is_int(ty) || bty_is_char(ty);
        case AST_BIN_GT_EQ: return bty_is_float(ty) || bty_is_int(ty) || bty_is_char(ty);
        case AST_BIN_EQ:    return bty_is_float(ty) || bty_is_int(ty) || bty_is_char(ty);
        case AST_BIN_NEQ:   return bty_is_float(ty) || bty_is_int(ty) || bty_is_char(ty);
        case AST_BIN_XOR:   return bty_is_bool(ty);
        case AST_BIN_OR:    return bty_is_bool(ty);
        case AST_BIN_AND:   return bty_is_bool(ty);
        default:            return false;
    }
}

bty_type_t* bty_synth_binop(bty_ctx_t* c, ast_node_t* n) {
    ast_binop_t op = n->u.n_binop;
    bty_type_t* lty = bty_synthesize(c, op.left);
    bty_type_t* rty = bty_synthesize(c, op.right);

    bty_type_t* ty = NULL;

    if( bty_is_subtype(lty, rty) )
        ty = rty;

    if( bty_is_subtype(rty, lty) )
        ty = lty;
    
    if( ty == NULL ) {
        trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, n->ref);
        trace_msg_append_fmt(m,
            "type-error: binary operator unknown result type\n\tLHS: %s\n\tRHS: %s",
            sprint_bty_type(c->arena, lty),
            sprint_bty_type(c->arena, rty));
        return bty_unknown(); // todo: perhaps return some kind of error? 
    }

    if( is_allowed_binop_operand_type(op.type, ty) == false ) {
        trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, n->ref);
        trace_msg_append_fmt(m,
            "type-error: unsupported operand type: %s",
            sprint_bty_type(c->arena, ty));
        return bty_unknown(); // todo: perhaps return some kind of error? 
    }

    switch(op.type) {
        case AST_BIN_MUL:   return ty;
        case AST_BIN_DIV:   return ty;
        case AST_BIN_MOD:   return ty;
        case AST_BIN_ADD:   return ty;
        case AST_BIN_SUB:   return ty;
        case AST_BIN_LT:    return bty_bool();
        case AST_BIN_GT:    return bty_bool();
        case AST_BIN_LT_EQ: return bty_bool();
        case AST_BIN_GT_EQ: return bty_bool();
        case AST_BIN_EQ:    return bty_bool();
        case AST_BIN_NEQ:   return bty_bool();
        case AST_BIN_XOR:   return bty_bool();
        case AST_BIN_OR:    return bty_bool();
        case AST_BIN_AND:   return bty_bool();
        default:            break;
    }

    trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, n->ref);
    trace_msg_append_fmt(m,
        "type-error: unknown binary operator\n\tLHS: %s\n\tRHS: %s",
        sprint_bty_type(c->arena, lty),
        sprint_bty_type(c->arena, rty));
    return bty_unknown();
}

bty_type_t* bty_synthesize(bty_ctx_t* c, ast_node_t* n) {
    switch(n->type) {
        case AST_VALUE: {
            return bty_from_const_expr(c->arena, n);
        }
        case AST_VAR_REF: {
            return bty_synth_var_reference(c, n->u.n_varref);
        }
        case AST_ARRAY: {
            bty_type_t* ty = bty_synth_all_same_or_null(c,
                n->u.n_array.content,
                n->u.n_array.count);
            if( ty != NULL )
                return bty_list(c->arena, ty); 
            trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, n->ref);
            trace_msg_append_costr(m, "type-error: array contains mixed type elements");
            return bty_unknown(); // todo: perhaps return some kind of error? 
        }
        case AST_TYANNOT: {
            bty_type_t* ty = bty_from_annotation(c, n->u.n_tyannot.type);
            bty_check(c, n->u.n_tyannot.expr, ty); // note: should check add to the context?
            return ty;
        }
        case AST_UNOP: {
            return bty_synth_unop(c, n);
        }
        case AST_BINOP: {
            return bty_synth_binop(c, n);
        }
        case AST_ASSIGN: {
            bty_type_t* ty = bty_synthesize(c, n->u.n_assign.left_var);
            bty_check(c, n->u.n_assign.right_value, ty);
            return ty;
        }
        case AST_RETURN: {
            return bty_synthesize(c, n->u.n_return.result);
        }
        case AST_BREAK: {
            return bty_void();
        }
        default: return bty_unknown();
    }
}



void bty_check(bty_ctx_t* c, ast_node_t* n, bty_type_t* et) {

    // note: not sure about this, adding a var to the context
    // only if it does not already exist
    if( n->type == AST_VAR_REF && bty_ctx_lookup(c, n->u.n_varref.name) == NULL ) {
        bool res = bty_ctx_insert(c, n->u.n_varref.name, et);
        assert(res);
        return;
    }

    bty_type_t* ty = bty_synthesize(c, n);
    if( bty_is_subtype(ty, et) == false ) {
        trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, n->ref);
        trace_msg_append_fmt(m,
            "type-error: expected %s but got %s",
            sprint_bty_type(c->arena, et),
            sprint_bty_type(c->arena, ty));
    }
}




