#ifndef GVM_COMPILER_TYPES_H_
#define GVM_COMPILER_TYPES_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "sh_types.h"

typedef enum lexeme_t {
    LCAT_NONE           = 0x00000000,

    LCAT_NEWLINE        = 0x00000001,
    LCAT_SLASH          = 0x00000002,
    LCAT_DOT            = 0x00000004,
    LCAT_MINUS          = 0x00000008,
    LCAT_QUOTE          = 0x00000010,
    LCAT_COMMA          = 0x00000020,
    LCAT_SEMI_COLON     = 0x00000040,
    LCAT_UNDERSCORE     = 0x00000080,
    LCAT_EQUAL          = 0x00000100,
    LCAT_LESS_THAN      = 0x00000200,
    LCAT_OPEN_PAREN     = 0x00000400,
    LCAT_OPEN_CURLY     = 0x00000800,
    LCAT_OPEN_SBRACKET  = 0x00001000,
    LCAT_GREATER_THAN   = 0x00002000,
    LCAT_CLOSE_PAREN    = 0x00004000,
    LCAT_CLOSE_CURLY    = 0x00008000,
    LCAT_CLOSE_SBRACKET = 0x00010000,
    LCAT_BANG           = 0x00020000,
    LCAT_POUND          = 0x00040000,
    
    LCAT_NUMBER         = 0x04000000,
    LCAT_LETTER         = 0x08000000,
    LCAT_SPACE          = 0x10000000,
    LCAT_SEPARATOR      = 0x20000000,
    LCAT_SYMBOLIC       = 0x40000000

} lexeme_t;

typedef enum token_type_t {
    TT_NOTHING,
    TT_INITIAL,
    TT_SPACE,
    TT_COMMENT,
    TT_SYMBOL,
    TT_NUMBER,
    TT_BOOLEAN,
    TT_STRING,
    TT_ARROW,
    TT_ASSIGN,
    TT_KW_IF,
    TT_KW_ELSE,
    TT_KW_FOR,
    TT_KW_IN,
    TT_KW_RETURN,
    TT_KW_BREAK,
    TT_KW_FUN_DEF,
    TT_CMP_EQ,
    TT_CMP_NEQ,
    TT_CMP_GT_EQ,
    TT_CMP_LT_EQ,
    TT_CMP_GT,
    TT_CMP_LT,
    TT_OPEN_PAREN,
    TT_CLOSE_PAREN,
    TT_OPEN_CURLY,
    TT_CLOSE_CURLY,
    TT_OPEN_SBRACKET,
    TT_CLOSE_SBRACKET,
    TT_UNOP_NOT,
    TT_BINOP_AND,
    TT_BINOP_OR,
    TT_BINOP_MUL,
    TT_BINOP_DIV,
    TT_BINOP_MOD,
    TT_BINOP_PLUS,
    TT_BINOP_MINUS,
    TT_HASH_SIGN,
    TT_SEPARATOR,
    TT_STATEMENT_END,
    TT_IMPORT,
    TT_EXPORT,
    TT_FINAL
} token_type_t;

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

typedef struct srcref_t {
    char*   source;
    size_t  idx_start;
    size_t  idx_end;
} srcref_t;

#define TRACE_MSG_MAX_LEN 256

typedef enum trace_msg_type_t {
    TM_NONE,
    TM_INFO,
    TM_WARNING,
    TM_ERROR,
    TM_OUT_OF_MEMORY,
    TM_INTERNAL_ERROR
} trace_msg_type_t;

typedef struct trace_msg_t {
    trace_msg_type_t type;
    char*       source_path;
    srcref_t    source_location;
    size_t      length;
    char        message[TRACE_MSG_MAX_LEN];
} trace_msg_t;

typedef struct trace_t {
    size_t       message_count;
    size_t       message_capacity;
    trace_msg_t* messages;
    char*        current_source_path;
    size_t       error_count;
} trace_t;

typedef enum lex_ptype_t {
    LP_IS,
    LP_IS_NOT
} lex_ptype_t;

typedef struct lex_predicate_t {
    lexeme_t        lexeme;
    lex_ptype_t     type;
} lex_predicate_t;

typedef struct token_t {
    token_type_t type;
    srcref_t ref;
} token_t;

typedef struct token_collection_t {
    token_t*    tokens;
    size_t      capacity;
    size_t      count;
} token_collection_t;

typedef struct parser_t {
    token_collection_t  collection;
    size_t              cursor;
    trace_t*            trace;
    arena_t*            arena;
} parser_t;

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

#define LANG_TYPENAME_VOID      "void"
#define LANG_TYPENAME_BOOL      "bool"
#define LANG_TYPENAME_FLOAT     "float"
#define LANG_TYPENAME_INT       "int"
#define LANG_TYPENAME_STRING    "string"
#define LANG_TYPENAME_ARRAY     "array"
#define LANG_TYPENAME_CHAR      "char"

#endif // GVM_COMPILER_TYPES_H_
