#ifndef GVM_LEXER_H_
#define GVM_LEXER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include "compiler/gvm_types.h"

#define IS_LETTER(C) (((C) >= 'a' && (C) <= 'z') || ((C) >= 'A' && (C) <= 'Z'))
#define IS_NUMBER(C)  ((C) >= '0' && (C) <= '9')

#define IS_WHITESPACE(C) (  \
       ((C) == '\n')        \
    || ((C) == ' ')         \
    || ((C) == '\t')        \
    || ((C) == '\r') )

#define IS_SEPARATOR(C)  ( ((C) == ',') || ((C) == ';') )

#define IS_SYMBOLIC(C) (((C) > 32) && ((C) < 127))

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
    } else if( IS_SYMBOLIC(character) ) {
        lexeme |= LCAT_SYMBOLIC;
    }

    return lexeme;
}

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