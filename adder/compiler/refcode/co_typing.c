#include "co_typing.h"
#include "co_ast.h"
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
        return asprint(a, "[%s]", inner);
    } else if(srcref_equals_string(annot->name, LANG_TYPENAME_STRING)) {
        return "[c]";
    } else if(srcref_equals_string(annot->name, LANG_TYPENAME_VOID)) {
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
char* typing_signature(arena_t* a, ast_node_t* n) {
    switch(n->type) {
        case AST_VALUE: {
            return value_signature(n->u.n_value);
        } break;
        case AST_VAR_DECL: {
            srcref_t name = n->u.n_vardecl.name;
            return asprint(a, "$%.*s",
                srcref_len(name),
                srcref_ptr(name));
        } break;
        case AST_ARRAY: {
            char* inner = NULL;
            for(size_t i = 0; i < n->u.n_array.count; i++) {
                char* tmp = typing_signature(a, n->u.n_array.content[i]);
                if( inner == NULL )
                    inner = tmp;
                if( strcmp(tmp, inner) != 0 ) {
                    inner = NULL;
                    break;
                }
            }
            if( inner == NULL )
                inner = "*";
            return asprint(a, "[%s]", inner);
        } break;
        case AST_BLOCK: {
            char* sign = "";
            for(size_t i = 0; i < n->u.n_block.count; i++) {
                ast_node_t* inner = n->u.n_block.content[i];
                sign = asprint(a, "%s%s", sign, typing_signature(a, inner));
            }
            return sign;
        } break;
        case AST_FUN_CALL: {
            srcref_t name = n->u.n_funcall.name;
            return asprint(a, "#%.*s:%s",
                srcref_len(name),
                srcref_ptr(name),
                typing_signature(a, n->u.n_funcall.args));
        } break;
        case AST_FUN_DECL: {
            return typing_signature(a, n->u.n_fundecl.funsign);
        } break;
        case AST_FUN_SIGN: {
            srcref_t name = n->u.n_funsign.name;
            return asprint(a, "#%.*s:%s",
                srcref_len(name),
                srcref_ptr(name),
                typing_signature(a, n->u.n_funsign.argspec));
        } break;
        case AST_BINOP: {
            return asprint(a, "#%s:%s%s",
                binop_name(n->u.n_binop.type),
                typing_signature(a, n->u.n_binop.left),
                typing_signature(a, n->u.n_binop.right));
        } break;
        case AST_UNOP: {
            return asprint(a, "#%s:%s",
                unop_name(n->u.n_unop.type),
                typing_signature(a, n->u.n_unop.inner));
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

char* infer(trace_t* trace, ast_node_t* node, ctx_t* ctx) {
    switch(node->type) {
        case AST_VALUE: {
            if( is_valid_value(node->u.n_value) == false ) {
                trace_msg_t* msg = trace_create_message(trace,
                    TM_ERROR, ast_extract_srcref(node));
                trace_msg_append_costr(msg, "value: unknown value type");
            }
            return value_signature(node->u.n_value);
        } break;
        case AST_VAR_DECL: {
            // Add variable signature to context
            srcref_t name = node->u.n_vardecl.name;
            ast_annot_t* annot = node->u.n_vardecl.type;
            char* annot_sign = annotation_signature(ctx->arena, annot);
            char* var_sign = asprint(ctx->arena, "$%.*s",
                srcref_len(name),
                srcref_ptr(name));
            if( annot_sign != NULL ) {
                ctx_insert(ctx, var_sign, annot_sign);
            } else {
                trace_msg_t* msg = trace_create_message(trace,
                    TM_ERROR, annot->name);
                trace_msg_append_costr(msg, "variable declaration: invalid type annotation");
            }
            return annot_sign;
        } break;
        case AST_ARRAY: {
            char* inner = NULL;
            for(size_t i = 0; i < node->u.n_array.count; i++) {
                char* tmp = infer(trace, node->u.n_array.content[i], ctx);
                if( inner == NULL )
                    inner = tmp;
                if( strcmp(tmp, inner) != 0 ) {
                    inner = NULL;
                    break;
                }
            }
            if( inner == NULL ) {
                trace_msg_t* msg = trace_create_message(trace,
                    TM_ERROR, ast_extract_srcref(node));
                trace_msg_append_costr(msg, "array definition: arrays must contain items of the same type");
                inner = "*";
            }
            return asprint(ctx->arena, "[%s]", inner);
        } break;
        case AST_BLOCK: {
            // expressions generate type signatures
            // - values
            // - variables (what about vars in a body?)
            // - function calls
            // statements do not generate type signatures
            // with the exception of the return statement
            char* sign = "";
            for(size_t i = 0; i < node->u.n_block.count; i++) {
                ast_node_t* inner = node->u.n_block.content[i];
                // infer all regardless if we use the result
                // (needed to check all statements / expressions
                //  inside the body)
                char* tmp = infer(trace, inner, ctx);
                sign = asprint(ctx->arena, "%s%s", sign, tmp);
            }
            return sign;
        } break;
        case AST_FUN_CALL: {
            srcref_t name = node->u.n_funcall.name;
            char* sign = asprint(ctx->arena, "#%.*s:%s",
                srcref_len(name),
                srcref_ptr(name),
                infer(trace, node->u.n_funcall.args, ctx));
            char* returnsign = ctx_infer(ctx, sign);
            if( returnsign == NULL ) {
                trace_msg_t* msg = trace_create_message(trace,
                    TM_ERROR, ast_extract_srcref(node));
                trace_msg_append_costr(msg, "function call: unknown function ");
                trace_msg_append_srcref(msg, ast_extract_srcref(node));
                returnsign = "";
            }
            return returnsign;
        } break;
        case AST_FUN_DECL: {
            ast_node_t* funsign = node->u.n_fundecl.funsign;

            // Add function signature to context
            infer(trace, funsign, ctx);

            // Get actual return signature
            // 'iiiii': multiple return statements returning int, this is ok
            // 'ibbf':  multiple return statements returning different types, not ok
            // 'i':     a single return statement returning an int, this is ok
            // '':      no return statement, also ok
            char* body_return = infer(trace, node->u.n_fundecl.body, ctx_clone(ctx));

            char* annot_return = annotation_signature(ctx->arena, funsign->u.n_funsign.return_type);
            if( annot_return == NULL ) {
                srcref_t ref = ast_srcref_from_annotation(funsign->u.n_funsign.return_type);
                trace_msg_t* msg = trace_create_message(trace,
                    TM_ERROR, ref);
                trace_msg_append_costr(msg, "function declaration: invalid return type annotation: ");
                trace_msg_append_srcref(msg, ref);
                return "";
            }

            // Compare returned type with declared type
            if( strcmp(body_return, "") == 0 )
                body_return = "n"; // no return statement (n=none)
            
            size_t seg_len = strlen(annot_return);
            size_t len = strlen(body_return);

            // Check if len is a multiple of seg_len
            bool match = (len % seg_len) == 0;
            if( match ) {
                for(size_t i = 0; i < len; i+=seg_len) {
                    if( strncmp(body_return + i, annot_return, seg_len) != 0 ) {
                        match = false;
                        break;
                    }
                }
            }

            if( match == false ) {
                trace_msg_t* msg = trace_create_message(trace,
                    TM_ERROR, ast_extract_srcref(node));
                trace_msg_append_costr(msg,
                    "function declaration: return type (");
                srcref_t ref = ast_srcref_from_annotation(funsign->u.n_funsign.return_type);
                trace_msg_append_srcref(msg, ref);
                trace_msg_append_costr(msg, ") does not match returned value type.");
            }

            return "";
        } break;
        case AST_FUN_SIGN: {
            // Add function signature to context
            ast_annot_t* annot = node->u.n_funsign.return_type;
            char* retsign = annotation_signature(ctx->arena, annot);
            srcref_t name = node->u.n_funsign.name;
            char* sign = asprint(ctx->arena, "#%.*s:%s",
                srcref_len(name),
                srcref_ptr(name),
                infer(trace, node->u.n_funsign.argspec, ctx));
            if( retsign != NULL ) {
                ctx_insert(ctx, sign, retsign);
            } else {
                srcref_t ref = ast_srcref_from_annotation(annot);
                trace_msg_t* msg = trace_create_message(trace,
                    TM_ERROR, ref);
                trace_msg_append_costr(msg, "function signature: invalid type annotation: ");
                trace_msg_append_srcref(msg, ref);
            }
            return "";
        } break;
        case AST_BINOP: {
            // Lookup global definition and return it's type
            char* sign = asprint(ctx->arena, "#%s:%s%s",
                binop_name(node->u.n_binop.type),
                infer(trace, node->u.n_binop.left, ctx),
                infer(trace, node->u.n_binop.right, ctx));
            char* rettype = ctx_infer(ctx, sign);
            if( rettype == NULL ) {
                trace_msg_t* msg = trace_create_message(trace,
                        TM_ERROR, ast_extract_srcref(node));
                trace_msg_append_costr(msg, "binary operation: undefined (");
                trace_msg_append_srcref(msg, node->u.n_binop.ref);
                trace_msg_append_costr(msg, ")");
                rettype = "";
            }
            return rettype;
        } break;
        case AST_UNOP: {
            // Lookup global definition and return it's type
            char* sign = asprint(ctx->arena, "#%s:%s",
                unop_name(node->u.n_unop.type),
                infer(trace, node->u.n_unop.inner, ctx));
            char* rettype = ctx_infer(ctx, sign);
            if( rettype == NULL ) {
                trace_msg_t* msg = trace_create_message(trace,
                        TM_ERROR, ast_extract_srcref(node));
                trace_msg_append_costr(msg, "unary operation: undefined (");
                trace_msg_append_srcref(msg, node->u.n_binop.ref);
                trace_msg_append_costr(msg, ")");
                rettype = "";
            }
            return rettype;
        } break;
        case AST_IF_CHAIN: {
            ast_node_t* ifcond = node->u.n_if.cond;
            ast_node_t* ifbody = node->u.n_if.iftrue;
            ast_node_t* next = node->u.n_if.next;

            char* sign = infer(trace, ifcond, ctx);
            size_t len = strlen(sign);
            for(size_t i = 0; i < len; i++) {
                if( sign[i] != 'b' ) {
                    trace_msg_t* msg = trace_create_message(trace,
                        TM_ERROR, ast_extract_srcref(node));
                    trace_msg_append_costr(msg, "if-condition: non boolean type");
                }
            }

            if( len == 0 ) {
                trace_msg_t* msg = trace_create_message(trace,
                        TM_ERROR, ast_extract_srcref(node)); // this will not be a valid reference
                trace_msg_append_costr(msg, "if-condition: empty");
            }

            char* bodysign = infer(trace, ifbody, ctx_clone(ctx));
            char* nextsign = infer(trace, next, ctx_clone(ctx));
            return asprint(ctx->arena, "%s%s", bodysign, nextsign);
        } break;
        case AST_ASSIGN: {
            ast_node_t* left = node->u.n_assign.left_var;
            ast_node_t* right = node->u.n_assign.right_value;
            char* left_sign = infer(trace, left, ctx);
            char* right_sign = infer(trace, right, ctx);
            if( strcmp(left_sign, right_sign) != 0 ) {
                trace_msg_t* msg = trace_create_message(trace,
                        TM_ERROR, ast_extract_srcref(node));
                trace_msg_append_costr(msg,
                    "assignment: variable type does not match value type:\n\t");
                trace_msg_append_srcref(msg, ast_extract_srcref(node));
            }
            return "";
        } break;
        case AST_VAR_REF: {
            srcref_t name = node->u.n_varref.name;
            
            char* sign = ctx_infer(ctx,
                            asprint(ctx->arena, "$%.*s",
                                srcref_len(name),
                                srcref_ptr(name)));
            if( sign == NULL ) {
                trace_msg_t* msg = trace_create_message(trace,
                        TM_ERROR, node->u.n_varref.name);
                trace_msg_append_costr(msg,
                    "variable reference: reference to undefined variable");
                sign = "";
            }
            return sign;
        } break;
        case AST_RETURN: {
            return ctx_infer(ctx, infer(trace, node->u.n_return.result, ctx));
        } break;
        case AST_BREAK: {
            return "";
        } break;
        case AST_FOREACH: {
            char* varsign = infer(trace, node->u.n_foreach.vardecl, ctx);
            char* collsign = infer(trace, node->u.n_foreach.collection, ctx);
            char* collsign_expect = asprint(ctx->arena, "[%s]", varsign);
            if( strcmp(collsign, collsign_expect) != 0 ) {
                trace_msg_t* msg = trace_create_message(trace,
                        TM_ERROR, node->u.n_foreach.vardecl->u.n_vardecl.name);
                trace_msg_append_costr(msg,
                    "for-each: variable type does not match array content");
            }
            return infer(trace, node->u.n_foreach.during, ctx);
        } break;
        default: {
            return "";
        } break;
    }
}

ctx_t* typing_check(arena_t* arena, trace_t* trace, ast_node_t* root) {
    ctx_t* ctx = ctx_create(arena, 16);
    ctx_insert(ctx, "#+:ff", "f");
    ctx_insert(ctx, "#-:ff", "f");
    ctx_insert(ctx, "#*:ff", "f");
    ctx_insert(ctx, "#/:ff", "f");
    ctx_insert(ctx, "#+:ii", "i");
    ctx_insert(ctx, "#-:ii", "i");
    ctx_insert(ctx, "#*:ii", "i");
    ctx_insert(ctx, "#/:ii", "i");
    ctx_insert(ctx, "#>:ii", "b");
    ctx_insert(ctx, "#<:ii", "b");
    ctx_insert(ctx, "#<=:ii", "b");
    ctx_insert(ctx, "#>=:ii", "b");
    ctx_insert(ctx, "#==:ii", "b");
    ctx_insert(ctx, "#!=:ii", "b");
    ctx_insert(ctx, "#>:ff", "b");
    ctx_insert(ctx, "#<:ff", "b");
    ctx_insert(ctx, "#<=:ff", "b");
    ctx_insert(ctx, "#>=:ff", "b");
    ctx_insert(ctx, "#==:ff", "f");
    ctx_insert(ctx, "#!=:ff", "f");
    ctx_insert(ctx, "#-:f", "f");
    ctx_insert(ctx, "#-:i", "i");
    ctx_insert(ctx, "#%:ii", "i");
    ctx_insert(ctx, "#not:b", "b");
    ctx_insert(ctx, "#and:bb", "b");
    ctx_insert(ctx, "#or:bb", "b");
    infer(trace, root, ctx);
    return ctx;
}