#include "gvm_parser.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gvm_types.h"
#include "gvm_utils.h"
#include "gvm_lexer.h"

token_t token_const(const char* content, token_type_t type) {
    return (token_t) {
        .ref = srcref_const(content),
        .type = type
    };
}

bool token_equals(token_t a, token_t b) {
    return a.type == b.type && srcref_equals(a.ref, b.ref);
}

bool parser_append_token(parser_t* parser, token_t token) {
    if( parser->token_capacity <= (parser->token_count + 1) ) {
        size_t capacity = (parser->token_count + 1) * 2;
        token_t* tokens = realloc(parser->tokens, capacity * sizeof(token_t));
        if( tokens == NULL ) {
            return false;
        } else {
            parser->token_capacity = capacity;
            parser->tokens = tokens;
        }
    }
    parser->tokens[parser->token_count++] = token;
    return true;
}


bool tokenize(parser_t* parser, char* buffer, size_t buffer_size, char* filepath) {

    lexer_t lexer;
    lexer_init(&lexer, buffer, buffer_size, filepath);

    srcref_t ref;
    lex_type_t lex_type;

    token_t wip_token = (token_t) {
        .ref = srcref(buffer, 0, buffer_size, filepath),
        .type = TT_INITIAL
    };

    while( (lex_type = lexer_get_next(&lexer, &ref)) != L_EOF ) {

        switch (wip_token.type) {
            case TT_STRING: {
                wip_token.ref = srcref_combine(wip_token.ref, ref);
                if( lex_type == L_QUOTE ) {
                    parser_append_token(parser, wip_token);
                    wip_token.type = TT_INITIAL;
                    wip_token.ref.idx_start = wip_token.ref.idx_end;
                }
            } break;
            case TT_COMMENT: {
                wip_token.ref = srcref_combine(wip_token.ref, ref);
                if( lex_type == L_NEWLINE ) {
                    parser_append_token(parser, wip_token);
                    wip_token.type = TT_INITIAL;
                    wip_token.ref.idx_start = wip_token.ref.idx_end;
                }
            } break;
            case TT_NUMBER: {
                if( lex_type == L_DOT ) {
                    wip_token.ref = srcref_combine(wip_token.ref, ref);
                } else {
                    parser_append_token(parser, wip_token);
                    wip_token.type = TT_INITIAL;
                    wip_token.ref.idx_start = wip_token.ref.idx_end;
                }
            } break;
            default: {
                switch (lex_type) {
                    case L_SLASH: {
                        if( srcref_len(ref) > 1 ) {
                            wip_token.ref = ref;
                            wip_token.type = TT_COMMENT;
                        } else {
                            wip_token.ref = ref;
                            wip_token.type = TT_SYMBOL;
                            parser_append_token(parser, wip_token);
                        }
                    } break;
                    case L_DASH:
                    case L_SPECIAL:
                    case L_LETTER: {
                        wip_token.ref = ref;
                        wip_token.type = TT_SYMBOL;
                        parser_append_token(parser, wip_token);
                    } break;
                    case L_NUMBER: {
                        wip_token.ref = ref;
                        wip_token.type = TT_NUMBER;
                    } break;
                    case L_GROUP_OPEN: {
                        wip_token.ref = ref;
                        wip_token.type = TT_GROUP_START;
                        parser_append_token(parser, wip_token);
                    } break;
                    case L_GROUP_CLOSE: {
                        wip_token.ref = ref;
                        wip_token.type = TT_GROUP_END;
                        parser_append_token(parser, wip_token);
                    } break;
                    case L_QUOTE: {
                        wip_token.ref = ref;
                        wip_token.type = TT_STRING;
                    } break;
                    case L_NEWLINE:
                    case L_WHITESPACE: {
                        wip_token.ref = ref;
                        wip_token.type = TT_SPACE;
                        parser_append_token(parser, wip_token);
                    } break;
                    default: {
                        printf("lexing error\n");
                    } break;
                }
            } break;
        }
    }

    // if multi-part token make sure to add the last one
    if( (wip_token.type == TT_NUMBER) || (wip_token.type == TT_COMMENT) ) {
        parser_append_token(parser, wip_token);
    }

    return true;
}

bool parser_init(parser_t* parser, char* text, size_t text_length, char* filepath) {

    parser->token_capacity = 16;
    parser->tokens = (token_t*) malloc( parser->token_capacity * sizeof(token_t) );
    if( parser->tokens == NULL ) {
        *parser = (parser_t) { 0 };
        return false;
    }

    parser->token_index = 0;
    parser->token_count = 0;

    return tokenize(parser, text, text_length, filepath);
}

char* parser_token_type_as_string(token_type_t type) {
    switch (type) {
        case TT_INITIAL: return "TT_INITIAL";
        case TT_SPACE: return "TT_SPACE";
        case TT_COMMENT: return "TT_COMMENT";
        case TT_SYMBOL: return "TT_SYMBOL";
        case TT_NUMBER: return "TT_NUMBER";
        case TT_BOOLEAN: return "TT_BOOLEAN";
        case TT_STRING: return "TT_STRING";
        case TT_GROUP_START: return "TT_GROUP_START";
        case TT_GROUP_END: return "TT_GROUP_END";
        case TT_FINAL: return "TT_FINAL";
        default: return "<UNKNOWN-TT>";
    }
}


bool parser_is_at_end(parser_t* parser) {
    return parser->token_index >= (parser->token_count - 1);
}

void parser_dump_tokens(parser_t* parser) {
    for(size_t i = 0; i < parser->token_count; i++) {
        printf("(%s '", parser_token_type_as_string(parser->tokens[i].type));
        srcref_print(parser->tokens[i].ref);
        printf("')\n");
    }
}
