#ifndef GVM_COMPILER_TYPE_H_
#define GVM_COMPILER_TYPE_H_

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
    
    LCAT_NUMBER         = 0x01000000,
    LCAT_LETTER         = 0x02000000,
    LCAT_SPACE          = 0x04000000,
    LCAT_SEPARATOR      = 0x08000000,
    LCAT_SYMBOLIC       = 0x10000000

} lexeme_t;

typedef enum token_type_t {
    TT_NOTHING              = 0x00000000,
    TT_SPACE                = 0x00000001,
    TT_COMMENT              = 0x00000002,
    TT_SYMBOL               = 0x00000004,
    TT_NUMBER               = 0x00000008,
    TT_BOOLEAN              = 0x00000010,
    TT_SEPARATOR            = 0x00000020,
    TT_STATEMENT_END        = 0x00000040,
    TT_STRING               = 0x00000080,
    TT_ARROW                = 0x00000100,
    TT_ASSIGN               = 0x00000200,
    TT_KW_IF                = 0x00000400,
    TT_KW_ELSE              = 0x00000800,
    TT_KW_FOR               = 0x00001000,
    TT_KW_RETURN            = 0x00002000,
    TT_KW_FUN_DEF           = 0x00004000,
    TT_KW_AND               = 0x00008000,
    TT_KW_OR                = 0x00010000,
    TT_KW_NOT               = 0x00020000,
    TT_CMP_EQ               = 0x00040000,
    TT_CMP_GT_EQ            = 0x00080000,
    TT_CMP_LT_EQ            = 0x00100000,
    TT_LT_OR_OPEN_ABRACKET  = 0x00200000,
    TT_GT_OR_CLOSE_ABRACKET = 0x00400000,
    TT_OPEN_PAREN           = 0x00800000,
    TT_CLOSE_PAREN          = 0x01000000,
    TT_OPEN_CURLY           = 0x02000000,
    TT_CLOSE_CURLY          = 0x04000000,
    TT_OPEN_SBRACKET        = 0x08000000,
    TT_CLOSE_SBRACKET       = 0x10000000,
    TT_INITIAL              = 0x20000000,
    TT_FINAL                = 0x40000000
} token_type_t;


typedef enum ast_node_type_t {
    AST_VALUE,
    AST_ARRAY,
    AST_IF,
    AST_IF_ELSE,
    AST_FOREACH,
    AST_BINOP,
    AST_UNOP,
    AST_ASSIGN,
    AST_VAR_DECL,
    AST_VAR_REF,
    AST_FUN_DECL,
    AST_FUN_CALL,
    AST_RETURN,
    AST_BREAK,
    AST_BLOCK
} ast_node_type_t;

typedef enum ast_value_type_t {
    AST_VALUE_TYPE_NONE,
    AST_VALUE_TYPE_NUMBER,
    AST_VALUE_TYPE_BOOL,
    AST_VALUE_TYPE_STRING
} ast_value_type_t;

typedef enum ast_binop_type_t {
    AST_BIN_ADD,
    AST_BIN_SUB,
    AST_BIN_MUL,
    AST_BIN_DIV,
    AST_BIN_AND,
    AST_BIN_OR,
    AST_BIN_XOR,
    AST_BIN_EQ,
    AST_BIN_LT_EQ,
    AST_BIN_GT_EQ,
    AST_BIN_LT,
    AST_BIN_GT
} ast_binop_type_t;

typedef enum ast_unop_type_t {
    AST_UN_NOT,
    AST_UN_NEG
} ast_unop_type_t;

typedef enum build_rescode_t {
    R_OK,
    R_ER_UNEXPECTED_TOKEN,
    R_ER_HOST_OUT_OF_MEMORY,
    R_ER_INVALID_STATE,
} build_rescode_t;


typedef struct build_result_t {
    build_rescode_t code;
    srcref_t ref;
    union {
        struct {
            char ignore;
        } nothing;
        struct {
            token_type_t token_expected_mask;
            token_type_t token_actual;
        } unexp_token;
    } info;
} build_result_t;


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
    build_result_t      result;
} parser_t;


typedef struct ast_node_t ast_node_t;

typedef struct ast_value_t {
    ast_value_type_t type;
    union {
        float       _number;
        bool        _bool;
        srcref_t    _string;
    } u;
} ast_value_t;

typedef struct ast_array_t {
    ast_value_type_t type;
    size_t           count;
    ast_node_t**     content;
} ast_array_t;

typedef struct ast_block_t {
    size_t       count;
    ast_node_t** content;
} ast_block_t;

typedef struct ast_vardecl_t {
    srcref_t         name;
    ast_value_type_t type;
} ast_vardecl_t;

typedef struct ast_varref_t {
    srcref_t        name;
} ast_varref_t;

typedef struct ast_if_t {
    ast_node_t* cond;
    ast_node_t* iftrue;
} ast_if_t;

typedef struct ast_ifelse_t {
    ast_node_t* cond;
    ast_node_t* iftrue;
    ast_node_t* iffalse;
} ast_ifelse_t;

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

typedef struct ast_fundecl_t {
    ast_value_type_t rettype;
    srcref_t    name;    
    ast_node_t* args;       // block, single instruction or null
    ast_node_t* body;       // block, single instruction or null
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
    union {
        ast_value_t     n_value;
        ast_varref_t    n_varref;
        ast_array_t     n_array;
        ast_vardecl_t   n_vardecl;
        ast_block_t     n_block;
        ast_if_t        n_if;
        ast_ifelse_t    n_ifelse;
        ast_fundecl_t   n_fundecl;
        ast_binop_t     n_binop;
        ast_unop_t      n_unop;
        ast_assign_t    n_assign;
        ast_return_t    n_return;
        ast_foreach_t   n_foreach;
        ast_funcall_t   n_funcall;
    } u;
} ast_node_t;

#endif // GVM_COMPILER_TYPE_H_