#include "co_typing.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

char* unop_name(ast_unop_type_t type) {
    // TODO:
    // these should match the source code symbols
    // in the language itself
    switch(type) {
        case AST_UN_NEG: return "-";
        case AST_UN_NOT: return "not";
        default: return "";
    }
}

char* binop_name(ast_binop_type_t type) {
    // TODO:
    // these should match the source code symbols
    // in the language itself
    switch(type) {
        case AST_BIN_ADD:   return "+";
        case AST_BIN_SUB:   return "-";
        case AST_BIN_MUL:   return "*";
        case AST_BIN_DIV:   return "/";
        case AST_BIN_MOD:   return "%";
        case AST_BIN_AND:   return "and";
        case AST_BIN_OR:    return "or";
        case AST_BIN_XOR:   return "xor";
        case AST_BIN_EQ:    return "==";
        case AST_BIN_NEQ:   return "!=";
        case AST_BIN_LT_EQ: return "<=";
        case AST_BIN_GT_EQ: return ">=";
        case AST_BIN_LT:    return "<";
        case AST_BIN_GT:    return ">";
        default: return "";
    }
}

char* value_signature(ast_value_t v) {
    switch(v.type) {
        case AST_VALUE_BOOL:   return "b";
        case AST_VALUE_CHAR:   return "c";
        case AST_VALUE_FLOAT:  return "f";
        case AST_VALUE_INT:    return "i";
        case AST_VALUE_NONE:   return "n";
        default: {
            printf("error: unknown value type\n");
            return "<unk>";
        } break;
    }
}

