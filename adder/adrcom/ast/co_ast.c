#include "adrcom/ast/co_ast.h"
#include "shared/sh_json.h"
#include <stdlib.h>
#include <stdarg.h>

ast_t* ast_leaf(arena_t* allocator, ast_tag_t tag) {
    ast_t* node = (ast_t*) aalloc(allocator, sizeof(ast_t));
    *node = (ast_t) { 0 };
    node->tag = tag;
    node->diagnostics = NULL;
    return node;
}

ast_t* ast(arena_t* allocator, ast_tag_t tag, int size) {
    ast_t* node = (ast_t*) aalloc(allocator, sizeof(ast_t));
    *node = (ast_t) {
        .tag = tag,
        .size = size,
        .diagnostics = NULL,
        .as.items = (ast_t**) aalloc(allocator,
            ast_calculate_capacity(size) * sizeof(ast_t*))
    };
    return node;
}

bool _ast_attach_diag(arena_t* allocator, ast_t* node, diag_kind_t kind, diphrase_t phrase, dimsg_t* msg, size_t msglen) {

    if(node->diagnostics == NULL) {
        node->diagnostics = (ast_diags_t*) aalloc(allocator, sizeof(ast_diags_t));
        if( node->diagnostics == NULL )
            return false;
        int init_size_one = 1;
        node->diagnostics->list = (diag_t**) aalloc(allocator,
            sizeof(diag_t*) * ast_calculate_capacity(init_size_one));
        if( node->diagnostics->list == NULL ) {
            node->diagnostics = NULL;
            return false;
        }
        node->diagnostics->size = 0;
        node->diagnostics->list[0] = mk_diag(allocator, kind, phrase, msg, msglen);
        if(node->diagnostics->list[0] == NULL)
            return false;
        node->diagnostics->size = init_size_one;
        return true;
    }

    int old_cap = ast_calculate_capacity(node->diagnostics->size);
    
    if(old_cap < (node->diagnostics->size + 1)) {
        int new_cap = ast_calculate_capacity(node->diagnostics->size + 1);
        diag_t** new_lst = (diag_t**) arealloc(allocator,
            node->diagnostics->list,
            sizeof(diag_t*) * new_cap);
        if( new_lst == NULL )
            return false;
        node->diagnostics->list = new_lst;
    }

    int new_index = node->diagnostics->size;
    node->diagnostics->list[new_index] = mk_diag(allocator, kind, phrase, msg, msglen);
    if(node->diagnostics->list[new_index] == NULL)
        return false;

    node->diagnostics->size += 1;
    return true;
}

srcref_t ast_aggregate_srcref(ast_t* node) {

    if(node->tag == AST_SYMBOL)
        return node->as.srcref;

    if(node->tag == AST_STRING)
        return node->as.srcref;

    if(node->size == 0)
        return (srcref_t) {0};

    srcref_t agg = { 0 };

    for(int i = 0; i < node->size; i++) {
        assert(node->as.items[i] != node);
        agg = srcref_combine(agg,
            ast_aggregate_srcref(node->as.items[i]));
    }

    return agg;
}


srcref_t ast_try_get_name(ast_t* n) {
    switch(n->tag) {
        case AST_SYMBOL:    return n->as.srcref;
        case AST_VARDECL:   /* FALLTHROUGH */
        case AST_VARREF:    /* FALLTHROUGH */
        case AST_FUNCALL:   /* FALLTHROUGH */
        case AST_FUNSIGN:   /* FALLTHROUGH */
        case AST_FUNDEFN: {
            for(int i = 0; i < n->size; i++) {
                srcref_t srcref = ast_try_get_name(n->as.items[i]);
                if( srcref_is_valid(srcref) )
                    return srcref;
            }
        }                   /* FALLTHROUGH */
        default:            break;
    }
    return (srcref_t) { 0 };
}

ast_t* ast_try_get(ast_t* n, ast_tag_t tag) {
    if(n->tag == tag)
        return n;
    for(int i = 0; i < n->size; i++) {
        if(n->as.items[i]->tag == tag)
            return n->as.items[i];
    }
    return NULL;
}


