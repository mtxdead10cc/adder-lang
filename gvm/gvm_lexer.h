#ifndef GVM_LEXER_H_
#define GVM_LEXER_H_

bool is_numeric(char c) {
    return c <= '9' && c >= '0';
}

bool is_alpha(char c) {
    return (c <= 'Z' && c >= 'A')
        || (c <= 'z' && c >= 'a');
}

bool is_whitespace(char c) {
    return c == ' '
        || c == '\t'
        || c == '\n';
}

#define _MIN(a, b) ((a) < (b) ? (a) : (b))

typedef enum lexeme_t {
    L_LETTER,
    L_NUMBER,
    L_DASH,
    L_UNDERSCORE,
    L_DOT,
    L_LPAREN,
    L_RPAREN,
    L_QUOTE,
    L_COLON,
    L_PUND_SIGN,
    L_WHITESPACE,
    L_NEWLINE,
    L_UNKNOWN
} lexeme_t;

lexeme_t lex_scan(char c) {
    switch(c) {
        case ':':   return L_COLON;
        case '"':   return L_QUOTE;
        case '-':   return L_DASH;
        case '_':   return L_UNDERSCORE;
        case '.':   return L_DOT;
        case '\n':  return L_NEWLINE;
        case ' ':   return L_WHITESPACE;
        case '\t':  return L_WHITESPACE;
        case '#':   return L_PUND_SIGN;
        case '(':   return L_LPAREN;
        case ')':   return L_RPAREN;
        default: {
            if( is_numeric(c) ) {
                return L_NUMBER;
            } else if (is_alpha(c)) {
                return L_LETTER;
            } else {
                return L_UNKNOWN;
            }
        }
    }
}

bool lex_is_separator(lexeme_t lex) {
    return lex == L_NEWLINE
        || lex == L_WHITESPACE;
}

bool lex_is_valid_number_part(lexeme_t lex) {
    return lex == L_NUMBER
        || lex == L_DOT;
}

bool lex_is_valid_symbol_part(lexeme_t lex) {
    return lex == L_LETTER
        || lex == L_NUMBER
        || lex == L_DASH
        || lex == L_UNDERSCORE;
}

#endif // GVM_LEXER_H_