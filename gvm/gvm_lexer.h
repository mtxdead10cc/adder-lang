#ifndef GVM_LEXER_H_
#define GVM_LEXER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

typedef enum lex_category_t {
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
    LCAT_GREATER_THAN   = 0x00000400,

    LCAT_NUMBER         = 0x00010000,
    LCAT_LETTER         = 0x00020000,
    LCAT_SPACE          = 0x00040000,
    LCAT_SCOPE_START    = 0x00080000,
    LCAT_SCOPE_END      = 0x00100000,
    LCAT_SEPARATOR      = 0x00200000,
    LCAT_SYMBOL         = 0x00400000

} lex_category_t;

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

inline static lex_category_t lexer_scan(char character) {
    lex_category_t category = LCAT_NONE;

    switch (character) {
        case '\n':  category |= LCAT_NEWLINE;       break;
        case '/':   category |= LCAT_SLASH;         break;
        case '.':   category |= LCAT_DOT;           break;
        case '-':   category |= LCAT_MINUS;         break;
        case '"':   category |= LCAT_QUOTE;         break;
        case ',':   category |= LCAT_COMMA;         break;
        case ';':   category |= LCAT_SEMI_COLON;    break;
        case '_':   category |= LCAT_UNDERSCORE;    break;
        case '=':   category |= LCAT_EQUAL;         break;
        case '<':   category |= LCAT_LESS_THAN;     break;
        case '>':   category |= LCAT_GREATER_THAN;  break;
        default:                                    break;
    }

    if( IS_LETTER(character) ) {
        category |= LCAT_LETTER;
    } else if( IS_NUMBER(character) ) {
        category |= LCAT_NUMBER;
    } else if( IS_WHITESPACE(character) ) {
        category |= LCAT_SPACE;
    } else if( IS_SCOPE_START(character) ) {
        category |= LCAT_SCOPE_START;
    } else if( IS_SCOPE_END(character) ) {
        category |= LCAT_SCOPE_END;
    }
    
    if( character >= 0x20 && character <= 0x7E  ) {
        category |= LCAT_SYMBOL;
    }

    return category;
}

typedef enum lex_ptype_t {
    LP_IS,
    LP_IS_NOT
} lex_ptype_t;

typedef struct lex_predicate_t {
    lex_category_t  category;
    lex_ptype_t     type;
} lex_predicate_t;


inline static lex_predicate_t lp_is(lex_category_t cat) {
    return (lex_predicate_t) {
        .category = cat,
        .type = LP_IS
    };
}

inline static lex_predicate_t lp_is_not(lex_category_t cat) {
    return (lex_predicate_t) {
        .category = cat,
        .type = LP_IS_NOT
    };
}

inline static bool lexer_match(lex_predicate_t predicate, lex_category_t actual) {
    switch(predicate.type) {
        case LP_IS:     return (predicate.category & actual) > 0;
        case LP_IS_NOT: return (predicate.category & actual) == 0;
        default:        return false;
    }
}



#endif // GVM_LEXER_H_