const char* ast_tag_to_string(ast_tag_t tag) {
    switch(tag) {
        case AST_UNDEFINED:                 return "AST_UNDEFINED";
        case AST__BEGIN_VALUES:             return "AST__BEGIN_VALUES";
        case AST_INT:                       return "AST_INT";
        case AST_FLOAT:                     return "AST_FLOAT";
        case AST_BOOL:                      return "AST_BOOL";
        case AST_CHAR:                      return "AST_CHAR";
        case AST_STRING:                    return "AST_STRING";
        case AST_SYMBOL:                    return "AST_SYMBOL";
        case AST_ARRAY:                     return "AST_ARRAY";
        case AST__END_VALUES:               return "AST__END_VALUES";
        case AST__BEGIN_UNARY_OPERATORS:    return "AST__BEGIN_UNARY_OPERATORS";
        case AST_UNA_NOT:                   return "AST_UNA_NOT";
        case AST_UNA_NEG:                   return "AST_UNA_NEG";
        case AST__END_UNARY_OPERATORS:      return "AST__END_UNARY_OPERATORS";
        case AST__BEGIN_BINARY_OPERATORS:   return "AST__BEGIN_BINARY_OPERATORS";
        case AST_BIN_MUL:                   return "AST_BIN_MUL";
        case AST_BIN_DIV:                   return "AST_BIN_DIV";
        case AST_BIN_MOD:                   return "AST_BIN_MOD";
        case AST_BIN_ADD:                   return "AST_BIN_ADD";
        case AST_BIN_SUB:                   return "AST_BIN_SUB";
        case AST_BIN_XOR:                   return "AST_BIN_XOR";
        case AST_BIN_LT:                    return "AST_BIN_LT";
        case AST_BIN_GT:                    return "AST_BIN_GT";
        case AST_BIN_LT_EQ:                 return "AST_BIN_LT_EQ";
        case AST_BIN_GT_EQ:                 return "AST_BIN_GT_EQ";
        case AST_BIN_EQ:                    return "AST_BIN_EQ";
        case AST_BIN_NEQ:                   return "AST_BIN_NEQ";
        case AST_BIN_OR:                    return "AST_BIN_OR";
        case AST_BIN_AND:                   return "AST_BIN_AND";
        case AST__END_BINARY_OPERATORS:     return "AST__END_BINARY_OPERATORS";
        case AST__BEGIN_HIGH_LEVEL:         return "AST__BEGIN_HIGH_LEVEL";
        case AST_TYDESCR:                   return "AST_TYDESCR";
        case AST_VARREF:                    return "AST_VARREF";
        case AST_VARDECL:                   return "AST_VARDECL";
        case AST_FUNDEFN:                   return "AST_FUNDEFN";
        case AST_FUNSIGN:                   return "AST_FUNSIGN";
        case AST_FUNCALL:                   return "AST_FUNCALL";
        case AST_FOREACH:                   return "AST_FOREACH";
        case AST_BLOCK:                     return "AST_BLOCK";
        case AST_ARGLIST:                   return "AST_ARGLIST";
        case AST_IFCHAIN:                   return "AST_IFCHAIN";
        case AST_RETURN:                    return "AST_RETURN";
        case AST_ASSIGN:                    return "AST_ASSIGN";
        case AST__END_HIGH_LEVEL:           return "AST__END_HIGH_LEVEL";
        case AST__COUNT:                    return "AST__COUNT";
        default:                            return "AST <UNKNOWN TAG>";
    }
}

bool ast_tag_is_unop(ast_tag_t tag) {
    return tag > AST__BEGIN_UNARY_OPERATORS
        && tag < AST__END_UNARY_OPERATORS;
}

bool ast_is_unop(ast_t* node) {
    return ast_tag_is_unop(node->tag);
}

bool ast_tag_is_binop(ast_tag_t tag) {
    return tag > AST__BEGIN_BINARY_OPERATORS
        && tag < AST__END_BINARY_OPERATORS;
}

bool ast_is_binop(ast_t* node) {
    return ast_tag_is_binop(node->tag);
}

bool ast_tag_is_value(ast_tag_t tag) {
    return tag > AST__BEGIN_VALUES
        && tag < AST__END_VALUES;
}

bool ast_is_value(ast_t* node) {
    return ast_tag_is_value(node->tag);
}

bool ast_tag_is_list(ast_tag_t tag) {
    return tag == AST_ARRAY
        || tag == AST_BLOCK
        || tag == AST_ARGLIST;
}

bool ast_is_list(ast_t* node) {
    return ast_tag_is_list(node->tag);
}

bool ast_is_valid_else_block(ast_t* node) {
    if( node == NULL )
        return false;
    return node->tag == AST_BLOCK
        && node->size > 0;
}

/////////////// BUILDERS /////////////////

ast_t* ast_int(arena_t* arena, int value) {
    ast_t* node = ast_leaf(arena, AST_INT);
    if(node == NULL)
        return NULL;
    node->as.value_int = value;
    return node;
}

