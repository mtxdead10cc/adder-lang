#include "shared/sh_parse_tools.h"
#include <string.h>
#include <stdlib.h>

pt_token_t pt_tokenizer_get_token(pt_token_gen_t tokr, char* text, ptrdiff_t text_current, ptrdiff_t text_length) {

    ptrdiff_t remaining = text_length - text_current;
    char* start_ptr     = text + text_current;
    int64_t tt          = -1;

    ptrdiff_t consumed = tokr(start_ptr, remaining, &tt);
    if( tt >= 0 && consumed > 0 ) {
        return (pt_token_t) {
            .text = start_ptr,
            .length = consumed,
            .type = tt
        };
    }

    return (pt_token_t) { 0 };
}

ptrdiff_t pt_tokenizer_seek_next_valid(pt_token_gen_t tokr, char* text, ptrdiff_t text_current, ptrdiff_t text_length) {
    for(ptrdiff_t d = text_current; d < text_length; d++) {
        ptrdiff_t test_start = d + text_current;
        pt_token_t t = pt_tokenizer_get_token(tokr, text, test_start, text_length - d);
        if( t.length > 0 && (t.length + test_start) < text_length )
            return test_start;
    }
    return 0;
}

bool pt_string_equal(char* a, ptrdiff_t a_len, const char* b, ptrdiff_t b_len) {
    if( a_len < b_len )
        return false;
    for (ptrdiff_t i = 0; i < b_len; i++) {
        if( a[i] != b[i] )
            return false;
    }
    return true;
}

bool pt_string_contains_char(char* str, ptrdiff_t str_len, char c) {
    if( str_len == 0 )
        return false;
    for (ptrdiff_t i = 0; i < str_len; i++) {
        if( str[i] == c )
            return true;
    }
    return false;
}

bool pt_char_is_any_of(char c, const char* chars, ptrdiff_t len) {
    for(ptrdiff_t i = 0; i < len; i++) {
        if( c == chars[i] )
            return true;
    }
    return false;
}


ptrdiff_t pt_seek_next_non_whitespace(char* text, ptrdiff_t remaining) {
    for(ptrdiff_t i = 0; i < remaining; i++) {
        if(pt_char_whitespace(text[i]) == false)
            return i;
    }
    return 0;
}

ptrdiff_t pt_scan_string(char* text, ptrdiff_t remaining) {
    if(text[0] != '"')
        return 0;
    for(ptrdiff_t i = 1; i < remaining; i++) {
        if( text[i] == '"' )
            return i+1;
    }
    return 0;
}

ptrdiff_t pt_scan_number(char* text, ptrdiff_t remaining) {

    if( remaining <= 0 )
        return 0;

    ptrdiff_t maxlen = 0;
    while (maxlen < remaining) {
        char c = text[maxlen];
        bool accept =      pt_char_number(c);
        accept = accept || pt_char_whitespace(c);
        accept = accept || pt_char_any_of(c, "-+.xX");
        if( accept == false )
            break;
        maxlen ++;
    }

    if( maxlen <= 0 )
        return 0;

    char tmp[maxlen+1];
    strncpy(tmp, text, maxlen);
    tmp[maxlen] = '\0';
    
    char* end = tmp;
    strtod(tmp, &end);

    return end - tmp;
}

pt_token_t pt_state_peek(pt_state_t* state, ptrdiff_t offs) {
    if( (state->index + offs) < state->token_count ) {
        return state->tokens[state->index + offs];
    }
    return (pt_token_t) { 0 };
}

bool pt_state_match_any(pt_state_t* state, int64_t tts) {
    return (pt_state_peek(state, 0).type & tts) > 0;
}

bool pt_state_advance(pt_state_t* state) {
    if( state->index < (state->token_count - 1) )
        state->index += 1;
    return state->index < state->token_count;
}

bool pt_state_advance_if(pt_state_t* state, int64_t tt) {
    if(pt_state_peek(state, 0).type == tt)
        return pt_state_advance(state);
    return false;
}

bool pt_state_has_tokens(pt_state_t* state) {
    return state->index < (state->token_count - 1);
}

pt_result_code_t pt_state_init(pt_state_t* state, pt_token_gen_t tokr, char* text, ptrdiff_t text_length) {

    if( state == NULL )
        return PT_RES_ERROR_PARAMETER;

    ptrdiff_t token_capacity = 8;
    ptrdiff_t text_index = 0;

    state->index = 0;
    state->token_count = 0;
    state->tokens = malloc(sizeof(pt_token_t) * token_capacity);

    if(state->tokens == NULL)
        return PT_RES_ERROR_MEMORY;

    while(text_index < text_length) {
        text_index += pt_seek_next_non_whitespace(text + text_index, text_length - text_index);

        pt_token_t token = pt_tokenizer_get_token(tokr, text, text_index, text_length);
        if( token.length == 0 )
            break;
        
        if( state->token_count == token_capacity ) {
            token_capacity *= 2;
            pt_token_t* newtok = realloc(state->tokens, sizeof(pt_token_t) * token_capacity);
            if( newtok == NULL ) {
                free(state->tokens);
                return PT_RES_ERROR_MEMORY;
            }
            state->tokens = newtok;
        }

        state->tokens[state->token_count] = token;
        state->token_count += 1;
        text_index += token.length;
    }

    if(text_index != text_length - 1)
        return PT_RES_INCOMPLETE;
    
    return PT_RES_OK;
}

void pt_state_free(pt_state_t* state) {
    if( state == NULL )
        return;
    if( state->tokens == NULL )
        return;
    free(state->tokens);
    *state = (pt_state_t) {0};
}