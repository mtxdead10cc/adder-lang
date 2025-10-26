#ifndef GVM_PARSER_TYPES_H_
#define GVM_PARSER_TYPES_H_

#include <adrcom/shared/co_types.h>

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

typedef enum pa_result_type_t {
    PAR_NOTHING,
    PAR_AST_NODE,
    PAR_BUILD_ERROR
} pa_result_type_t;

typedef struct pa_result_t {
    pa_result_type_t    type; 
    void*               data;
    bool                group_expression; // exprs like "-(a + b)"
} pa_result_t;

#endif // GVM_PARSER_TYPES_H_