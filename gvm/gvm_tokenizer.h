#ifndef GVM_TOKENIZER_H_
#define GVM_TOKENIZER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "gvm_strmatch.h"

typedef enum token_type_t {
    TT_INITIAL,
    TT_SPACE,
    TT_COMMENT,
    TT_SYMBOL,
    TT_NUMBER,
    TT_BOOLEAN,
    TT_STRING,
    TT_GROUP_START,
    TT_GROUP_END,
    TT_FINAL,
    TT_TOKEN_TYPE_COUNT
} token_type_t;

typedef struct token_t {
    token_type_t type;
    srcref_t ref;
} token_t;

typedef struct token_collection_t {
    token_t*    tokens;
    size_t      capacity;
    size_t      count;
} token_collection_t;

inline static token_t token_const(const char* content, token_type_t type) {
    return (token_t) {
        .ref = srcref_const(content),
        .type = type
    };
}

inline static bool token_equals(token_t a, token_t b) {
    return a.type == b.type && srcref_equals(a.ref, b.ref);
}

inline static char* token_get_type_name(token_type_t type) {
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


inline static bool tokens_init(token_collection_t* collection, size_t capacity) {
    collection->count = 0;
    collection->tokens = (token_t*) malloc( capacity * sizeof(token_t) );
    if( collection->tokens == NULL ) {
        return false;
    }
    collection->capacity = capacity;
    return true;
}

inline static void tokens_clear(token_collection_t* collection) {
    collection->count = 0;
}

inline static bool tokens_append(token_collection_t* collection, token_t token) {
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

inline static void tokens_destroy(token_collection_t* collection) {
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

inline static size_t sweep_until(tokenizer_state_t* state, size_t start, char c) {
    size_t cursor = start;
    size_t stop = state->buffer_size - 1;
    while ( state->buffer[cursor] != c && cursor <= stop ) {
        cursor ++;
    }
    if( cursor < stop ) {
        cursor ++;
    }
    return cursor - start;
}

inline static size_t sweep_while_whitespace(tokenizer_state_t* state, size_t start) {
    size_t cursor = start;
    size_t stop = state->buffer_size - 1;
    while ( SM_IS_ANY_OF(state->buffer[cursor], ' ', '\n', '\r', '\t') && cursor <= stop ) {
        cursor ++;
    }
    return cursor - start;
}

inline static size_t sweep_while_number(tokenizer_state_t* state, size_t start) {
    size_t cursor = start;
    size_t stop = state->buffer_size - 1;
    while ( SM_IS_NUMBER(state->buffer[cursor]) && cursor <= stop ) {
        cursor ++;
    }
    return cursor - start;
}

inline static bool match_cursor_str(tokenizer_state_t* state, size_t len, char* str) {
    if( state->cursor + len >= state->buffer_size ) {
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        if( state->buffer[state->cursor + i] != str[i] ) {
            return false;
        }
    }
    return true;
}

inline static bool try_collect_comment(token_collection_t* collection, tokenizer_state_t* state) {
    if( match_cursor_str(state, 2, "//") == false ) {
        return false;
    }
    size_t cursor_start = state->cursor;
    size_t sweep_len = sweep_until(state,
        cursor_start + 2,
        '\n');
    if( sweep_len > 0 ) {
        state->cursor += (2 + sweep_len);
        return tokens_append(collection, (token_t) {
            .ref = (srcref_t) {
                .filepath = state->filepath,
                .source = state->buffer,
                .idx_start = cursor_start,
                .idx_end = state->cursor
            },
            .type = TT_COMMENT
        });
    }
    return sweep_len > 0;
}

inline static bool try_collect_string(token_collection_t* collection, tokenizer_state_t* state) {
    if( state->buffer[state->cursor] != '\"' ) {
        return false;
    }
    size_t cursor_start = state->cursor;
    size_t sweep_len = sweep_until(state,
        cursor_start + 1,
        '\"');
    if( sweep_len > 0 ) {
        state->cursor += (1 + sweep_len);
        return tokens_append(collection, (token_t) {
            .ref = (srcref_t) {
                .filepath = state->filepath,
                .source = state->buffer,
                .idx_start = cursor_start,
                .idx_end = state->cursor
            },
            .type = TT_STRING
        });
    }
    return sweep_len > 0;
}

inline static bool try_collect_number(token_collection_t* collection, tokenizer_state_t* state) {
    size_t cursor_start = state->cursor;
    size_t sweep_len = sweep_while_number(state, cursor_start);
    if ( sweep_len == 0 ) {
        return false;
    }

    state->cursor += sweep_len;

    token_t number_token = (token_t) {
        .ref = (srcref_t) {
            .filepath = state->filepath,
            .source = state->buffer,
            .idx_start = cursor_start,
            .idx_end = state->cursor
        },
        .type = TT_NUMBER
    };
    
    if( state->buffer[state->cursor] != '.' ) {
        return tokens_append(collection, number_token);
    }

    number_token.ref.idx_end += 1;
    state->cursor += 1;

    sweep_len = sweep_while_number(state, state->cursor);
    if( sweep_len == 0 ) {
        return tokens_append(collection, number_token);
    }

    number_token.ref.idx_end += sweep_len;
    state->cursor += sweep_len;

    return tokens_append(collection, number_token);
}

inline static bool try_collect_space(token_collection_t* collection, tokenizer_state_t* state) {
    size_t cursor_start = state->cursor;
    size_t sweep_len = sweep_while_whitespace(state, cursor_start);
    if( sweep_len == 0 ) {
        return false;
    }
    state->cursor += sweep_len;
    return tokens_append(collection, (token_t) {
        .ref = (srcref_t) {
            .filepath = state->filepath,
            .source = state->buffer,
            .idx_start = cursor_start,
            .idx_end = state->cursor
        },
        .type = TT_SPACE
    });
}

inline static bool merge_collected_top(token_collection_t* collection) {
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

inline static bool tokenizer_analyze(token_collection_t* collection, char* text, size_t text_length, char* filepath) {

    tokenizer_state_t state = (tokenizer_state_t) {
        .buffer = text,
        .buffer_size = text_length,
        .filepath = filepath,
        .cursor = 0
    };

    while ( state.cursor < state.buffer_size ) {
        
        size_t last_cursor_pos = state.cursor;

        try_collect_space(collection, &state);
        try_collect_comment(collection, &state);
        try_collect_string(collection, &state);
        try_collect_number(collection, &state);

        if( last_cursor_pos == state.cursor ) {
            return false;
        }
    }

    return true;
}



#endif // GVM_TOKENIZER_H_