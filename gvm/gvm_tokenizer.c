#include "gvm_tokenizer.h"
#include <stdio.h>

bool tokens_init(token_collection_t* collection, size_t capacity) {
    collection->count = 0;
    collection->tokens = (token_t*) malloc( capacity * sizeof(token_t) );
    if( collection->tokens == NULL ) {
        return false;
    }
    collection->capacity = capacity;
    return true;
}

void tokens_clear(token_collection_t* collection) {
    collection->count = 0;
}

bool tokens_append(token_collection_t* collection, token_t token) {
    if( collection->capacity <= (collection->count + 1) ) {
        size_t capacity = (collection->count + 1) * 2;
        token_t* tokens = realloc(collection->tokens, capacity * sizeof(token_t));
        if( tokens == NULL ) {
            return false;
        } else {
            collection->capacity = capacity;
            collection->tokens = tokens;
        }
    }
    collection->tokens[collection->count++] = token;
    return true;
}

bool tokens_merge_top(token_collection_t* collection) {
    if( collection->count < 2 ) {
        return collection->count == 1;
    } else {
        collection->count -= 1;
        token_t pop = collection->tokens[collection->count];
        token_t top = collection->tokens[collection->count-1]; 
        assert( top.type == pop.type );
        top.ref = srcref_combine(top.ref, pop.ref);
        collection->tokens[collection->count-1] = top;
        return true;
    }
}

void tokens_print(token_collection_t* collection) {
    for(size_t i = 0; i < collection->count; i++) {
        printf("(%s '", token_get_type_name(collection->tokens[i].type));
        srcref_print(collection->tokens[i].ref);
        printf("')\n");
    }
}

void tokens_destroy(token_collection_t* collection) {
    if( collection == NULL ) {
        return;
    }
    if( collection->tokens != NULL ) {
        free(collection->tokens);
    }
    collection->tokens = NULL;
    collection->capacity = 0;
    collection->count = 0;
}

typedef struct tokenizer_state_t {
    char* buffer;
    size_t buffer_size;
    char* filepath;
    size_t cursor;
} tokenizer_state_t;

size_t sweep_while(tokenizer_state_t* state, size_t start_offset, lex_predicate_t lex) {
    size_t cursor = state->cursor + start_offset;
    size_t stop = state->buffer_size - 1;
    while ( lexer_match(lex, lexer_scan(state->buffer[cursor])) && cursor <= stop ) {
        cursor ++;
    }
    return cursor - state->cursor - start_offset;
}

bool match_cursor(tokenizer_state_t* state, lex_predicate_t lex) {
    if( state->cursor >= state->buffer_size ) {
        return false;
    }
    return lexer_match(lex, lexer_scan(state->buffer[state->cursor]));
}

bool match_cursor_and_next(tokenizer_state_t* state, lex_predicate_t lex_0, lex_predicate_t lex_1) {
    if( state->cursor + 1 >= state->buffer_size ) {
        return false;
    }
    return  lexer_match(lex_0, lexer_scan(state->buffer[state->cursor + 0]))
         && lexer_match(lex_1, lexer_scan(state->buffer[state->cursor + 1]));
}

srcref_map_t create_token_map() {
    srcref_map_t map;
    srcref_map_init(&map, 16);

    srcref_map_insert(&map, srcref_const("if"), TT_KW_IF);
    srcref_map_insert(&map, srcref_const("else"), TT_KW_ELSE);
    srcref_map_insert(&map, srcref_const("for"), TT_KW_FOR);
    srcref_map_insert(&map, srcref_const("fun"), TT_KW_FUN_DEF);
    srcref_map_insert(&map, srcref_const("true"), TT_BOOLEAN);
    srcref_map_insert(&map, srcref_const("false"), TT_BOOLEAN);

    return map;
}

void destroy_token_map(srcref_map_t* map) {
    srcref_map_destroy(map);
}