char* annotation_signature(arena_t* a, ast_annot_t* annot) {
    if( annot == NULL )
        return NULL;
    if(srcref_equals_string(annot->name, LANG_TYPENAME_ARRAY)) {
        if( annot->childcount != 1 )
            return NULL;
        char* inner = annotation_signature(a, annot->children[0]);
        if( inner == NULL )
            return NULL;
        return asprintf(a, "[%s]", inner);
    } else if(srcref_equals_string(annot->name, LANG_TYPENAME_STRING)) {
        return "[c]";
    } else if(srcref_equals_string(annot->name, LANG_TYPENAME_NONE)) {
        return "n";
    } else if(srcref_equals_string(annot->name, LANG_TYPENAME_INT)) {
        return "i";
    } else if(srcref_equals_string(annot->name, LANG_TYPENAME_FLOAT)) {
        return "f";
    } else if(srcref_equals_string(annot->name, LANG_TYPENAME_BOOL)) {
        return "b";
    } else if(srcref_equals_string(annot->name, LANG_TYPENAME_CHAR)) {
        return "c";
    } else {
        return NULL;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * 
 *   int                 -> 'i'
 *   float               -> 'f'
 *   bool                -> 'b'
 *   char                -> 'c'
 *   array of char       -> '[c]'
 *   tuple of int * bool -> '(ib)'              <- TODO
 *   fundecl (in types)  -> '#funname:ifcbi'
 *   funcall (in types)  -> '#funname:ifcbi'
 *   variable            -> '$varname'
 */
char* make_signature(arena_t* a, ast_node_t* n) {
    switch(n->type) {
        case AST_VALUE: {
            return value_signature(n->u.n_value);
        } break;
        case AST_VAR_DECL: {
            srcref_t name = n->u.n_vardecl.name;
            return asprintf(a, "$%.*s",
                srcref_len(name),
                srcref_ptr(name));
        } break;
        case AST_ARRAY: {
            char* inner = NULL;
            for(size_t i = 0; i < n->u.n_array.count; i++) {
                char* tmp = make_signature(a, n->u.n_array.content[i]);
                if( inner == NULL )
                    inner = tmp;
                if( strcmp(tmp, inner) != 0 ) {
                    inner = NULL;
                    break;
                }
            }
            if( inner == NULL )
                inner = "*";
            return asprintf(a, "[%s]", inner);
        } break;
        case AST_BLOCK: {
            char* sign = "";
            for(size_t i = 0; i < n->u.n_block.count; i++) {
                ast_node_t* inner = n->u.n_block.content[i];
                sign = asprintf(a, "%s%s", sign, make_signature(a, inner));
            }
            return sign;
        } break;
        case AST_FUN_CALL: {
            srcref_t name = n->u.n_funcall.name;
            return asprintf(a, "#%.*s:%s",
                srcref_len(name),
                srcref_ptr(name),
                make_signature(a, n->u.n_funcall.args));
        } break;
        case AST_FUN_DECL: {
            return make_signature(a, n->u.n_fundecl.funsign);
        } break;
        case AST_FUN_SIGN: {
            srcref_t name = n->u.n_funsign.name;
            return asprintf(a, "#%.*s:%s",
                srcref_len(name),
                srcref_ptr(name),
                make_signature(a, n->u.n_funsign.argspec));
        } break;
        case AST_BINOP: {
            return asprintf(a, "#%s:%s%s",
                binop_name(n->u.n_binop.type),
                make_signature(a, n->u.n_binop.left),
                make_signature(a, n->u.n_binop.right));
        } break;
        case AST_UNOP: {
            return asprintf(a, "#%s:%s",
                unop_name(n->u.n_unop.type),
                make_signature(a, n->u.n_unop.inner));
        } break;
        default: {
            return "";
        } break;
    }
}

ctx_t* ctx_create(arena_t* a, size_t capacity) {
    ctx_t* ctx = (ctx_t*) aalloc(a, sizeof(ctx_t));
    ctx->arena = a;
    ctx->capacity = capacity;
    ctx->size = 0;
    ctx->kvps = (ctx_kvp_t*) aalloc(a, sizeof(ctx_kvp_t) * ctx->capacity);
    return ctx;
}

ctx_t* ctx_clone(ctx_t* src) {
    ctx_t* ctx = (ctx_t*) aalloc(src->arena, sizeof(ctx_t));
    ctx->arena = src->arena;
    ctx->capacity = src->capacity;
    ctx->size = src->size;
    ctx->kvps = (ctx_kvp_t*) aalloc(src->arena, sizeof(ctx_kvp_t) * ctx->capacity);
    memcpy(ctx->kvps, src->kvps, sizeof(ctx_kvp_t) * src->size);
    return ctx;
}

typedef struct bsres_t {
    bool found;
    int index;
} bsres_t;

bsres_t binsearch(ctx_kvp_t* kvps, int low, int high, char* key) {
    if ( high >= low ) {
        int mid = low + (high - low) / 2;
        if (strcmp(kvps[mid].key, key) == 0)
            return (bsres_t) { true, mid };
        if (strcmp(kvps[mid].key, key) > 0)
            return binsearch(kvps, low, mid - 1, key);
        return binsearch(kvps, mid + 1, high, key);
    }
    return (bsres_t) { false, low };
}

ctx_res_t ctx_ensure_capacity(ctx_t* ctx, size_t extra) {
    size_t required = (ctx->size + extra);
    if( required >= ctx->capacity ) {
        size_t new_cap = required * 2;
        ctx_kvp_t* kvps = arealloc(ctx->arena, ctx->kvps, new_cap * sizeof(ctx_kvp_t));
        if( kvps == NULL ) {
            return CTX_RES_OUT_OF_MEMORY;
        }
        ctx->capacity = new_cap;
        ctx->kvps = kvps;
    }
    return CTX_RES_OK;
}

void ctx_make_room_at(ctx_t* ctx, int index) {
    if( (int) ctx->size > index ) {
        memmove(&ctx->kvps[index+1],
            &ctx->kvps[index],
            sizeof(ctx_kvp_t) * (ctx->size - index));
    }
    ctx->size++;
}

bool ctx_idenity_mapping(char* key) {
    return key[0] == 'i'
        || key[0] == 'f'
        || key[0] == 'c'
        || key[0] == 'b'
        || key[0] == 'n'
        || key[0] == '('
        || key[0] == '[';
}

ctx_res_t ctx_insert(ctx_t* ctx, char* key, char* val) {

    assert(ctx_idenity_mapping(key) == false);

    ctx_res_t alloc_res = ctx_ensure_capacity(ctx, 1);
    if( alloc_res != CTX_RES_OK )
        return alloc_res;

    bsres_t res = binsearch(ctx->kvps, 0, (int) ctx->size - 1, key);
    if( res.found == false ) {
        ctx_make_room_at(ctx, res.index);
        ctx->kvps[res.index] = (ctx_kvp_t) {
            .key = key,
            .val = val
        };
        return CTX_RES_OK;
    } 

    return CTX_RES_FORBIDDEN;
}

char* ctx_lookup(ctx_t* ctx, char* key) {
    bsres_t res = binsearch(ctx->kvps, 0, (int) ctx->size - 1, key);
    if( res.found ) {
        return ctx->kvps[res.index].val;
    }
    return NULL;
}

char* ctx_infer(ctx_t* ctx, char* key) {
    char* last = key;
    do {
        last = key;
        key = ctx_lookup(ctx, key);
        if( key == NULL )
            break;
    } while(strcmp(last, key) != 0);
    return last;
}

void ctx_dump(ctx_t* ctx) {
    printf("ctx_dump (size=%lu, capacity=%lu)\n",
        ctx->size, ctx->capacity);
    for(size_t i = 0; i < ctx->size; i++) {
        printf("  \"%s\": \"%s\"\n", 
            ctx->kvps[i].key,
            ctx->kvps[i].val);
    }
}

bool is_valid_value(ast_value_t v) {
    ast_value_type_t type = v.type;
    return type == AST_VALUE_BOOL
        || type == AST_VALUE_CHAR
        || type == AST_VALUE_FLOAT
        || type == AST_VALUE_INT
        || type == AST_VALUE_NONE;
}

#define REFSTACK_CAP 32

typedef struct typing_state_t {
    trace_t*  trace;
    int       reftop;
    srcref_t  refstack[REFSTACK_CAP];
} typing_state_t;

srcref_t state_get_ref(typing_state_t* state) {
    int reftop = state->reftop;
    if( reftop < 0 )
        return trace_no_ref();
    if( reftop >= REFSTACK_CAP )
        return state->refstack[REFSTACK_CAP-1];
    return state->refstack[state->reftop];
}

void state_push_ref(typing_state_t* state, srcref_t ref) {
    state->reftop ++;
    if( state->reftop < REFSTACK_CAP ) {
        state->refstack[state->reftop] = ref;
    }
}

void state_pop_ref(typing_state_t* state) {
    state->reftop --;
}


char* infer(typing_state_t* state, ast_node_t* node, ctx_t* ctx) {
    switch(node->type) {
        case AST_VALUE: {
            if( is_valid_value(node->u.n_value) == false ) {
                trace_msg_t* msg = trace_create_message(state->trace,
                    TM_ERROR, state_get_ref(state));
                trace_msg_append_costr(msg, "unknown value type");
            }
            return value_signature(node->u.n_value);
        } break;
        case AST_VAR_DECL: {
            srcref_t name = node->u.n_vardecl.name;
            ast_annot_t* annot = node->u.n_vardecl.type;
            char* annot_sign = annotation_signature(ctx->arena, annot);
            char* var_sign = asprintf(ctx->arena, "$%.*s",
                srcref_len(name),
                srcref_ptr(name));
            if( annot_sign != NULL ) {
                ctx_insert(ctx, var_sign, annot_sign);
            } else {
                trace_msg_t* msg = trace_create_message(state->trace,
                    TM_ERROR, annot->name);
                trace_msg_append_costr(msg, "invalid type annotation");
            }
        } break;
        case AST_ARRAY: {
            char* inner = NULL;
            for(size_t i = 0; i < node->u.n_array.count; i++) {
                char* tmp = infer(state, node->u.n_array.content[i], ctx);
                if( inner == NULL )
                    inner = tmp;
                if( strcmp(tmp, inner) != 0 ) {
                    inner = NULL;
                    break;
                }
            }
            if( inner == NULL ) {
                trace_msg_t* msg = trace_create_message(state->trace,
                    TM_ERROR, state_get_ref(state));
                trace_msg_append_costr(msg, "arrays must contain items of the same type");
                inner = "*";
            }
            return asprintf(ctx->arena, "[%s]", inner);
        } break;
        case AST_BLOCK: {
            // TODO: if the block is a tuple whe should do this
            // BUT if it is a function body perhaps we need to
            // handle that differently?
            // Since if in a function the type of returned var
            // should be returned.
            char* sign = "";
            for(size_t i = 0; i < node->u.n_block.count; i++) {
                ast_node_t* inner = node->u.n_block.content[i];
                sign = asprintf(ctx->arena, "%s%s", sign, infer(state, inner, ctx));
            }
            return sign;
        } break;
        case AST_FUN_CALL: {
            // TODO: lookup in context and compare with call sign
            srcref_t name = node->u.n_funcall.name;
            return asprintf(ctx->arena, "#%.*s:%s",
                srcref_len(name),
                srcref_ptr(name),
                infer(state, node->u.n_funcall.args, ctx));
        } break;
        case AST_FUN_DECL: {
            // TODO:
            // run infer to add to context
            // run through the body
            // compare returned type with annotation 
            return infer(state, node->u.n_fundecl.funsign, ctx);
        } break;
        case AST_FUN_SIGN: {
            // TODO: add to context
            srcref_t name = node->u.n_funsign.name;
            return asprintf(ctx->arena, "#%.*s:%s",
                srcref_len(name),
                srcref_ptr(name),
                infer(state, node->u.n_funsign.argspec, ctx));
        } break;
        case AST_BINOP: {
            // TODO: lookup and compare with global defs
            return asprintf(ctx->arena, "#%s:%s%s",
                binop_name(node->u.n_binop.type),
                infer(state, node->u.n_binop.left, ctx),
                infer(state, node->u.n_binop.right, ctx));
        } break;
        case AST_UNOP: {
            // TODO: lookup and compare with global defs
            return asprintf(ctx->arena, "#%s:%s",
                unop_name(node->u.n_unop.type),
                infer(state, node->u.n_unop.inner, ctx));
        } break;
        case AST_IF_CHAIN: {

        } break;
        case AST_FOREACH: {

        } break;
        case AST_ASSIGN: {

        } break;
        case AST_VAR_REF: {

        } break;
        case AST_RETURN: {

        } break;
        case AST_BREAK: {

        } break;
        default: {

        } break;
    }

    return "";
}

void check(arena_t* arena, trace_t* trace, ast_node_t* root) {
    typing_state_t state = {
        .trace = trace,
        .reftop = 0,
        .refstack = { {0} }
    };
    ctx_t* ctx = ctx_create(arena, 16);
    infer(&state, root, ctx);
}