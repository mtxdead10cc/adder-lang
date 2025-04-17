#include "co_bty.h"
#include <assert.h>
#include <string.h>
#include "co_types.h"
#include "co_utils.h"
#include "co_trace.h"
#include "co_ast.h"
#include <sh_log.h>
#include <sh_utils.h>

static bty_type_t bty_base_types[] = {
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
        return bty_list(a, bty_error(a, BTY_ERR_TYPECHECK));

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

bty_type_t* bty_func(arena_t* a) {
    bty_type_t* fun = (bty_type_t*) aalloc(a, sizeof(bty_type_t));
    fun->tag = BTY_FUNC;
    fun->u.fun.argc = 0;
    fun->u.fun.args = NULL;
    fun->u.fun.ret = bty_void();
    fun->u.fun.exported = false;
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

void bty_func_set_return_type(bty_type_t* fun, bty_type_t* ret) {
    assert(fun->tag == BTY_FUNC);
    fun->u.fun.ret = ret;
}

void bty_func_set_exported(bty_type_t* fun) {
    assert(fun->tag == BTY_FUNC);
    fun->u.fun.exported = true;
}

bty_type_t* bty_list(arena_t* a, bty_type_t* content_type) {
    bty_type_t* lst = (bty_type_t*) aalloc(a, sizeof(bty_type_t));
    lst->tag = BTY_LIST;
    lst->u.con = content_type;
    return lst;
}

bty_type_t* bty_error(arena_t* a, uint32_t error_code) {
    bty_type_t* ty = (bty_type_t*) aalloc(a, sizeof(bty_type_t));
    ty->tag = BTY_ERROR;
    ty->u.err.erc = error_code;
    return ty;
}

bty_type_t* bty_return(arena_t* a, bty_type_t* type) {
    bty_type_t* ty = (bty_type_t*) aalloc(a, sizeof(bty_type_t));
    ty->tag = BTY_RETURN;
    ty->u.con = type;
    return ty;
}

bty_type_t* bty_sometimes(arena_t* a, bty_type_t* type) {
    bty_type_t* ty = (bty_type_t*) aalloc(a, sizeof(bty_type_t));
    ty->tag = BTY_SOMETIMES;
    ty->u.con = type;
    return ty;
}

bty_type_t* bty_always(arena_t* a, bty_type_t* type) {
    bty_type_t* ty = (bty_type_t*) aalloc(a, sizeof(bty_type_t));
    ty->tag = BTY_ALWAYS;
    ty->u.con = type;
    return ty;
}


bool bty_is_error(bty_type_t* ty) {
    return ty->tag == BTY_ERROR;
}

bool bty_is_void(bty_type_t* ty) {
    return ty->tag == BTY_VOID;
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

bool bty_is_return(bty_type_t* ty) {
    return ty->tag == BTY_RETURN;
}

bool bty_is_sometimes(bty_type_t* ty) {
    return ty->tag == BTY_SOMETIMES;
}

bool bty_is_always(bty_type_t* ty) {
    return ty->tag == BTY_ALWAYS;
}

char* sprint_bty_type(arena_t* a, bty_type_t* ty) {
    switch(ty->tag) {
        case BTY_ERROR: {
            return asprint(a, "error (0x%X)", ty->u.err.erc);
        } break;
        case BTY_VOID: {
            return asprint(a, "void");
        } break;
        case BTY_FLOAT: {
            return asprint(a, "float");
        } break;
        case BTY_INT: {
            return asprint(a, "int");
        } break;
        case BTY_CHAR: {
            return asprint(a, "char");
        } break;
        case BTY_BOOL: {
            return asprint(a, "bool");
        } break;
        case BTY_SOMETIMES: {
            return asprint(a,
                "%s or nothing", 
                sprint_bty_type(a, 
                    ty->u.con));
        } break;
        case BTY_ALWAYS: {
            return asprint(a,
                "%s", 
                sprint_bty_type(a, 
                    ty->u.con));
        } break;
        case BTY_RETURN: {
            return asprint(a,
                "return type %s", 
                sprint_bty_type(a, 
                    ty->u.con));
        } break;
        case BTY_LIST: {
            return asprint(a,
                "list of %s", 
                sprint_bty_type(a, 
                    ty->u.con));
        } break;
        case BTY_FUNC: {
            char* text = "";
            for(int i = 0; i < ty->u.fun.argc; i++) {
                if( i > 0 )
                    text = asprint(a, "%s, ", text);
                text = asprint(a, 
                    "%s%s", text,
                    sprint_bty_type(a,
                        ty->u.fun.args[i]));
            }
            return asprint(a, 
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
    return bty_is_subtype(child->u.con, parent->u.con);
}

bool bty_handle_error_subtype(bty_type_t* child, bty_type_t* parent) {
    assert(child->tag == BTY_ERROR);
    (void)(parent);
    (void)(child);
    return false;
}

bool bty_is_return_subtype(bty_type_t* child, bty_type_t* parent) {
    assert(child->tag == BTY_RETURN);
    if( parent->tag != BTY_RETURN )
        return false;
    return bty_is_subtype(child->u.con, parent->u.con);
}

bool bty_is_always_subtype(bty_type_t* child, bty_type_t* parent) {
    assert(child->tag == BTY_ALWAYS);
    if( parent->tag != BTY_ALWAYS )
        return false;
    return bty_is_subtype(child->u.con, parent->u.con);
}


bool bty_is_subtype(bty_type_t* child, bty_type_t* parent) {
    switch(child->tag) {
        case BTY_FLOAT:     return parent->tag == BTY_FLOAT;
        case BTY_INT:       return parent->tag == BTY_INT
                                || parent->tag == BTY_FLOAT;
        case BTY_CHAR:      return parent->tag == BTY_CHAR
                                || parent->tag == BTY_FLOAT
                                || parent->tag == BTY_INT;
        case BTY_BOOL:      return parent->tag == BTY_BOOL;
        case BTY_VOID:      return parent->tag == BTY_VOID;
        case BTY_ALWAYS:    return bty_is_always_subtype(child, parent);
        case BTY_RETURN:    return bty_is_return_subtype(child, parent);
        case BTY_LIST:      return bty_is_list_subtype(child, parent);
        case BTY_FUNC:      return bty_is_func_subtype(child, parent);
        case BTY_ERROR:     return bty_handle_error_subtype(child, parent);
        default: {
            sh_log_error("bty_is_subtype: unknown type\n");
            return false;
        }
    }
}

bool bty_is_equal(bty_type_t* a, bty_type_t* b) {
    if( a->tag != b->tag )
        return false;
    if( a->tag == BTY_LIST )
        return bty_is_equal(a->u.con, b->u.con);
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

int srcrefcmp(char* actual, srcref_t ref) {
    int actual_len = strlen(actual);
    int ref_len = srcref_len(ref);
    if( actual_len == ref_len )
        return strncmp(actual, srcref_ptr(ref), ref_len);
    return ref_len - actual_len;
}

res_t bty_ctx_binsearch(bty_ctx_kvp_t* kvps, int low, int high, srcref_t name) {
    if ( high >= low ) {
        int mid = low + (high - low) / 2;
        if (srcrefcmp(kvps[mid].name, name) == 0)
            return (res_t) { true, mid };
        if (srcrefcmp(kvps[mid].name, name) > 0)
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
            .name = asprint(ctx->arena, "%.*s",
                (int) srcref_len(name), srcref_ptr(name)),
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

void bty_ctx_dump(cstr_t str, bty_ctx_t* ctx) {
    cstr_append_fmt(str, "ctx_dump (size=%d, capacity=%d)\n",
        ctx->size, ctx->capacity);
    for(int i = 0; i < ctx->size; i++) {
        cstr_append_fmt(str, "  \"%s\": %s\n",
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

bty_type_t* bty_from_ast_annot(arena_t* a, trace_t* t, ast_annot_t* asta) {
    if(srcref_equals_string(asta->name, LANG_TYPENAME_ARRAY)) {
        if( asta->childcount != 1 ) {
            trace_msg_t* m = trace_create_message(t, TM_ERROR, asta->name);
            trace_msg_append_costr(m, "invalid type annotation");
            return NULL;
        }
        bty_type_t* inner = bty_from_ast_annot(a, t, asta->children[0]);
        if( inner == NULL ) {
            trace_msg_t* m = trace_create_message(t, TM_ERROR, asta->name);
            trace_msg_append_costr(m, "invalid type annotation (list content)");
            return NULL;
        }
        return bty_list(a, inner);
    } else if(srcref_equals_string(asta->name, LANG_TYPENAME_STRING)) {
        return bty_list(a, bty_char());
    } else if(srcref_equals_string(asta->name, LANG_TYPENAME_VOID)) {
        return bty_void();
    } else if(srcref_equals_string(asta->name, LANG_TYPENAME_INT)) {
        return bty_int();
    } else if(srcref_equals_string(asta->name, LANG_TYPENAME_FLOAT)) {
        return bty_float();
    } else if(srcref_equals_string(asta->name, LANG_TYPENAME_BOOL)) {
        return bty_bool();
    } else if(srcref_equals_string(asta->name, LANG_TYPENAME_CHAR)) {
        return bty_char();
    } else {
        trace_msg_t* m = trace_create_message(t, TM_ERROR, asta->name);
        trace_msg_append_costr(m, "unhandled type signature");
        return NULL;
    }
}

bty_type_t* bty_extract_type(arena_t* a, trace_t* t, ast_node_t* n) {
    if( n->type != AST_TYANNOT )
        return NULL;
    ast_node_t* expr = n->u.n_tyannot.expr;
    ast_annot_t* annot = n->u.n_tyannot.type;
    switch(expr->type) {
        case AST_VAR_REF: return bty_from_ast_annot(a, t, annot);
        case AST_FUN_DECL: {

            bty_type_t* ft = bty_func(a);

            bty_func_set_return_type(ft,
                bty_from_ast_annot(a, t, annot));

            assert(expr->u.n_fundecl.argspec->type == AST_ARGLIST);

            ast_arglist_t args = expr->u.n_fundecl.argspec->u.n_args;
            for(size_t i = 0; i < args.count; i++) {
                bty_type_t* at = bty_extract_type(a,t,args.content[i]);
                if( at == NULL )
                    return NULL;
                bty_func_add_arg(a, ft, at);
            }

            if( expr->u.n_fundecl.exported ) {
                bty_func_set_exported(ft);
            }

            return ft;
        }
        case AST_FUN_EXDECL: {

            bty_type_t* ft = bty_func(a);

            bty_func_set_return_type(ft,
                bty_from_ast_annot(a, t, annot));

            assert(expr->u.n_funexdecl.argspec->type == AST_ARGLIST);

            ast_arglist_t args = expr->u.n_funexdecl.argspec->u.n_args;
            for(size_t i = 0; i < args.count; i++) {
                bty_type_t* at = bty_extract_type(a,t,args.content[i]);
                if( at == NULL )
                    return NULL;
                bty_func_add_arg(a, ft, at);
            }

            return ft;
        }
        default: return NULL;
    }
}

srcref_t bty_extract_name(ast_node_t* n) {
    if( n->type == AST_VAR_REF )
        return n->u.n_varref.name;
    if( n->type == AST_FUN_DECL )
        return n->u.n_fundecl.name;
    if( n->type == AST_FUN_EXDECL )
        return n->u.n_funexdecl.name;
    if( n->type == AST_TYANNOT )
        return bty_extract_name(n->u.n_tyannot.expr);
    return (srcref_t) { 0 };
}

bty_type_t* bty_synth_var_reference(bty_ctx_t* c, ast_varref_t v) {
    bty_type_t* ty = bty_ctx_lookup(c, v.name);
    if( ty == NULL ) {
        trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, v.name);
        trace_msg_append_costr(m, "reference to undefined variable: ");
        trace_msg_append_srcref(m, v.name);
        return bty_error(c->arena, BTY_ERR_TYPECHECK);
    }
    return ty;
}


bty_type_t* bty_synth_all_same_or_null(bty_ctx_t* c, ast_node_t** coll, size_t len) {
    if( len == 0 )
        return bty_error(c->arena, BTY_ERR_TYPECHECK);
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
                return bty_error(c->arena, BTY_ERR_TYPECHECK);
            }
        } break;
        case AST_UN_NEG: {
            if( bty_is_subtype(ty, bty_float()) == false ) {
                trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, n->ref);
                trace_msg_append_fmt(m,
                    "type-error: can't interpret %s as a numeric value",
                    sprint_bty_type(c->arena, ty));
                return bty_error(c->arena, BTY_ERR_TYPECHECK);
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
        return bty_error(c->arena, BTY_ERR_TYPECHECK);
    }

    if( is_allowed_binop_operand_type(op.type, ty) == false ) {
        trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, n->ref);
        trace_msg_append_fmt(m,
            "type-error: unsupported operand type: %s",
            sprint_bty_type(c->arena, ty));
        return bty_error(c->arena, BTY_ERR_TYPECHECK);
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
    return bty_error(c->arena, BTY_ERR_TYPECHECK);
}

bty_type_t* bty_synth_funcall(bty_ctx_t* c, ast_funcall_t fc) {
    if( fc.args->type != AST_ARGLIST ) {
        trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, fc.name);
        trace_msg_append_costr(m, "invalid argument(s)");
        return bty_error(c->arena, BTY_ERR_INTERNAL);
    }
    bty_type_t* fnty = bty_ctx_lookup(c, fc.name);
    if( fnty == NULL ) {
        trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, fc.name);
        trace_msg_append_fmt(m, "function '%.*s' could not be found",
            srcref_len(fc.name),
            srcref_ptr(fc.name));
        return bty_error(c->arena, BTY_ERR_TYPECHECK);
    }
    assert(fnty->tag == BTY_FUNC);
    ast_arglist_t al = fc.args->u.n_args;
    if( al.count != (size_t) fnty->u.fun.argc ) {
        trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, fc.name);
        trace_msg_append_costr(m, "argument count mismatch");
        return bty_error(c->arena, BTY_ERR_TYPECHECK);
    }
    for(int i = 0; i < fnty->u.fun.argc; i++) {
        bty_check(c, al.content[i], fnty->u.fun.args[i]);
    }
    return fnty->u.fun.ret;
}

static inline bool is_valid_if_link(ast_node_t* n) {
    if( n == NULL )
        return false;
    return n->type == AST_IF_CHAIN;
}

static inline bool is_valid_else_block(ast_node_t* n) {
    if( n == NULL )
        return false;
    if( n->type != AST_BLOCK )
        return false;
    return n->u.n_block.count > 0;
}

typedef struct agg_t {
    trace_t* trace;
    arena_t* arena;
    bty_type_t* type;
    int cnt_always;
    int cnt_sometimes;
    int cnt_never;
} agg_t;

bty_type_t* agg_select_container_type(agg_t* agg, bty_type_t* t, srcref_t ref) {
    assert(t != NULL);
    if( agg->type == NULL )
        return t;
    if( bty_is_subtype(t, agg->type) )
        return agg->type;
    if( bty_is_subtype(agg->type, t) )
        return t;
    trace_msg_t* m = trace_create_message(agg->trace, TM_ERROR, ref);
    trace_msg_append_fmt(m,
        "type-error: expected return type %s but got %s",
        sprint_bty_type(agg->arena, agg->type),
        sprint_bty_type(agg->arena, t));
    return agg->type;
}

bool bty_synth_aggregate(agg_t* agg, bty_ctx_t* c, ast_node_t* n, bool clone_ctx) {
    if( clone_ctx )
        c = bty_ctx_clone(c);
    bty_type_t* t = bty_synthesize(c, n);
    switch(t->tag) {
        case BTY_ALWAYS: {
            agg->cnt_always ++;
            agg->type = agg_select_container_type(agg, t->u.con, n->ref);
            return true;
        } break;
        case BTY_SOMETIMES: {
            agg->cnt_sometimes ++;
            agg->type = agg_select_container_type(agg, t->u.con, n->ref);
            return false;
        } break;
        case BTY_RETURN: {
            agg->cnt_always ++;
            agg->type = agg_select_container_type(agg, t, n->ref);
            return true;
        } break;
        case BTY_VOID: {
            agg->cnt_never ++;
            return false;
        } break;
        default: {
            agg->cnt_never ++;
            return false;
        } break;
    }
}

bty_type_t* bty_synth_if(bty_ctx_t* c, ast_node_t* n) {

    agg_t agg = {
        .cnt_always = 0,
        .cnt_sometimes = 0,
        .cnt_never = 0,
        .type = NULL,
        .trace = c->trace,
        .arena = c->arena
    };

    while ( is_valid_if_link(n) ) {
        ast_if_t ifs = n->u.n_if;
        bty_check(c, ifs.cond, bty_bool());
        bty_synth_aggregate(&agg, c, ifs.iftrue, true);
        n = ifs.next;
    }

    if( is_valid_else_block(n) ) {
        // all bodies (including else) return always(t)                             -> always(t)
        // at least one body (including else) returns sometimes(t) or always(t)     -> sometimes(t)
        // otherwise                                                                -> never
        bty_synth_aggregate(&agg, c, n, true);
        int total = agg.cnt_always
            + agg.cnt_never 
            + agg.cnt_sometimes;
        if( total == agg.cnt_always )
            return bty_always(c->arena, agg.type);
    }

    // no else branch:
    //    at least one body returns sometimes(t) or always(t)   -> sometimes(t)
    //    all bodies return never                               -> never
    if( (agg.cnt_always + agg.cnt_sometimes) > 0 )
        return bty_sometimes(c->arena, agg.type);

    return bty_void();
}

bty_type_t* bty_synth_body(bty_ctx_t* c, ast_node_t* n) {

    // all inferred are never                                   -> never
    // all inferred are never with at least one sometimes(t)    -> sometimes(t)
    // at least one inferred is always(t) (and is last)         -> always(t)
    
    assert(n->type == AST_BLOCK);

    agg_t agg = {
        .cnt_always = 0,
        .cnt_sometimes = 0,
        .cnt_never = 0,
        .type = NULL,
        .trace = c->trace,
        .arena = c->arena
    };

    ast_block_t blk = n->u.n_block;
    int count = blk.count > 0 ? (int) blk.count : 0;
    for(int i = 0; i < count; i++) {
        bool always_returns = bty_synth_aggregate(&agg, c, blk.content[i], false);
        if( always_returns ) {
            if( i < (count - 1) ) {
                sh_log_error("todo: error - unreachable code (following return statement).\n");
            }
            return bty_always(c->arena, agg.type);
        }
    }

    if( agg.cnt_sometimes == 0 )
        return bty_void();

    return bty_sometimes(c->arena, agg.type);
}

bty_type_t* bty_synth_foreach(bty_ctx_t* c, ast_node_t* n) {
    assert(n->type == AST_FOREACH);
    ast_foreach_t fe = n->u.n_foreach;
    bty_ctx_t* loop_ctx = bty_ctx_clone(c);
    bty_type_t* vt = bty_synthesize(loop_ctx, fe.vardecl);
    bty_check(c, fe.collection, bty_list(c->arena, vt));
    return bty_synthesize(loop_ctx, fe.during);
}

bty_type_t* bty_synthesize(bty_ctx_t* c, ast_node_t* n) {
    switch(n->type) {
        case AST_ARRAY: {
            bty_type_t* ty = bty_synth_all_same_or_null(c,
                n->u.n_array.content,
                n->u.n_array.count);
            if( ty != NULL )
                return bty_list(c->arena, ty); 
            trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, n->ref);
            trace_msg_append_costr(m, "type-error: array contains mixed type elements");
            return bty_error(c->arena, BTY_ERR_TYPECHECK);
        }
        case AST_TYANNOT: {
            bty_type_t* ty = bty_extract_type(c->arena, c->trace, n);
            srcref_t name = bty_extract_name(n);
            if( ty == NULL || srcref_is_valid(name) == false ) {
                trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, n->ref);
                trace_msg_append_costr(m, "type-error: invalid type annotation(s)");
                return bty_error(c->arena, BTY_ERR_TYPECHECK);
            }
            bool insert_ok = bty_ctx_insert(c, name, ty);
            if( insert_ok == false ) {
                trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, n->ref);
                trace_msg_append_fmt(m, "type-error: the name '%.*s' is already in use in this context",
                    srcref_len(name),
                    srcref_ptr(name));
            }
            bty_check(c, n->u.n_tyannot.expr, ty);
            return ty;
        }
        case AST_ASSIGN: {
            bty_type_t* ty = bty_synthesize(c, n->u.n_assign.left_var);
            bty_check(c, n->u.n_assign.right_value, ty);
            return bty_void();
        }
        case AST_RETURN: {
            bty_type_t* type = bty_synthesize(c, n->u.n_return.result);
            if( type->tag == BTY_VOID )
                return bty_void();
            return bty_return(c->arena, type);
        } break;
        case AST_VALUE:     return bty_from_const_expr(c->arena, n);
        case AST_VAR_REF:   return bty_synth_var_reference(c, n->u.n_varref);
        case AST_UNOP:      return bty_synth_unop(c, n);
        case AST_BINOP:     return bty_synth_binop(c, n);
        case AST_BREAK:     return bty_void();
        case AST_FUN_CALL:  return bty_synth_funcall(c, n->u.n_funcall);
        case AST_IF_CHAIN:  return bty_synth_if(c, n);
        case AST_BLOCK:     return bty_synth_body(c, n);
        case AST_FOREACH:   return bty_synth_foreach(c, n);
        default:            return bty_error(c->arena, BTY_ERR_INTERNAL);
    }
}


void bty_check_fundecl(bty_ctx_t* c, ast_node_t* n, bty_type_t* et) {

    assert(et->tag == BTY_FUNC);

    if( n->type == AST_FUN_EXDECL ) {
        return;
    }
    
    if ( n->type != AST_FUN_DECL ) {
        trace_msg_t* m = trace_create_message(c->trace,
            TM_INTERNAL_ERROR, 
            ast_extract_srcref(n));
        trace_msg_append_fmt(m,
            "internal-error: fundecl unexpected node type %s",
            ast_node_type_as_string(n->type));
        return;
    }

    if( srcref_equals_string(n->u.n_fundecl.name, "main") ) {
        if( n->u.n_fundecl.exported == false ) {
            trace_msg_t* m = trace_create_message(c->trace,
            TM_ERROR, 
            ast_extract_srcref(n));
            trace_msg_append_costr(m,
                "type-error: the main function ast"
                " node should have been marked as"
                " exported, but it was not.");
            return;
        }
    }

    bty_fun_t ft = et->u.fun;
    ast_fundecl_t fd = n->u.n_fundecl;
    assert( fd.argspec->type == AST_ARGLIST );
    ast_arglist_t al = fd.argspec->u.n_args;
    if( al.count != (size_t) ft.argc ) {
        trace_msg_t* m = trace_create_message(c->trace,
            TM_ERROR, 
            ast_extract_srcref(n));
        trace_msg_append_fmt(m,
            "type-error: expected %d args, but got %lu",
            ft.argc,
            al.count);
        return;
    }

    bty_ctx_t* body_ctx = bty_ctx_clone(c);

    size_t count = al.count;
    for(size_t i = 0; i < count; i++) {
        bty_check(body_ctx, al.content[i], ft.args[i]);
    }

    if( ft.ret->tag == BTY_VOID ) {
        bty_check(body_ctx, fd.body, bty_void());    
    } else {
        bty_check(body_ctx, fd.body,
            bty_always(c->arena, 
                bty_return(c->arena, ft.ret)));
    }
}


void bty_check(bty_ctx_t* c, ast_node_t* n, bty_type_t* et) {

    if( et->tag == BTY_FUNC ) {
        bty_check_fundecl(c, n, et);
        return;
    }

    bty_type_t* ty = bty_synthesize(c, n);
    if( bty_is_subtype(ty, et) == false ) {
        trace_msg_t* m = trace_create_message(c->trace, TM_ERROR, ast_extract_srcref(n));
        trace_msg_append_fmt(m,
            "type-error: expected %s but got %s\n",
            sprint_bty_type(c->arena, et),
            sprint_bty_type(c->arena, ty));
        return;
    }
}

bool is_toplevel_definition(ast_node_t* n) {
    if(n->type == AST_TYANNOT) {
        ast_node_type_t t = n->u.n_tyannot.expr->type;
        return t == AST_FUN_DECL || t == AST_FUN_EXDECL;
    }
    return false;
}

int bty_count_entrypoints(bty_ctx_t* ctx) {
    int export_count = 0;
    for(int i = 0; i < ctx->size; i++) {
        bty_type_t* ty = ctx->kvps[i].type;
        if( ty->tag != BTY_FUNC )
            continue;
        if( ty->u.fun.exported )
            export_count ++;
    }
    return export_count;
}

bool bty_typecheck(bty_ctx_t* ctx, ast_node_t* program) {

    assert(program->type == AST_BLOCK);
    size_t count = program->u.n_block.count;

    for(size_t i = 0; i < count; i++) {
        ast_node_t* def = program->u.n_block.content[i];
        if( is_toplevel_definition(def) == false ) {
            trace_msg_t* m = trace_create_message(ctx->trace, TM_ERROR, def->ref);
            trace_msg_append_fmt(m,
                "type-error: invalid top-level definition:\n ****** \n > %.*s\n ****** \n",
                srcref_len(def->ref),
                srcref_ptr(def->ref));
        } else {
            bty_type_t* dt = bty_synthesize(ctx, def);
            if( dt->tag == BTY_ERROR ) {
                trace_msg_t* m = trace_create_message(ctx->trace, TM_ERROR, def->ref);
                trace_msg_append_fmt(m,
                    "type-error: %s\n",
                     (dt->u.err.erc == BTY_ERR_INTERNAL
                        ? "internal error"
                        : "typecheck failed"));
            }
        }
    }

    if( trace_get_error_count(ctx->trace) > 0 )
        return false;

    int num_exported = bty_count_entrypoints(ctx);

    if( num_exported == 0 ) {
        trace_msg_t* m = trace_create_message(ctx->trace, TM_ERROR, program->ref);
        trace_msg_append_fmt(m,
            "type-error: the program has no entry points"
            "\n  fix this by"
            "\n  - adding a main function"
            "\n  - exporting a function (optional)");
        return false;
    }

    return true;
}




