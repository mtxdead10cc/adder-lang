#ifndef GVM_LEXER_H_
#define GVM_LEXER_H_

#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

inline static bool is_numeric(char c) {
    return c <= '9' && c >= '0';
}

inline static bool is_whitespace(char c) {
    return c == ' '
        || c == '\t'
        || c == '\n';
}

typedef enum lex_type_t {
    L_LETTER,
    L_NUMBER,
    L_SPECIAL,
    L_DASH,
    L_SLASH,
    L_DOT,
    L_GROUP_OPEN,
    L_GROUP_CLOSE,
    L_QUOTE,
    L_WHITESPACE,
    L_NEWLINE,
    L_UNKNOWN,
    L_EOF,
    L_LEXEME_COUNT
} lex_type_t;

typedef struct lexer_t {
    char* filepath;
    char* buffer;
    size_t buffer_size;
    size_t index;
} lexer_t;

inline static lex_type_t scan(char c) {
    switch(c) {
        case '"':
            return L_QUOTE;
        case '-':
            return L_DASH;
        case '.':
            return L_DOT;
        case '\n':
            return L_NEWLINE;
        case ' ':
        case '\t':
            return L_WHITESPACE;
        case '(':
        case '{':
        case '<':
            return L_GROUP_OPEN;
        case ')':
        case '}':
        case '>':
            return L_GROUP_CLOSE;
        case '/':
            return L_SLASH;
        case '#':
        case '+':
        case '*':
        case '&':
        case '|':
        case '_':
        case ':':
        case '=':
        case ';':
            return L_SPECIAL;
        default: {
            if( is_numeric(c) ) {
                return L_NUMBER;
            } else if (isalpha(c)) {
                return L_LETTER;
            } else {
                return L_UNKNOWN;
            }
        }
    }
}


inline static void lexer_init(lexer_t* lexer, char* text, size_t size, char* filepath) {
    lexer->buffer = text;
    lexer->buffer_size = size;
    lexer->filepath = filepath;
    lexer->index = 0;
}

inline static lex_type_t lexer_get_next(lexer_t* lexer, srcref_t* next) {

    next->filepath = lexer->filepath;
    next->source = lexer->buffer;

    if( lexer->index >= lexer->buffer_size ) {
        next->idx_end = lexer->buffer_size - 1;
        next->idx_start = lexer->buffer_size - 1;
        return L_EOF;
    }

    lex_type_t type = scan(lexer->buffer[lexer->index]);
    next->idx_start = lexer->index;

    do {
        lexer->index++;
    } while( type == scan(lexer->buffer[lexer->index]) );

    next->idx_end = lexer->index;
    return type;
}



#endif // GVM_LEXER_H_