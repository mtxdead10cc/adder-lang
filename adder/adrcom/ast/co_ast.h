#ifndef AST_EXPR_H_
#define AST_EXPR_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <adrcom/shared/co_types.h>
#include <adrcom/shared/co_utils.h>

#include <shared/sh_utils.h>
#include <shared/sh_arena.h>
#include <shared/sh_log.h>

typedef enum ast_tag_t {

    AST_UNDEFINED       = 0x00,

    AST__BEGIN_VALUES,

    AST_INT,
    AST_FLOAT,
    AST_BOOL,
    AST_CHAR,
    AST_STRING,
    AST_SYMBOL,
    AST_ARRAY,

    AST__END_VALUES,

    AST__BEGIN_UNARY_OPERATORS,

    AST_UNA_NOT,
    AST_UNA_NEG,

    AST__END_UNARY_OPERATORS,

    AST__BEGIN_BINARY_OPERATORS,

    AST_BIN_MUL,       
    AST_BIN_DIV,
    AST_BIN_MOD,
    AST_BIN_ADD,
    AST_BIN_SUB,
    AST_BIN_XOR,
    AST_BIN_LT,
    AST_BIN_GT,
    AST_BIN_LT_EQ,
    AST_BIN_GT_EQ,
    AST_BIN_EQ,
    AST_BIN_NEQ,
    AST_BIN_OR,
    AST_BIN_AND,

    AST__END_BINARY_OPERATORS,

    AST__BEGIN_HIGH_LEVEL,

    AST_TYDESCR,
    AST_VARDECL,
    AST_VARREF,
    AST_FUNDEFN,
    AST_FUNSIGN,
    AST_FUNCALL,
    AST_FOREACH,
    AST_BLOCK,
    AST_ARGLIST,
    AST_IFCHAIN,
    AST_RETURN,
    AST_ASSIGN,

    AST__END_HIGH_LEVEL,

} ast_tag_t;


typedef struct ast_t ast_t;

typedef struct ast_t {
    ast_tag_t       tag;
    int             size;
    union {
        ast_t**     items;
        srcref_t    srcref;
        int         value_int;
        bool        value_bool;
        float       value_float;
        char        value_char;
    } as;
} ast_t;

inline static int ast_calculate_capacity(int size) {
    int n = size;
    for(int s = 1; s < 32; s++) {
        if(n <= (1 << s))
            return (1 << s);
    }
    assert(false && "node max capacity (due to int32)");
    return -1;
}

ast_t*      ast_leaf(arena_t* allocator, ast_tag_t tag);
ast_t*      ast(arena_t* allocator, ast_tag_t tag, int size);

srcref_t    ast_agg_srcrefs(ast_t* n);
srcref_t    ast_try_get_name(ast_t* n);
ast_t*      ast_try_get(ast_t* n, ast_tag_t tag);

const char* ast_tag_to_string(ast_tag_t tag);
void ast_print(ast_t* node);

typedef struct json_value_t json_value_t;

json_value_t* ast_to_json(arena_t* ator, ast_t* node);

inline static bool ast_tag_is_unop(ast_tag_t tag) {
    return tag > AST__BEGIN_UNARY_OPERATORS
        && tag < AST__END_UNARY_OPERATORS;
}

inline static bool ast_is_unop(ast_t* node) {
    return ast_tag_is_unop(node->tag);
}

inline static bool ast_tag_is_binop(ast_tag_t tag) {
    return tag > AST__BEGIN_BINARY_OPERATORS
        && tag < AST__END_BINARY_OPERATORS;
}

inline static bool ast_is_binop(ast_t* node) {
    return ast_tag_is_binop(node->tag);
}

inline static bool ast_tag_is_value(ast_tag_t tag) {
    return tag > AST__BEGIN_VALUES
        && tag < AST__END_VALUES;
}

inline static bool ast_is_value(ast_t* node) {
    return ast_tag_is_value(node->tag);
}

inline static bool ast_tag_is_list(ast_tag_t tag) {
    return tag == AST_ARRAY
        || tag == AST_BLOCK
        || tag == AST_ARGLIST;
}

inline static bool ast_is_list(ast_t* node) {
    return ast_tag_is_list(node->tag);
}

inline static bool ast_is_valid_else_block(ast_t* node) {
    if( node == NULL )
        return false;
    return node->tag == AST_BLOCK
        && node->size > 0;
}

/////////////// BUILDERS /////////////////

inline static ast_t* ast_int(arena_t* arena, int value) {
    ast_t* node = ast_leaf(arena, AST_INT);
    node->as.value_int = value;
    return node;
}

inline static ast_t* ast_float(arena_t* arena, float value) {
    ast_t* node = ast_leaf(arena, AST_FLOAT);
    node->as.value_float = value;
    return node;
}