ast_t* ast_float(arena_t* arena, float value) {
    ast_t* node = ast_leaf(arena, AST_FLOAT);
    if(node == NULL)
        return NULL;
    node->as.value_float = value;
    return node;
}

ast_t* ast_bool(arena_t* arena, bool value) {
    ast_t* node = ast_leaf(arena, AST_BOOL);
    if(node == NULL)
        return NULL;
    node->as.value_bool = value;
    return node;
}

ast_t* ast_char(arena_t* arena, char value) {
    ast_t* node = ast_leaf(arena, AST_CHAR);
    if(node == NULL)
        return NULL;
    node->as.value_char = value;
    return node;
}

ast_t* ast_string(arena_t* arena, srcref_t value) {
    ast_t* node = ast_leaf(arena, AST_STRING);
    if(node == NULL)
        return NULL;
    node->as.srcref = value;
    return node;
}

ast_t* ast_symbol(arena_t* arena, srcref_t value) {
    assert(srcref_is_valid(value));
    ast_t* node = ast_leaf(arena, AST_SYMBOL);
    if(node == NULL)
        return NULL;
    node->as.srcref = value;
    return node;
}

ast_t* ast_variable_reference(arena_t* arena, srcref_t ref) {
    assert(srcref_is_valid(ref));
    ast_t* n = ast(arena, AST_VARREF, 1);
    n->as.items[AST_VARREF_SYMBOL] = ast_symbol(arena, ref);
    return n;
}

ast_t* ast_variable_declaration(arena_t* arena, ast_t* type, ast_t* varref) {
    ast_t* n = ast(arena, AST_VARDECL, 2);
    n->as.items[AST_VARDECL_TYDESCR] = type;
    n->as.items[AST_VARDECL_VARREF] = varref;
    return n;
}

ast_t* _ast_list(arena_t* arena, ast_tag_t tag) {
    assert(ast_tag_is_list(tag));
    ast_t* node = ast(arena, tag, 0);
    return node;
}

bool _ast_list_append(arena_t* arena, ast_t* list, ast_t* value) {

    if( list == NULL )
        return false;

    if( ast_is_list(list) == false )
        return false;

    int capacity = ast_calculate_capacity(list->size);
    if( capacity <= 0 )
        return false;

    if( capacity == list->size ) {
        ptrdiff_t newcap = ast_calculate_capacity(list->size + 1);
        assert(list->size >= 0 && newcap > list->size);
        ast_t** items = arealloc(arena,
            list->as.items,
            sizeof(ast_t*) * newcap);
        if( items == NULL )
            return false;
        list->as.items = items;
    }
    
    list->as.items[list->size] = value;
    list->size += 1;

    return true;
}

ast_t* ast_array(arena_t* arena) {
    return _ast_list(arena, AST_ARRAY);
}

bool ast_array_append(arena_t* arena, ast_t* array, ast_t* value) {
    return _ast_list_append(arena, array, value);
}

ast_t* ast_arglist(arena_t* arena) {
    return _ast_list(arena, AST_ARGLIST);
}

bool ast_arglist_append(arena_t* arena, ast_t* args, ast_t* value) {
    return _ast_list_append(arena, args, value);
}

ast_t* ast_block(arena_t* arena) {
    return _ast_list(arena, AST_BLOCK);
}

bool ast_block_append(arena_t* arena, ast_t* block, ast_t* value) {
    return _ast_list_append(arena, block, value);
}

ast_t* ast_function_call(arena_t* arena, srcref_t name, ast_t* arglist) {
    assert(srcref_is_valid(name));
    assert(arglist->tag == AST_ARGLIST);
    ast_t* n = ast(arena, AST_FUNCALL, 2);
    n->as.items[AST_FUNCALL_SYMBOL] = ast_symbol(arena, name);
    n->as.items[AST_FUNCALL_ARGLIST] = arglist;
    return n;
}

ast_t* ast_unary_operation(arena_t* arena, ast_tag_t op, ast_t* inner) {
    assert(ast_tag_is_unop(op));
    ast_t* n = ast(arena, op, 1);
    n->as.items[AST_UNAOP_INNER] = inner;
    return n;
}

ast_t* ast_binary_operation(arena_t* arena, ast_tag_t op, ast_t* left, ast_t* right) {
    assert(ast_tag_is_binop(op));
    ast_t* n = ast(arena, op, 2);
    n->as.items[AST_BINOP_LEFT] = left;
    n->as.items[AST_BINOP_RIGHT] = right;
    return n;
}

