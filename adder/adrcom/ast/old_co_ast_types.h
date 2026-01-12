#ifndef GVM_AST_TYPES_H_
#define GVM_AST_TYPES_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <adrcom/shared/co_types.h>

typedef enum ast_value_type_t {
    AST_VALUE_NONE,
    AST_VALUE_INT,
    AST_VALUE_BOOL,
    AST_VALUE_CHAR,
    AST_VALUE_FLOAT
} ast_value_type_t;

typedef enum ast_node_type_t {
    AST_VALUE,
    AST_ARRAY,
    AST_IF_CHAIN,
    AST_FOREACH,
    AST_BINOP,
    AST_UNOP,
    AST_ASSIGN,
    AST_TYANNOT,
    AST_VAR_REF,
    AST_FUN_EXDECL,
    AST_FUN_DECL,
    AST_FUN_CALL,
    AST_RETURN,
    AST_BREAK,
    AST_BLOCK,
    AST_ARGLIST
} ast_node_type_t;

// ordered by precedence
typedef enum ast_binop_type_t {
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
    AST_BIN_AND
} ast_binop_type_t;

typedef enum ast_unop_type_t {
    AST_UN_NOT,
    AST_UN_NEG
} ast_unop_type_t;


typedef struct ast_node_t ast_node_t;

typedef struct ast_value_t {
    ast_value_type_t type;
    union {
        int         _int;
        char        _char;
        float       _float;
        bool        _bool;
    } u;
} ast_value_t;

typedef struct ast_annot_t ast_annot_t;
typedef struct ast_annot_t { // type annotation
    srcref_t       name;
    size_t         childcount;
    ast_annot_t**  children;
} ast_annot_t;

typedef struct ast_array_t {
    size_t           count;
    ast_node_t**     content;
} ast_array_t;

typedef struct ast_block_t {
    size_t       count;
    ast_node_t** content;
} ast_block_t;

typedef struct ast_arglist_t {
    size_t       count;
    ast_node_t** content;
} ast_arglist_t;

typedef struct ast_tyannot_t {
    ast_annot_t* type;
    ast_node_t*  expr;
} ast_tyannot_t;

typedef struct ast_varref_t {
    srcref_t        name;
} ast_varref_t;

typedef struct ast_if_t {
    ast_node_t* cond;
    ast_node_t* iftrue;
    ast_node_t* next; // else if or else
} ast_if_t;

typedef struct ast_foreach_t {
    ast_node_t* vardecl;    // this should be an ast_vardecl_t
    ast_node_t* collection; // var-ref or array-decl
    ast_node_t* during;     // block or single instruction
} ast_foreach_t;

typedef struct ast_binop_t {
    ast_binop_type_t type;
    ast_node_t*      left;
    ast_node_t*      right;
} ast_binop_t;

typedef struct ast_unop_t {
    ast_unop_type_t  type;
    ast_node_t*      inner;
} ast_unop_t;

typedef struct ast_funexdecl_t {
    srcref_t            name;
    ast_node_t*         argspec;
} ast_funexdecl_t;

typedef struct ast_fundecl_t {
    srcref_t            name;
    ast_node_t*         argspec;
    ast_node_t*         body;
    bool                exported;
} ast_fundecl_t;

typedef struct ast_funcall_t {
    srcref_t    name;
    ast_node_t* args;
} ast_funcall_t;

typedef struct ast_assign_t {
    ast_node_t* left_var;
    ast_node_t* right_value;
} ast_assign_t;

typedef struct ast_return_t {
    ast_node_t* result;
} ast_return_t;
 
typedef struct ast_node_t {
    ast_node_type_t     type;
    srcref_t            ref;
    union {
        ast_value_t     n_value;
        ast_varref_t    n_varref;
        ast_array_t     n_array;
        ast_tyannot_t   n_tyannot;
        ast_block_t     n_block;
        ast_arglist_t   n_args;
        ast_if_t        n_if;
        ast_fundecl_t   n_fundecl;
        ast_funexdecl_t n_funexdecl;
        ast_binop_t     n_binop;
        ast_unop_t      n_unop;
        ast_assign_t    n_assign;
        ast_return_t    n_return;
        ast_foreach_t   n_foreach;
        ast_funcall_t   n_funcall;
    } u;
} ast_node_t;

#endif // GVM_AST_TYPES_H_