inline static ast_t* ast_bool(arena_t* arena, bool value) {
    ast_t* node = ast_leaf(arena, AST_BOOL);
    node->as.value_bool = value;
    return node;
}

inline static ast_t* ast_char(arena_t* arena, char value) {
    ast_t* node = ast_leaf(arena, AST_CHAR);
    node->as.value_char = value;
    return node;
}

inline static ast_t* ast_string(arena_t* arena, srcref_t value) {
    ast_t* node = ast_leaf(arena, AST_STRING);
    node->as.srcref = value;
    return node;
}

inline static ast_t* ast_symbol(arena_t* arena, srcref_t value) {
    ast_t* node = ast_leaf(arena, AST_SYMBOL);
    node->as.srcref = value;
    return node;
}


#define AST_VARREF_SYMBOL 0

inline static ast_t* ast_variable_reference(arena_t* arena, srcref_t ref) {
    ast_t* n = ast(arena, AST_VARREF, 1);
    n->as.items[AST_VARREF_SYMBOL] = ast_symbol(arena, ref);
    return n;
}

#define AST_VARDECL_TYDESCR 0
#define AST_VARDECL_VARREF  1

inline static ast_t* ast_variable_declaration(arena_t* arena, ast_t* type, ast_t* varref) {
    ast_t* n = ast(arena, AST_VARDECL, 2);
    n->as.items[AST_VARDECL_TYDESCR] = type;
    n->as.items[AST_VARDECL_VARREF] = varref;
    return n;
}

inline static ast_t* _ast_list(arena_t* arena, ast_tag_t tag) {
    assert(ast_tag_is_list(tag));
    ast_t* node = ast(arena, tag, 0);
    return node;
}

