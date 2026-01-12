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
    AST_SRCREF,
    AST_ARRAY,
    AST_FLAGS,

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

    AST_TYDESCR,    // (descr <type-name> <type-args>)
    AST_VARDECL,    // (vardecl (type-desc ...) (srcref <name>))
    AST_VARREF,     // (varref (srcref <name>))
    AST_FUNDECL,    // (fundecl (type-desc ...) (srcref <name>) (arglist (annot <arg> <type>) ...))
    AST_FUNDEF,     // (fundef (fundecl ...) (block ...))
    AST_FUNCALL,    // (funcall (srcref <name>) (arglist <arg> ...))
    AST_FOREACH,    // (foreach (vardecl|varref ...) )
    AST_BLOCK,
    AST_ARGLIST,
    AST_IFCHAIN,
    AST_RETURN,
    AST_ASSIGN,

    AST__END_HIGH_LEVEL,

} ast_tag_t;


typedef struct ast_t ast_t;

typedef enum ast_flags_t {
    AST_FLAG_NOFLAG = 0x0000,
    AST_FLAG_IMPORT = 0x0001,
    AST_FLAG_EXPORT = 0x0002
} ast_flags_t;

typedef struct ast_t {
    ast_tag_t       tag;
    int             size;
    union {
        ast_t**     items;
        srcref_t    srcref;
        ast_flags_t flags;
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
ast_flags_t ast_try_get_flags(ast_t* n);

const char* ast_tag_to_string(ast_tag_t tag);
void ast_print(ast_t* node);

typedef struct json_value_t json_value_t;

json_value_t* ast_to_json(ast_t* node);

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

#endif // AST_EXPR_H_