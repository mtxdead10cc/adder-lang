#include "gvm_parser.h"
#include "gvm_types.h"
#include "gvm.h"
#include "gvm_config.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* PARSER * * * * * * * * 
*
* A pretty straight forward scanner, tokenizer and parser.
* The only thing to note is that when text is provided the
* parser copies the textbuffer and stores a reference in 
* parser_t (parser_text_t). The tokens generated via
* tokenize(...) only stores references to positions within
* the text buffer (as opposed to storing text snippets), 
* hence the need for parser_get_token_string_ptr(...), 
* parser_get_token_string_length(...), etc.
*
*/

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

lexeme_t scan(char c) {
    switch(c) {
        case ':':   return L_COLON;
        case '"':   return L_QUOTE;
        case '-':   return L_DASH;
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

gvm_result_t tokens_init(parser_tokens_t* tokens, int initial_capacity) {
    tokens->array = (token_t*) malloc(initial_capacity * sizeof(token_t));
    if( tokens->array == NULL ) {
        tokens->capacity = 0;
        tokens->size = 0;
        return RES_OUT_OF_MEMORY;
    }
    tokens->size = 0;
    tokens->capacity = initial_capacity;
    return RES_OK;
}

gvm_result_t tokens_push_token(parser_tokens_t* tokens, token_t token) {
    if( tokens->size >= tokens->capacity ) {
        int new_capacity = (int) ( (float) tokens->size * 1.3f );
        token_t* new_tokens = (token_t*) realloc(tokens->array, new_capacity * sizeof(token_t));
        if( new_tokens == NULL ) {
            return RES_OUT_OF_MEMORY;
        }
        tokens->array = new_tokens;
        tokens->capacity = new_capacity;
    }
    tokens->array[tokens->size] = token;
    tokens->size ++;
    return RES_OK;
}

gvm_result_t tokens_push_on_change(parser_tokens_t* tokens, token_type_t tt, int line, int column, int index) {
    token_type_t last = TT_UNKNOWN;
    gvm_result_t res = RES_OK;
    if( tokens->size > 0 ) {
        last = tokens->array[tokens->size - 1].type;
    }
    if( last != tt || last == TT_UNKNOWN ) {
        res = tokens_push_token(tokens, (token_t) {
            .src_column = column,
            .src_line = line,
            .src_index = index,
            .type = tt,
            .index = tokens->size
        });
    }
    return res;
}

typedef enum tok_state_t {
    TS_INIT,
    TS_QUOTED,
    TS_COMMENT,
    TS_NUMBER,
    TS_SYMBOL,
    TS_VEC2,
    TS_SEPARATOR,
    TS_COLON,
    TS_ERROR
} tok_state_t;

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
        || lex == L_DASH;
}

typedef struct state_update_t {
    tok_state_t state;
    tok_state_t prev_state;
    bool is_part_of_prev;
} state_update_t;

#define _MK_RESULT(PREV, IS_PREV, NEXT) ((state_update_t) {(NEXT), (PREV), (IS_PREV)})

state_update_t get_new_state(tok_state_t state, lexeme_t lexeme) {
    switch (state) {
        case TS_INIT: {
            switch(lexeme) {
                case L_LETTER:      return _MK_RESULT(state, false, TS_SYMBOL);
                case L_NUMBER:      return _MK_RESULT(state, false, TS_NUMBER);
                case L_NEWLINE:     return _MK_RESULT(state, false, TS_SEPARATOR);
                case L_WHITESPACE:  return _MK_RESULT(state, false, TS_SEPARATOR);
                case L_PUND_SIGN:   return _MK_RESULT(state, false, TS_COMMENT);
                case L_LPAREN:      return _MK_RESULT(state, false, TS_VEC2);
                default:            return _MK_RESULT(state, false, TS_ERROR);
            }
        } break;
        case TS_COMMENT: {
            if( lexeme != L_NEWLINE ) {
                return _MK_RESULT(state, false, TS_COMMENT);
            } else {
                return _MK_RESULT(state, false, TS_SEPARATOR);
            }
        } break;
        case TS_QUOTED: {
            if( lexeme != L_QUOTE ) {
                return _MK_RESULT(state, false, TS_QUOTED);
            } else {
                return _MK_RESULT(state, true, TS_SEPARATOR);
            }
        } break;
        case TS_NUMBER: {
            if( lex_is_valid_number_part(lexeme) ) {
                return _MK_RESULT(state, false, TS_NUMBER);
            } else if ( lex_is_separator(lexeme) ) {
                return _MK_RESULT(state, false, TS_SEPARATOR);
            } else {
                return _MK_RESULT(state, false, TS_ERROR);
            }
        } break;
        case TS_SYMBOL: {
            if( lex_is_valid_symbol_part(lexeme) ) {
                return _MK_RESULT(state, false, TS_SYMBOL);
            } else if ( lex_is_separator(lexeme) ) {
                return _MK_RESULT(state, false, TS_SEPARATOR);
            } else if ( lexeme == L_COLON ) {
                return _MK_RESULT(state, false, TS_COLON);
            } else {
                return _MK_RESULT(state, false, TS_ERROR);
            }
        } break;
        case TS_COLON: {
            if ( lex_is_separator(lexeme) ) {
                return _MK_RESULT(state, false, TS_SEPARATOR);
            } else if ( lexeme == L_PUND_SIGN ) {
                return _MK_RESULT(state, false, TS_COMMENT);
            } else {
                return _MK_RESULT(state, false, TS_ERROR);
            }
        } break;
        case TS_VEC2: {
            if( lexeme != L_RPAREN ) {
                return _MK_RESULT(state, false, TS_VEC2);
            } else {
                return _MK_RESULT(state, true, TS_SEPARATOR);
            }
        }
        case TS_SEPARATOR: {
            if ( lex_is_separator(lexeme) ) {
                return _MK_RESULT(state, false, TS_SEPARATOR);
            }
            switch (lexeme) {
                case L_DASH: return _MK_RESULT(state, false, TS_NUMBER);
                case L_NUMBER: return _MK_RESULT(state, false, TS_NUMBER);
                case L_PUND_SIGN: return _MK_RESULT(state, false, TS_COMMENT);
                case L_QUOTE: return _MK_RESULT(state, false, TS_QUOTED);
                case L_LETTER: return _MK_RESULT(state, false, TS_SYMBOL);
                case L_LPAREN: return _MK_RESULT(state, false, TS_VEC2);
                default: return _MK_RESULT(state, false, TS_ERROR);
            }
        }
        default: return _MK_RESULT(state, false, TS_ERROR);
    }
}

token_type_t state_to_type(tok_state_t state) {
    switch(state) {
        case TS_COMMENT: return TT_COMMENT;
        case TS_QUOTED: return TT_STRING;
        case TS_NUMBER: return TT_NUMBER;
        case TS_VEC2: return TT_VEC2;
        case TS_SYMBOL: return TT_SYMBOL;
        case TS_COLON: return TT_COLON;
        case TS_SEPARATOR: return TT_SEPARATOR;
        default: return TT_UNKNOWN;
    }
}

gvm_result_t tokenize(parser_text_t* text, parser_tokens_t* tokens) {

    gvm_result_t res = tokens_init(tokens, 16);
    int line = 1;
    int column = 1;

    state_update_t update_result = (state_update_t) {
        .state = TS_INIT,
        .prev_state = TS_INIT,
        .is_part_of_prev = false
    };

    int text_length = text->size;

    for(int i = 0; i < (text_length + 1); i++) {
        // make sure we process the last token
        // by faking an extra trailing whitespace
        lexeme_t lex = (i < text_length)
            ? scan(text->array[i])
            : L_NEWLINE; 

        update_result = get_new_state(update_result.state, lex);
        if( update_result.state == TS_ERROR ) {
            printf("error: invalid lexer state.\n"
                   "\tline: %i column: %i\n",
                   line, column);
            break;
        }

        token_type_t token_type = TT_UNKNOWN;
        if( update_result.is_part_of_prev ) {
            token_type = state_to_type(update_result.prev_state);
        } else {
            token_type = state_to_type(update_result.state);
        }
        res = tokens_push_on_change(tokens, token_type, line, column, i);
        if( res != RES_OK ) {
            printf("error: failed to construct token buffer.\n");
            gvm_print_if_error(res, "tokenize");
            break;
        }

        column ++;
        if( lex == L_NEWLINE ) {
            line ++;
            column = 1;
        }
    }

    // NOTE: to decrease memory footprint it
    // would be nice to realloc the dynamic
    // token array so capacity == size when
    // we are done.

    return res;
}

parser_t* parser_create(char* text) {
    parser_t* p = (parser_t*) malloc(sizeof(parser_t));
    if( p == NULL ) {
        printf("error: p_create, out of memory.\n");
        return NULL;
    }
    memset(p, 0, sizeof(parser_t));
    p->text.size = strlen(text);
    p->text.array = (char*) malloc((p->text.size + 1) * sizeof(char));
    if( p->text.array == NULL ) {
        parser_destroy(p);
        return NULL;
    }
    p->text.array[p->text.size] = '\0';
    memcpy(p->text.array, text, p->text.size);
    p->current = 0;
    gvm_result_t res = tokenize(&p->text, &p->tokens);
    gvm_print_if_error(res, "tokenize");
    if( res != RES_OK ) {
        parser_destroy(p);
        return NULL;
    }
    return p;
}

void parser_destroy(parser_t* p) {
    if( p == NULL ) {
        return;
    }
    if( p->text.array != NULL ) {
        free(p->text.array);
        p->text.array = NULL;
        p->text.size = 0;
    }
    if( p->tokens.array != NULL ) {
        free(p->tokens.array);
        p->tokens.size = 0;
        p->tokens.capacity = 0;
    }
    free(p);
}

void parser_reset(parser_t* p) {
    p->current = 0;
}

bool parser_is_at_end(parser_t* p) {
    return (p->current >= p->tokens.size);
}

bool parser_advance(parser_t* p) {
    if( parser_is_at_end(p) ) {
        return false;
    }
    p->current ++;
    return true;
}

bool parser_match(parser_t* p, token_type_t tt) {
    if( parser_is_at_end(p) ) {
        return false;
    }
    if( p->tokens.array[p->current].type == tt ) {
        return true;
    }
    return false;
}

bool parser_consume(parser_t* p, token_type_t tt) {
    if( parser_is_at_end(p) ) {
        printf("error: expected '%s' but parser reached end of file.\n",
            parser_tt_to_str(tt));
        return false;
    }
    token_t current = p->tokens.array[p->current];
    if( current.type == tt ) {
        return parser_advance(p);
    } else {
        printf("error: expected '%s' (actual '%s')\n\tline: %i\n\tcolumn: %i\n",
            parser_tt_to_str(tt),
            parser_tt_to_str(current.type),
            current.src_line,
            current.src_column);
        return false;
    }
}

char* parser_tt_to_str(token_type_t tt) {
    switch (tt) {
        case TT_COLON: return "COLON";
        case TT_NUMBER: return "NUMBER";
        case TT_VEC2: return "VEC2";
        case TT_SEPARATOR: return "SEPARATOR";
        case TT_COMMENT: return "COMMENT";
        case TT_STRING: return "STRING";
        case TT_SYMBOL: return "SYMBOL";
        case TT_UNKNOWN: return "UNKNOWN";
        case TT_END: return "END";
        default: return "<undefined>";
    }
}

token_t parser_current(parser_t* p) {
    return p->tokens.array[_MIN(p->current, p->tokens.size-1)];
}

token_t parser_peek(parser_t* p, int lookahead) {
    int index = p->current + lookahead;
    int last_index = p->tokens.size - 1;
    if( index > last_index ) {
        token_t err = p->tokens.array[last_index];
        err.type = TT_UNKNOWN;
        return err;
    }
    return p->tokens.array[index];
}

char* parser_get_token_string_ptr(parser_t* parser, token_t token) {
    return parser->text.array + token.src_index;
}

int parser_get_token_string_length(parser_t* parser, token_t token) {
    int str_start = token.src_index;
    int str_end = (token.index < (parser->tokens.size - 1))
        ? parser->tokens.array[(token.index + 1)].src_index
        : parser->tokens.array[(parser->tokens.size - 1)].src_index;
    return str_end - str_start;
}

int parser_get_token_int_value(parser_t* parser, token_t token) {
    int str_start = token.src_index;
    int str_end = (token.index < (parser->tokens.size - 1))
        ? parser->tokens.array[(token.index + 1)].src_index
        : parser->tokens.array[(parser->tokens.size - 1)].src_index;
    int len = str_end - str_start;
    if( len > PARSER_CHAR_BUFFER_LEN ) {
        len = PARSER_CHAR_BUFFER_LEN;
    }
    char buf[PARSER_CHAR_BUFFER_LEN] = { 0 };
    for(int i = 0; i < len; i++) {
        buf[i] = parser->text.array[i + str_start];
    }
    return atoi(buf);
}

void parser_debug_print_tokens(parser_t* parser) {
    parser_tokens_t tokens = parser->tokens;
    for (int i = 0; i < tokens.size; i++) {
        token_t token = tokens.array[i];
        int len = parser_get_token_string_length(parser, token);
        char* str = parser_get_token_string_ptr(parser, token);
        printf("TOKEN #%i: \"%.*s\"\n  (%s)\n  -----\n", i, len, str, parser_tt_to_str(token.type));
    }
}
