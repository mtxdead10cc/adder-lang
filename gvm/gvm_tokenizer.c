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
    char*               buffer;
    size_t              buffer_size;
    char*               filepath;
    size_t              cursor;
    srcref_map_t        kw_map_alpha;
    srcref_map_t        kw_map_symbolic;
} tokenizer_state_t;

size_t sweep_while(tokenizer_state_t* state, size_t start_offset, lex_predicate_t lex) {
    size_t cursor = state->cursor + start_offset;
    size_t stop = state->buffer_size - 1;
    while ( lexer_match(lex, lexer_scan(state->buffer[cursor])) && cursor <= stop ) {
        cursor ++;
    }
    return cursor - state->cursor;
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

token_t sweep_make_token(
    tokenizer_state_t* state,
    size_t offset,
    size_t trailing,
    lex_predicate_t while_true,
    token_type_t token_type)
{
    size_t start = state->cursor;
    size_t len = sweep_while(state, offset, while_true);
    assert(len > 0 && "unexpected sweep_while progress");
    len = len + offset + trailing;
    state->cursor = start + len;
    return (token_t) {
        .ref = srcref(state->buffer, start, len, state->filepath),
        .type = token_type
    };
}

void sweep_discard_token(
    tokenizer_state_t* state,
    size_t offset,
    size_t trailing,
    lex_predicate_t while_true)
{
    size_t start = state->cursor;
    size_t len = sweep_while(state, offset, while_true);
    assert(len > 0 && "unexpected sweep_while progress");
    len = len + offset + trailing;
    state->cursor = start + len;
}

srcref_map_t create_keyword_token_map() {
    srcref_map_t map;
    srcref_map_init(&map, 50);

    srcref_map_insert(&map, srcref_const("if"),     TT_KW_IF);
    srcref_map_insert(&map, srcref_const("else"),   TT_KW_ELSE);
    srcref_map_insert(&map, srcref_const("for"),    TT_KW_FOR);
    srcref_map_insert(&map, srcref_const("fun"),    TT_KW_FUN_DEF);
    srcref_map_insert(&map, srcref_const("true"),   TT_BOOLEAN);
    srcref_map_insert(&map, srcref_const("false"),  TT_BOOLEAN);
    srcref_map_insert(&map, srcref_const("not"),    TT_KW_NOT);
    srcref_map_insert(&map, srcref_const("and"),    TT_KW_AND);
    srcref_map_insert(&map, srcref_const("or"),     TT_KW_OR);

    return map;
}

token_t lookup_alpha_token(tokenizer_state_t* state) {
    size_t start = state->cursor;
    size_t len = sweep_while(state, 0,
        lp_is(LCAT_LETTER|LCAT_UNDERSCORE|LCAT_NUMBER));
    assert(len > 0 && "unexpected sweep_while progress");
    state->cursor = start + len;
    srcref_t ref = srcref(state->buffer, start, len, state->filepath);
    uint32_t* tt = srcref_map_lookup(&state->kw_map_alpha, ref);
    return (token_t) {
        .ref = ref,
        .type = tt != NULL ? *tt : TT_SYMBOL
    };
}

srcref_map_t create_symbolic_token_map() {
    srcref_map_t map;
    srcref_map_init(&map, 50);

    srcref_map_insert(&map, srcref_const("->"), TT_ARROW);
    srcref_map_insert(&map, srcref_const("=="), TT_CMP_EQ);
    srcref_map_insert(&map, srcref_const("<="), TT_CMP_LT_EQ);
    srcref_map_insert(&map, srcref_const(">="), TT_CMP_GT_EQ);
    srcref_map_insert(&map, srcref_const("<"),  TT_LT_OR_OPEN_ABRACKET);
    srcref_map_insert(&map, srcref_const(">"),  TT_GT_OR_CLOSE_ABRACKET);
    srcref_map_insert(&map, srcref_const("("),  TT_OPEN_PAREN);
    srcref_map_insert(&map, srcref_const(")"),  TT_CLOSE_PAREN);
    srcref_map_insert(&map, srcref_const("{"),  TT_OPEN_CURLY);
    srcref_map_insert(&map, srcref_const("}"),  TT_CLOSE_CURLY);
    srcref_map_insert(&map, srcref_const("["),  TT_OPEN_SBRACKET);
    srcref_map_insert(&map, srcref_const("]"),  TT_CLOSE_SBRACKET);
    srcref_map_insert(&map, srcref_const("="),  TT_ASSIGN);
    srcref_map_insert(&map, srcref_const(","),  TT_SEPARATOR);
    srcref_map_insert(&map, srcref_const(";"),  TT_STATEMENT_END);

    return map;
}

size_t get_symbolic_token_len(tokenizer_state_t* state) {
    if( match_cursor_and_next(state,
            lp_is(LCAT_MINUS),
            lp_is(LCAT_GREATER_THAN)))
    {
        return 2; // arrow
    }
    else if (match_cursor_and_next(state, 
            lp_is(LCAT_EQUAL|LCAT_LESS_THAN|LCAT_GREATER_THAN),
            lp_is(LCAT_GREATER_THAN)))
    {
        return 2; // cmps
    }
    else 
    {
        return 1; // other
    }
}

token_t lookup_symbolic_token(tokenizer_state_t* state) {
    size_t start = state->cursor;
    size_t len = get_symbolic_token_len(state);
    state->cursor = start + len;
    srcref_t ref = srcref(state->buffer, start, len, state->filepath);
    uint32_t* tt = srcref_map_lookup(&state->kw_map_symbolic, ref);
    return (token_t) {
        .ref = ref,
        .type = tt != NULL ? *tt : TT_SYMBOL
    };
}

void destroy_token_map(srcref_map_t* map) {
    srcref_map_destroy(map);
}

bool tokenizer_analyze(token_collection_t* collection, tokenizer_args_t* args) {

    tokenizer_state_t state = (tokenizer_state_t) {
        .buffer = args->text,
        .buffer_size = args->text_length,
        .filepath = args->filepath,
        .cursor = 0,
        .kw_map_alpha = create_keyword_token_map(),
        .kw_map_symbolic = create_symbolic_token_map()
    };

    while ( state.cursor < state.buffer_size ) {
        
        size_t last_cursor_pos = state.cursor;

        if(  match_cursor(&state, lp_is(LCAT_SPACE)) ) {                                           // SPACE
            if( args->include_spaces ) {
                tokens_append(collection,
                    sweep_make_token(&state, 0, 0, lp_is(LCAT_SPACE), TT_SPACE));
            } else {
                sweep_discard_token(&state, 0, 0, lp_is(LCAT_SPACE));
            }
        } else if( match_cursor_and_next(&state, lp_is(LCAT_SLASH), lp_is(LCAT_SLASH)) ) {         // COMMENT
            if( args->include_comments ) {
                tokens_append(collection,
                    sweep_make_token(&state, 2, 1, lp_is_not(LCAT_NEWLINE), TT_COMMENT));
            } else {
                sweep_discard_token(&state, 2, 1, lp_is_not(LCAT_NEWLINE));
            }
        } else if ( match_cursor(&state, lp_is(LCAT_QUOTE)) ) {                                     // STRING
            tokens_append(collection,
                sweep_make_token(&state, 1, 1, lp_is_not(LCAT_QUOTE), TT_STRING));
        } else if ( match_cursor(&state, lp_is(LCAT_NUMBER)) ) {                                    // NUMBER
            tokens_append(collection,
                sweep_make_token(&state, 0, 0, lp_is(LCAT_NUMBER|LCAT_DOT), TT_NUMBER));
        } else if ( match_cursor(&state, lp_is(LCAT_LETTER|LCAT_UNDERSCORE)) ) {
            tokens_append(collection, lookup_alpha_token(&state));                                  // IDENTIFIER
        } else if ( match_cursor(&state, lp_is(LCAT_SYMBOLIC)) ) {
            tokens_append(collection, lookup_symbolic_token(&state));                               // SYMBOLIC
        }

        if( last_cursor_pos == state.cursor ) {
            break;
        }
    }

    destroy_token_map(&state.kw_map_alpha);
    destroy_token_map(&state.kw_map_symbolic);

    return true;
}