ast_t* ast_type_descriptor(arena_t* arena, srcref_t name, ast_t* arglist) {
    assert(srcref_is_valid(name));
    if(arglist == NULL)
        arglist = ast_arglist(arena);
    assert(arglist->tag == AST_ARGLIST);
    ast_t* n = ast(arena, AST_TYDESCR, 2);
    n->as.items[AST_TYDESCR_SYMBOL] = ast_symbol(arena, name);
    n->as.items[AST_TYDESCR_ARGLIST] = arglist;
    return n;
}

ast_t* ast_assignment(arena_t* arena, ast_t* left, ast_t* right) {
    ast_t* n = ast(arena, AST_ASSIGN, 2);
    n->as.items[AST_ASSIGN_LEFT] = left;
    n->as.items[AST_ASSIGN_RIGHT] = right;
    return n;
}

ast_t* ast_if_chain(arena_t* arena, ast_t* condition, ast_t* if_true, ast_t* if_next) {
    ast_t* n = ast(arena, AST_IFCHAIN, 3);
    n->as.items[AST_IFCHAIN_COND] = condition;
    n->as.items[AST_IFCHAIN_IFTRUE] = if_true;
    n->as.items[AST_IFCHAIN_IFNEXT] = if_next;
    return n;
}

ast_t* ast_foreach(arena_t* arena, ast_t* var, ast_t* collection, ast_t* loop_body) {
    assert(var->tag == AST_VARREF || var->tag == AST_VARDECL);
    //assert(loop_body->tag == AST_BLOCK);
    ast_t* n = ast(arena, AST_FOREACH, 3);
    n->as.items[AST_FOREACH_VAR] = var;
    n->as.items[AST_FOREACH_COLL] = collection;
    n->as.items[AST_FOREACH_BODY] = loop_body;
    return n;
}

ast_t* ast_return(arena_t* arena, ast_t* return_expr) {
    ast_t* n = ast(arena, AST_RETURN, 1);
    n->as.items[AST_RETURN_EXPR] = return_expr;
    return n;
}

ast_t* ast_function_signature(arena_t* arena, ast_t* type, srcref_t name, ast_t* arglist, int ffistate) {
    assert(srcref_is_valid(name));
    assert(arglist->tag == AST_ARGLIST);
    ast_t* n = ast(arena, AST_FUNSIGN, 4);
    n->as.items[AST_FUNSIGN_TYDESCR] = type;
    n->as.items[AST_FUNSIGN_SYMBOL] = ast_symbol(arena, name);
    n->as.items[AST_FUNSIGN_ARGLIST] = arglist;
    n->as.items[AST_FUNSIGN_FFI] = ast_int(arena, ffistate);
    return n;
}

ast_t* ast_function_definition(arena_t* arena, ast_t* funsign, ast_t* body) {
    assert(funsign->tag == AST_FUNSIGN);
    assert(body->tag == AST_BLOCK);
    ast_t* n = ast(arena, AST_FUNDEFN, 2);
    n->as.items[AST_FUNDEFN_FUNSIGN] = funsign;
    n->as.items[AST_FUNDEFN_BODY] = body;
    return n;
}

void ast_set_exported(ast_t* n) {
    if(n->tag == AST_FUNDEFN)
        n = n->as.items[AST_FUNDEFN_FUNSIGN];
    if(n->tag == AST_FUNSIGN)
        n->as.items[AST_FUNSIGN_FFI]->as.value_int = AST_FUNSIGN_FFI_VAL_EXPORT;
}

void ast_set_imported(ast_t* n) {
    if(n->tag == AST_FUNDEFN)
        n = n->as.items[AST_FUNDEFN_FUNSIGN];
    if(n->tag == AST_FUNSIGN)
        n->as.items[AST_FUNSIGN_FFI]->as.value_int = AST_FUNSIGN_FFI_VAL_IMPORT;
}

bool ast_is_exported(ast_t* n) {
    int ffi_state = 0;
    if(n->tag == AST_FUNDEFN)
        n = n->as.items[AST_FUNDEFN_FUNSIGN];
    if(n->tag == AST_FUNSIGN)
        ffi_state = n->as.items[AST_FUNSIGN_FFI]->as.value_int;
    return ffi_state == AST_FUNSIGN_FFI_VAL_EXPORT;
}

bool ast_is_imported(ast_t* n) {
    int ffi_state = 0;
    if(n->tag == AST_FUNDEFN)
        n = n->as.items[AST_FUNDEFN_FUNSIGN];
    if(n->tag == AST_FUNSIGN)
        ffi_state = n->as.items[AST_FUNSIGN_FFI]->as.value_int;
    return ffi_state == AST_FUNSIGN_FFI_VAL_IMPORT;
}
