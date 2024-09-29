#ifndef GVM_LEXER_H_
#define GVM_LEXER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

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

#define IS_LETTER(C) (((C) >= 'a' && (C) <= 'z') || ((C) >= 'A' && (C) <= 'Z'))
#define IS_NUMBER(C)  ((C) >= '0' && (C) <= '9')

#define IS_WHITESPACE(C) (  \
       ((C) == '\n')        \
    || ((C) == ' ')         \
    || ((C) == '\t')        \
    || ((C) == '\r') )

#define IS_SCOPE_START(C) (((C) == '{') \
                        || ((C) == '[') \
                        || ((C) == '(') \
                        || ((C) == '<') )

#define IS_SCOPE_END(C) (((C) == '}')\
                      || ((C) == ']')\
                      || ((C) == ')')\
                      || ((C) == '>'))

#define IS_SEPARATOR(C)  ( ((C) == ',') || ((C) == ';') )

inline static lexeme_t lexer_scan_char_type(char character) {
    switch (character) {
        case '\n':  return LCAT_NEWLINE;
        case '/':   return LCAT_SLASH;
        case '.':   return LCAT_DOT;
        case '-':   return LCAT_MINUS;
        case '"':   return LCAT_QUOTE;
        case ',':   return LCAT_COMMA;
        case ';':   return LCAT_SEMI_COLON;
        case '_':   return LCAT_UNDERSCORE;
        case '=':   return LCAT_EQUAL;
        case '<':   return LCAT_LESS_THAN;
        case '(':   return LCAT_OPEN_PAREN;
        case '{':   return LCAT_OPEN_CURLY;
        case '[':   return LCAT_OPEN_SBRACKET;
        case '>':   return LCAT_GREATER_THAN;
        case ')':   return LCAT_CLOSE_PAREN;
        case '}':   return LCAT_CLOSE_CURLY;
        case ']':   return LCAT_CLOSE_SBRACKET;
        default:    return LCAT_NONE;
    }
}

inline static lexeme_t lexer_scan(char character) {

    lexeme_t lexeme = lexer_scan_char_type(character);

    if( IS_LETTER(character) ) {
        lexeme |= LCAT_LETTER;
    } else if( IS_NUMBER(character) ) {
        lexeme |= LCAT_NUMBER;
    } else if( IS_WHITESPACE(character) ) {
        lexeme |= LCAT_SPACE;
    } else if( lexeme != LCAT_NONE ) {
        lexeme |= LCAT_SYMBOLIC;
    }

    return lexeme;
}

typedef enum lex_ptype_t {
    LP_IS,
    LP_IS_NOT
} lex_ptype_t;

typedef struct lex_predicate_t {
    lexeme_t        lexeme;
    lex_ptype_t     type;
} lex_predicate_t;


inline static lex_predicate_t lp_is(lexeme_t cat) {
    return (lex_predicate_t) {
        .lexeme = cat,
        .type = LP_IS
    };
}

inline static lex_predicate_t lp_is_not(lexeme_t cat) {
    return (lex_predicate_t) {
        .lexeme = cat,
        .type = LP_IS_NOT
    };
}

inline static bool lexer_match(lex_predicate_t predicate, lexeme_t actual) {
    switch(predicate.type) {
        case LP_IS:     return (predicate.lexeme & actual) > 0;
        case LP_IS_NOT: return (predicate.lexeme & actual) == 0;
        default:        return false;
    }
}



#endif // GVM_LEXER_H_