bool tokenizer_analyze(token_collection_t* collection, char* text, size_t text_length, char* filepath) {

    tokenizer_state_t state = (tokenizer_state_t) {
        .buffer = text,
        .buffer_size = text_length,
        .filepath = filepath,
        .cursor = 0
    };

    srcref_map_t map = create_token_map();

    // TODO
    // [ ] 'true' / 'false'     => TT_BOOLEAN
    // [ ] '->'                 => TT_ARROW
    // [ ] '==', '<=', '>='     => TT_COMPARISON
    // [ ] ';'                  => TT_END_STATEMENT
    // [ ] ','                  => TT_SEPARATOR
    // [ ] 'if', 'for' ...      => TT_KEYWORD

    while ( state.cursor < state.buffer_size ) {
        
        size_t last_cursor_pos = state.cursor;

        if(  match_cursor(&state, lp_is(LCAT_SPACE)) ) {
            size_t start = state.cursor;
            size_t len = sweep_while(&state, 0, lp_is(LCAT_SPACE));
            assert(len > 0 && "unexpected sweep_while progress");
            len = len;
            state.cursor = start + len;
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = TT_SPACE
            });
        } else if( match_cursor_and_next(&state, lp_is(LCAT_SLASH), lp_is(LCAT_SLASH)) ) {
            size_t start = state.cursor;
            size_t len = sweep_while(&state, 2, lp_is_not(LCAT_NEWLINE));
            assert(len > 0 && "unexpected sweep_while progress");
            len = len + 2; // plus the two slashes
            state.cursor = start + len;
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = TT_COMMENT
            });
        } else if ( match_cursor(&state, lp_is(LCAT_QUOTE)) ) {
            size_t start = state.cursor;
            size_t len = sweep_while(&state, 1, lp_is_not(LCAT_QUOTE));
            assert(len > 0 && "unexpected sweep_while progress");
            len = len + 2; // + beginning and end quotes
            state.cursor = start + len;
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = TT_STRING
            });
        } else if ( match_cursor(&state, lp_is(LCAT_NUMBER)) ) {
            size_t start = state.cursor;
            size_t len = sweep_while(&state, 0, lp_is(LCAT_NUMBER|LCAT_DOT));
            assert(len > 0 && "unexpected sweep_while progress");
            state.cursor = start + len;
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = TT_NUMBER
            });
        } else if ( match_cursor(&state, lp_is(LCAT_LETTER|LCAT_UNDERSCORE)) ) {
            size_t start = state.cursor;
            size_t len = sweep_while(&state, 0, lp_is(LCAT_NUMBER|LCAT_LETTER|LCAT_UNDERSCORE));
            assert(len > 0 && "unexpected sweep_while progress");
            state.cursor = start + len;
            srcref_t ref = srcref(state.buffer, start, len, state.filepath);
            uint32_t* tt = srcref_map_lookup(&map, ref);
            tokens_append(collection, (token_t) {
                .ref = ref,
                .type = tt != NULL ? *tt : TT_SYMBOL
            });
        } else if ( match_cursor_and_next(&state,
                        lp_is(LCAT_MINUS),
                        lp_is(LCAT_GREATER_THAN)) )
        {
            size_t start = state.cursor;
            size_t len = 2;
            state.cursor = start + len;
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = TT_ARROW
            });
        } else if ( match_cursor_and_next(&state,
                        lp_is(LCAT_GREATER_THAN|LCAT_LESS_THAN|LCAT_EQUAL),
                        lp_is(LCAT_EQUAL)) ) 
        {
            
            token_type_t tt = TT_CMP_EQ;
            if( match_cursor(&state, lp_is(LCAT_GREATER_THAN)) ){
                tt = TT_CMP_GT_EQ;
            } else if ( match_cursor(&state, lp_is(LCAT_LESS_THAN)) ) {
                tt = TT_CMP_LT_EQ;
            }

            size_t start = state.cursor;
            size_t len = 2;
            state.cursor = start + len;
            
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = tt
            });

        } else if ( match_cursor(&state, lp_is(LCAT_COMMA)) ) {
            size_t start = state.cursor;
            size_t len = 1;
            state.cursor = start + len;
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = TT_SEPARATOR
            });
        } else if ( match_cursor(&state, lp_is(LCAT_SEMI_COLON)) ) {
            size_t start = state.cursor;
            size_t len = 1;
            state.cursor = start + len;
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = TT_STATEMENT_END
            });
        } else if ( match_cursor(&state, lp_is(LCAT_LESS_THAN)) ) {
            size_t start = state.cursor;
            size_t len = 1;
            state.cursor = start + len;
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = TT_CMP_LT
            });
        } else if ( match_cursor(&state, lp_is(LCAT_EQUAL)) ) {
            size_t start = state.cursor;
            size_t len = 1;
            state.cursor = start + len;
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = TT_ASSIGN
            });
        } else if ( match_cursor(&state, lp_is(LCAT_GREATER_THAN)) ) {
            size_t start = state.cursor;
            size_t len = 1;
            state.cursor = start + len;
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = TT_CMP_GT
            });
        } else if ( match_cursor(&state, lp_is(LCAT_SCOPE_START)) ) {
            size_t start = state.cursor;
            size_t len = 1;
            state.cursor = start + len;
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = TT_GROUP_START
            });
        } else if ( match_cursor(&state, lp_is(LCAT_SCOPE_END)) ) {
            size_t start = state.cursor;
            size_t len = 1;
            state.cursor = start + len;
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = TT_GROUP_END
            });
        } else if ( match_cursor(&state, lp_is(LCAT_SYMBOL)) ) {
            size_t start = state.cursor;
            size_t len = 1;
            state.cursor = start + len;
            tokens_append(collection, (token_t) {
                .ref = srcref(state.buffer, start, len, state.filepath),
                .type = TT_SYMBOL
            });
        }

        if( last_cursor_pos == state.cursor ) {
            break;
        }
    }

    destroy_token_map(&map);

    return true;
}