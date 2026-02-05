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
#include <shared/sh_diag.h>

typedef enum ast_tag_t {

    AST_UNDEFINED       = 0x00,

    AST__BEGIN_VALUES,

    AST_INT,
    AST_FLOAT,
    AST_BOOL,
    AST_CHAR,
    AST_STRING,
    AST_SYMBOL,
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

    AST_ARRAY,
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

    AST__COUNT

} ast_tag_t;

typedef struct ast_t ast_t;

typedef struct ast_diags_t {
    diag_kind_t kind;
    int         size;
    diag_t**    list;
} ast_diags_t;

typedef struct ast_value_t {
    srcref_t    srcref;
    union {
        int         _int;
        bool        _bool;
        float       _float;
        char        _char;
        uint32_t    _flags; 
    } as;
} ast_value_t;

typedef struct ast_t {
    ast_tag_t           tag;
    int                 size;
    ast_diags_t*        diagnostics;
    union {
        ast_t**         items;
        ast_value_t     value;
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

srcref_t    ast_aggregate_srcref(ast_t* n);
srcref_t    ast_try_get_name(ast_t* n);
ast_t*      ast_try_get(ast_t* n, ast_tag_t tag);

bool        _ast_attach_diag(arena_t* allocator,
                             ast_t* node,
                             diag_kind_t kind,
                             diphrase_t phrase,
                             dimsg_t* msg,
                             size_t msglen);

#define ast_attach_error(ARENA, NODE, PHRASE, ...)  \
    _ast_attach_diag((ARENA),                       \
        (NODE), DIAG_ERROR, (PHRASE),               \
        VA_ARRAY(dimsg_t, __VA_ARGS__),             \
        VA_ARRAYLEN(dimsg_t, __VA_ARGS__))

#define ast_attach_warning(ARENA, NODE, PHRASE, ...)\
    _ast_attach_diag((ARENA),                       \
        (NODE), DIAG_WARNING, (PHRASE),             \
        VA_ARRAY(dimsg_t, __VA_ARGS__),             \
        VA_ARRAYLEN(dimsg_t, __VA_ARGS__))

const char* ast_tag_to_string(ast_tag_t tag);

bool ast_tag_is_unop(ast_tag_t tag);
bool ast_is_unop(ast_t* node);
bool ast_tag_is_binop(ast_tag_t tag);
bool ast_is_binop(ast_t* node);
bool ast_tag_is_value(ast_tag_t tag);
bool ast_is_value(ast_t* node);
bool ast_tag_is_list(ast_tag_t tag);
bool ast_is_list(ast_t* node);
bool ast_is_valid_else_block(ast_t* node);
bool ast_tag_is_highlevel(ast_tag_t tag);

/////////////// BUILDERS /////////////////

ast_t* ast_int(arena_t* arena, int value, srcref_t ref);
ast_t* ast_float(arena_t* arena, float value, srcref_t ref);
ast_t* ast_bool(arena_t* arena, bool value, srcref_t ref);
ast_t* ast_char(arena_t* arena, char value, srcref_t ref);
ast_t* ast_string(arena_t* arena, srcref_t value);
ast_t* ast_symbol(arena_t* arena, srcref_t value);
ast_t* ast_flags(arena_t* arena, uint32_t flags, srcref_t ref);

int64_t ast_find_flags(ast_t* node, int depth);

bool ast_is_exported(ast_t* node);
bool ast_is_imported(ast_t* node);

#define AST_VARREF_SYMBOL 0

ast_t* ast_variable_reference(arena_t* arena, srcref_t ref);

#define AST_VARDECL_TYDESCR 0
#define AST_VARDECL_VARREF  1

ast_t* ast_variable_declaration(arena_t* arena, ast_t* type, ast_t* varref);

ast_t* ast_array(arena_t* arena);
bool   ast_array_append(arena_t* arena, ast_t* array, ast_t* value);

ast_t* ast_arglist(arena_t* arena);
bool   ast_arglist_append(arena_t* arena, ast_t* args, ast_t* value);

ast_t* ast_block(arena_t* arena);
bool   ast_block_append(arena_t* arena, ast_t* block, ast_t* value);

#define AST_FUNCALL_SYMBOL  0
#define AST_FUNCALL_ARGLIST 1

ast_t* ast_function_call(arena_t* arena, srcref_t name, ast_t* arglist);

#define AST_UNAOP_INNER 0

ast_t* ast_unary_operation(arena_t* arena, ast_tag_t op, ast_t* inner);

#define AST_BINOP_LEFT  0
#define AST_BINOP_RIGHT 1

ast_t* ast_binary_operation(arena_t* arena, ast_tag_t op, ast_t* left, ast_t* right);

#define AST_TYDESCR_SYMBOL  0
#define AST_TYDESCR_ARGLIST 1

ast_t* ast_type_descriptor(arena_t* arena, srcref_t name, ast_t* arglist);

#define AST_ASSIGN_LEFT  0
#define AST_ASSIGN_RIGHT 1

ast_t* ast_assignment(arena_t* arena, ast_t* left, ast_t* right);

#define AST_IFCHAIN_COND   0
#define AST_IFCHAIN_IFTRUE 1
#define AST_IFCHAIN_IFNEXT 2

ast_t* ast_if_chain(arena_t* arena, ast_t* condition, ast_t* if_true, ast_t* if_next);

#define AST_FOREACH_VAR  0
#define AST_FOREACH_COLL 1
#define AST_FOREACH_BODY 2

ast_t* ast_foreach(arena_t* arena, ast_t* var, ast_t* collection, ast_t* loop_body);

#define AST_RETURN_EXPR 0

ast_t* ast_return(arena_t* arena, ast_t* return_expr);

#define AST_FUNSIGN_TYDESCR  0
#define AST_FUNSIGN_SYMBOL   1
#define AST_FUNSIGN_ARGLIST  2
#define AST_FUNSIGN_FFI      3

#define AST_FUNSIGN_FFI_FLAG_IMPORT 0x01
#define AST_FUNSIGN_FFI_FLAG_EXPORT 0x02

ast_t* ast_function_signature(arena_t* arena, ast_t* type, srcref_t name, ast_t* arglist, ast_t* flags);

#define AST_FUNDEFN_FUNSIGN 0
#define AST_FUNDEFN_BODY    1

ast_t* ast_function_definition(arena_t* arena, ast_t* funsign, ast_t* body);

#endif // AST_EXPR_H_