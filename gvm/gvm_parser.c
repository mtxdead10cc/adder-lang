#include "gvm_parser.h"
#include "gvm_types.h"
#include "gvm.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

typedef enum lexeme_t {
    L_LETTER,
    L_NUMBER,
    L_DASH,
    L_DOT,
    L_QUOTE,
    L_COLON,
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
            .type = tt
        });
    }
    return res;
}


gvm_result_t tokenize(parser_text_t* text, parser_tokens_t* tokens) {

    gvm_result_t res = tokens_init(tokens, 16);
    lexeme_t last = L_UNKNOWN; // current - 1
    int line = 1;
    int column = 1;
    bool in_quoute = false;
    
    for(int i = 0; i < text->size; i++) {
        lexeme_t lex = scan(text->array[i]);

        if( res != RES_OK ) {
            break;
        }

        if( in_quoute ) {
            res = tokens_push_on_change(tokens, TT_STRING, line, column, i);
        } else if ( lex == L_QUOTE ) {
            in_quoute = !in_quoute;
        } else if ( lex == L_COLON ) {
            res = tokens_push_on_change(tokens, TT_COLON, line, column, i);
        } else if ( lex == L_DOT && last == L_NUMBER ) {
            res = tokens_push_on_change(tokens, TT_NUMBER, line, column, i);
        } else if ( lex == L_NUMBER ) {
            res = tokens_push_on_change(tokens, TT_NUMBER, line, column, i);
        } else if ( lex == L_DASH && last == L_LETTER ) {
            res = tokens_push_on_change(tokens, TT_SYMBOL, line, column, i);
        } else if ( lex == L_LETTER ) {
            res = tokens_push_on_change(tokens, TT_SYMBOL, line, column, i);
        } else if ( lex == L_WHITESPACE || lex == L_NEWLINE ) {
            res = tokens_push_on_change(tokens, TT_SEPARATOR, line, column, i);
        } else {
            res = tokens_push_on_change(tokens, TT_UNKNOWN, line, column, i);
        }

        column ++;
        if( lex == L_NEWLINE ) {
            line ++;
            column = 1;
        }

        last = lex;
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
    p->text.array = (char*) malloc(p->text.size * sizeof(char));
    if( p->text.array == NULL ) {
        parser_destroy(p);
        return NULL;
    }
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

bool parser_is_at_end(parser_t* p) {
    return p->current < p->tokens.size;
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

void parser_current_as_string(parser_t* parser, char* buffer, int max_len) {
    int token_index = parser->current;
    parser_token_as_string(parser, token_index, buffer, max_len);
}

void parser_token_as_string(parser_t* parser, int token_index, char* buffer, int max_len) {
    int str_start = parser->tokens.array[token_index].src_index;
    int str_end = (token_index < (parser->tokens.size - 1))
        ? parser->tokens.array[(token_index + 1)].src_index
        : parser->tokens.array[(parser->tokens.size - 1)].src_index;
    int len = str_end - str_start;
    if( len > max_len ) {
        len = max_len;
    }
    for(int j = 0; j < len; j++) {
        buffer[j] = parser->text.array[j + str_start];
    }
}

char* parser_tt_to_str(token_type_t tt) {
    switch (tt) {
        case TT_COLON: return "COLON";
        case TT_NUMBER: return "NUMBER";
        case TT_SEPARATOR: return "SEPARATOR";
        case TT_STRING: return "STRING";
        case TT_SYMBOL: return "SYMBOL";
        case TT_UNKNOWN: return "UNKNOWN";
        case TT_END: return "END";
        default: return "<undefined>";
    }
}
