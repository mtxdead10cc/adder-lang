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

typedef enum ast_expr_tag_t {

    ATAG_VALUE_NONE,
    ATAG_VALUE_INT,
    ATAG_VALUE_BOOL,
    ATAG_VALUE_CHAR,
    ATAG_VALUE_FLOAT,
    ATAG_UNOP_NOT,
    ATAG_UNOP_NEG,
    ATAG_BIN_MUL,
    ATAG_BIN_DIV,
    ATAG_BIN_MOD,
    ATAG_BIN_ADD,
    ATAG_BIN_SUB,
    ATAG_BIN_XOR,
    ATAG_BIN_LT,
    ATAG_BIN_GT,
    ATAG_BIN_LT_EQ,
    ATAG_BIN_GT_EQ,
    ATAG_BIN_EQ,
    ATAG_BIN_NEQ,
    ATAG_BIN_OR,
    ATAG_BIN_AND,
    ATAG_ARRAY,
    ATAG_IF_CHAIN,
    ATAG_FOREACH,
    ATAG_ASSIGN,
    ATAG_TYANNOT,
    ATAG_VAR_REF,
    ATAG_FUN_EXDECL,
    ATAG_FUN_DECL,
    ATAG_FUN_CALL,
    ATAG_ARGLIST,
    ATAG_RETURN,
    ATAG_BLOCK,
    ATAG_TYPE_VOID,
    ATAG_TYPE_FLOAT,
    ATAG_TYPE_INT,
    ATAG_TYPE_CHAR,
    ATAG_TYPE_BOOL,
    ATAG_TYPE_SOMETIMES,
    ATAG_TYPE_ALWAYS,
    ATAG_TYPE_LIST,
    ATAG_TYPE_FUNC,
    ATAG_TYPE_RETURN

} ast_expr_tag_t;

typedef enum ast_kvp_value_tag_t {
    AVAL_UNDEFINED,
    AVAL_EXPR,
    AVAL_LIST,
    AVAL_SRCREF,
    AVAL_INT,
    AVAL_FLOAT,
    AVAL_BOOL,
    AVAL_CHAR
} ast_kvp_value_tag_t;

typedef enum ast_key_t {
    AKEY_NAME,
    AKEY_SOURCE,
    AKEY_VALUE,
    AKEY_RETURN,
    AKEY_ARGS,
    AKEY_BODY,
    AKEY_IF_CONDITION,
    AKEY_IF_TRUE,
    AKEY_IF_NEXT,
    AKEY_LOOP_VAR_DECL,
    AKEY_LOOP_COLLECTION,
    AKEY_LOOP_BODY,
    AKEY_LEFT,
    AKEY_RIGHT,
    AKEY_INNER,
    AKEY_COUNT
} ast_key_t;

typedef struct ast_kvp_value_t ast_kvp_value_t;

typedef struct ast_list_t {
    int           size;
    int           capacity;
    ast_kvp_value_t*  content;
} ast_list_t;

typedef struct ast_expr_t ast_expr_t;

typedef struct ast_kvp_value_t {
    ast_kvp_value_tag_t     tag;
    union {
        ast_expr_t*     expr;
        ast_list_t      list;
        srcref_t        srcref;
        int             value_int;
        bool            value_bool;
        float           value_float;
        char            value_char;
    } u;
} ast_kvp_value_t;

typedef struct ast_expr_t {
    ast_expr_tag_t  tag;
    ast_kvp_value_t     map[AKEY_COUNT];
} ast_expr_t;

ast_kvp_value_t ast_value_int(int value);
ast_kvp_value_t ast_value_float(float value);
ast_kvp_value_t ast_value_bool(bool value);
ast_kvp_value_t ast_value_char(char value);
ast_kvp_value_t ast_value_string(srcref_t value);
ast_kvp_value_t ast_value_srcref(srcref_t value);
ast_kvp_value_t ast_value_expr(ast_expr_t* ptr);

ast_kvp_value_t ast_value_list(arena_t* allocator, int capacity);
bool        ast_value_list_append(arena_t* allocator, ast_kvp_value_t* list, ast_kvp_value_t value);
ast_kvp_value_t ast_value_list_get(ast_kvp_value_t* list, int index);

ast_expr_t  ast_expr(ast_expr_tag_t tag);
void        ast_expr_set(ast_expr_t* node, ast_key_t key, ast_kvp_value_t value);
ast_kvp_value_t ast_expr_get(ast_expr_t* node, ast_key_t key);

typedef struct json_value_t json_value_t;

json_value_t* ast_expr_to_json(ast_expr_t* node);

#endif // AST_EXPR_H_