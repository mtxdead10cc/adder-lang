#ifndef AST_BUILDER_H_
#define AST_BUILDER_H_

#include "adrcom/ast/co_ast.h"

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

inline static ast_t* ast_srcref(arena_t* arena, srcref_t value) {
    ast_t* node = ast_leaf(arena, AST_SRCREF);
    node->as.srcref = value;
    return node;
}

inline static ast_t* ast_flags(arena_t* arena, ast_flags_t value) {
    ast_t* node = ast_leaf(arena, AST_FLAGS);
    node->as.flags = value;
    return node;
}

inline static ast_t* ast_variable_reference(arena_t* arena, srcref_t ref) {
    ast_t* node = ast(arena, AST_VARREF, 1);
    node->as.items[0] = ast_srcref(arena, ref);
    return node;
}

inline static ast_t* ast_variable_declaration(arena_t* arena, ast_t* type, srcref_t ref) {
    ast_t* node = ast(arena, AST_VARDECL, 2);
    node->as.items[0] = type;
    node->as.items[1] = ast_srcref(arena, ref);
    return node;
}

inline static ast_t* _ast_list(arena_t* arena, ast_tag_t tag, int size) {
    ast_t* node = ast(arena, tag, size);
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

inline static ast_t* ast_array(arena_t* arena, int capacity) {
    return _ast_list(arena, AST_ARRAY, capacity);
}

inline static bool ast_array_append(arena_t* arena, ast_t* array, ast_t* value) {
    return _ast_list_append(arena, array, value);
}

inline static ast_t* ast_arglist(arena_t* arena, int capacity) {
    return _ast_list(arena, AST_ARGLIST, capacity);
}

inline static bool ast_arglist_append(arena_t* arena, ast_t* args, ast_t* value) {
    return _ast_list_append(arena, args, value);
}

inline static ast_t* ast_block(arena_t* arena, int capacity) {
    return _ast_list(arena, AST_BLOCK, capacity);
}

inline static bool ast_block_append(arena_t* arena, ast_t* block, ast_t* value) {
    return _ast_list_append(arena, block, value);
}

inline static ast_t* ast_function_call(arena_t* arena, srcref_t name, ast_t* arglist) {
    assert(arglist->tag == AST_ARGLIST);
    ast_t* node = ast(arena, AST_FUNCALL, 2);
    node->as.items[0] = ast_srcref(arena, name);
    node->as.items[1] = arglist;
    return node;
}

inline static ast_t* ast_unary_operation(arena_t* arena, ast_tag_t op, ast_t* inner) {
    assert(ast_tag_is_unop(op));
    ast_t* node = ast(arena, op, 1);
    node->as.items[0] = inner;
    return node;
}

inline static ast_t* ast_binary_operation(arena_t* arena, ast_tag_t op, ast_t* left, ast_t* right) {
    assert(ast_tag_is_binop(op));
    ast_t* node = ast(arena, op, 2);
    node->as.items[0] = left;
    node->as.items[1] = right;
    return node;
}

inline static ast_t* ast_type_descriptor(arena_t* arena, srcref_t name, ast_t* arglist) {
    if(arglist == NULL)
        arglist = ast_arglist(arena, 0);
    assert(arglist->tag == AST_ARGLIST);
    ast_t* node = ast(arena, AST_TYDESCR, 2);
    node->as.items[0] = ast_srcref(arena, name);
    node->as.items[1] = arglist;
    return node;
}

inline static ast_t* ast_assignment(arena_t* arena, ast_t* left, ast_t* right) {
    ast_t* node = ast(arena, AST_ASSIGN, 2);
    node->as.items[0] = left;
    node->as.items[1] = right;
    return node;
}

inline static ast_t* ast_if_chain(arena_t* arena, ast_t* condition, ast_t* if_true, ast_t* if_next) {
    ast_t* if_chain = ast(arena, AST_IFCHAIN, 3);
    if_chain->as.items[0] = condition;
    if_chain->as.items[1] = if_true;
    if_chain->as.items[2] = if_next;
    return if_chain;
}

inline static ast_t* ast_foreach(arena_t* arena, ast_t* var, ast_t* collection, ast_t* loop_body) {
    ast_t* foreach = ast(arena, AST_FOREACH, 3);
    assert(var->tag == AST_VARREF);
    assert(loop_body->tag == AST_BLOCK);
    foreach->as.items[0] = var;
    foreach->as.items[1] = collection;
    foreach->as.items[2] = loop_body;
    return foreach;
}

inline static ast_t* ast_return(arena_t* arena, ast_t* return_expr) {
    ast_t* node = ast(arena, AST_RETURN, 1);
    node->as.items[0] = return_expr;
    return node;
}

inline static ast_t* ast_function_declaration(arena_t* arena, ast_t* type, srcref_t name, ast_t* arglist, ast_flags_t flags) {
    ast_t* node = ast(arena, AST_FUNDECL, 3);
    assert(arglist->tag == AST_ARGLIST);
    node->as.items[0] = type;
    node->as.items[1] = ast_srcref(arena, name);
    node->as.items[2] = arglist;
    node->as.items[3] = ast_flags(arena, flags);
    return node;
}

inline static ast_t* ast_function_definition(arena_t* arena, ast_t* fundecl, ast_t* body) {
    ast_t* node = ast(arena, AST_FUNDEF, 2);
    assert(fundecl->tag == AST_FUNDECL);
    assert(body->tag == AST_BLOCK);
    node->as.items[0] = fundecl;
    node->as.items[1] = body;
    return node;
}

#endif // AST_BUILDER_H_