inline static bool _ast_list_append(arena_t* arena, ast_t* list, ast_t* value) {

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

inline static ast_t* ast_array(arena_t* arena) {
    return _ast_list(arena, AST_ARRAY);
}

inline static bool ast_array_append(arena_t* arena, ast_t* array, ast_t* value) {
    return _ast_list_append(arena, array, value);
}

inline static ast_t* ast_arglist(arena_t* arena) {
    return _ast_list(arena, AST_ARGLIST);
}

inline static bool ast_arglist_append(arena_t* arena, ast_t* args, ast_t* value) {
    return _ast_list_append(arena, args, value);
}

inline static ast_t* ast_block(arena_t* arena) {
    return _ast_list(arena, AST_BLOCK);
}

inline static bool ast_block_append(arena_t* arena, ast_t* block, ast_t* value) {
    return _ast_list_append(arena, block, value);
}

#define AST_FUNCALL_SYMBOL  0
#define AST_FUNCALL_ARGLIST 1

inline static ast_t* ast_function_call(arena_t* arena, srcref_t name, ast_t* arglist) {
    assert(arglist->tag == AST_ARGLIST);
    ast_t* n = ast(arena, AST_FUNCALL, 2);
    n->as.items[AST_FUNCALL_SYMBOL] = ast_symbol(arena, name);
    n->as.items[AST_FUNCALL_ARGLIST] = arglist;
    return n;
}

#define AST_UNAOP_INNER 0

inline static ast_t* ast_unary_operation(arena_t* arena, ast_tag_t op, ast_t* inner) {
    assert(ast_tag_is_unop(op));
    ast_t* n = ast(arena, op, 1);
    n->as.items[AST_UNAOP_INNER] = inner;
    return n;
}

#define AST_BINOP_LEFT  0
#define AST_BINOP_RIGHT 1

inline static ast_t* ast_binary_operation(arena_t* arena, ast_tag_t op, ast_t* left, ast_t* right) {
    assert(ast_tag_is_binop(op));
    ast_t* n = ast(arena, op, 2);
    n->as.items[AST_BINOP_LEFT] = left;
    n->as.items[AST_BINOP_RIGHT] = right;
    return n;
}

#define AST_TYDESCR_SYMBOL  0
#define AST_TYDESCR_ARGLIST 1

inline static ast_t* ast_type_descriptor(arena_t* arena, srcref_t name, ast_t* arglist) {
    if(arglist == NULL)
        arglist = ast_arglist(arena);
    assert(arglist->tag == AST_ARGLIST);
    ast_t* n = ast(arena, AST_TYDESCR, 2);
    n->as.items[AST_TYDESCR_SYMBOL] = ast_symbol(arena, name);
    n->as.items[AST_TYDESCR_ARGLIST] = arglist;
    return n;
}

#define AST_ASSIGN_LEFT  0
#define AST_ASSIGN_RIGHT 1

inline static ast_t* ast_assignment(arena_t* arena, ast_t* left, ast_t* right) {
    ast_t* n = ast(arena, AST_ASSIGN, 2);
    n->as.items[AST_ASSIGN_LEFT] = left;
    n->as.items[AST_ASSIGN_RIGHT] = right;
    return n;
}

#define AST_IFCHAIN_COND   0
#define AST_IFCHAIN_IFTRUE 1
#define AST_IFCHAIN_IFNEXT 2

inline static ast_t* ast_if_chain(arena_t* arena, ast_t* condition, ast_t* if_true, ast_t* if_next) {
    ast_t* n = ast(arena, AST_IFCHAIN, 3);
    n->as.items[AST_IFCHAIN_COND] = condition;
    n->as.items[AST_IFCHAIN_IFTRUE] = if_true;
    n->as.items[AST_IFCHAIN_IFNEXT] = if_next;
    return n;
}

#define AST_FOREACH_VAR  0
#define AST_FOREACH_COLL 1
#define AST_FOREACH_BODY 2

inline static ast_t* ast_foreach(arena_t* arena, ast_t* var, ast_t* collection, ast_t* loop_body) {
    assert(var->tag == AST_VARREF || var->tag == AST_VARDECL);
    //assert(loop_body->tag == AST_BLOCK);
    ast_t* n = ast(arena, AST_FOREACH, 3);
    n->as.items[AST_FOREACH_VAR] = var;
    n->as.items[AST_FOREACH_COLL] = collection;
    n->as.items[AST_FOREACH_BODY] = loop_body;
    return n;
}

#define AST_RETURN_EXPR 0

inline static ast_t* ast_return(arena_t* arena, ast_t* return_expr) {
    ast_t* n = ast(arena, AST_RETURN, 1);
    n->as.items[AST_RETURN_EXPR] = return_expr;
    return n;
}

#define AST_FUNSIGN_TYDESCR  0
#define AST_FUNSIGN_SYMBOL   1
#define AST_FUNSIGN_ARGLIST  2
#define AST_FUNSIGN_FFI      3

#define AST_FUNSIGN_FFI_VAL_EXPORT 1
#define AST_FUNSIGN_FFI_VAL_IMPORT 2

inline static ast_t* ast_function_signature(arena_t* arena, ast_t* type, srcref_t name, ast_t* arglist, int ffistate) {
    assert(arglist->tag == AST_ARGLIST);
    ast_t* n = ast(arena, AST_FUNSIGN, 4);
    n->as.items[AST_FUNSIGN_TYDESCR] = type;
    n->as.items[AST_FUNSIGN_SYMBOL] = ast_symbol(arena, name);
    n->as.items[AST_FUNSIGN_ARGLIST] = arglist;
    n->as.items[AST_FUNSIGN_FFI] = ast_int(arena, ffistate);
    return n;
}

#define AST_FUNDEFN_FUNSIGN 0
#define AST_FUNDEFN_BODY    1

inline static ast_t* ast_function_definition(arena_t* arena, ast_t* funsign, ast_t* body) {
    assert(funsign->tag == AST_FUNSIGN);
    assert(body->tag == AST_BLOCK);
    ast_t* n = ast(arena, AST_FUNDEFN, 2);
    n->as.items[AST_FUNDEFN_FUNSIGN] = funsign;
    n->as.items[AST_FUNDEFN_BODY] = body;
    return n;
}

inline static void ast_set_exported(ast_t* n) {
    if(n->tag == AST_FUNDEFN)
        n = n->as.items[AST_FUNDEFN_FUNSIGN];
    if(n->tag == AST_FUNSIGN)
        n->as.items[AST_FUNSIGN_FFI]->as.value_int = AST_FUNSIGN_FFI_VAL_EXPORT;
}

inline static void ast_set_imported(ast_t* n) {
    if(n->tag == AST_FUNDEFN)
        n = n->as.items[AST_FUNDEFN_FUNSIGN];
    if(n->tag == AST_FUNSIGN)
        n->as.items[AST_FUNSIGN_FFI]->as.value_int = AST_FUNSIGN_FFI_VAL_IMPORT;
}

inline static bool ast_is_exported(ast_t* n) {
    int ffi_state = 0;
    if(n->tag == AST_FUNDEFN)
        n = n->as.items[AST_FUNDEFN_FUNSIGN];
    if(n->tag == AST_FUNSIGN)
        ffi_state = n->as.items[AST_FUNSIGN_FFI]->as.value_int;
    return ffi_state == AST_FUNSIGN_FFI_VAL_EXPORT;
}

inline static bool ast_is_imported(ast_t* n) {
        int ffi_state = 0;
    if(n->tag == AST_FUNDEFN)
        n = n->as.items[AST_FUNDEFN_FUNSIGN];
    if(n->tag == AST_FUNSIGN)
        ffi_state = n->as.items[AST_FUNSIGN_FFI]->as.value_int;
    return ffi_state == AST_FUNSIGN_FFI_VAL_IMPORT;
}

#endif // AST_EXPR_H_