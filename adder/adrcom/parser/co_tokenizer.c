#include "adrcom/parser/co_tokenizer.h"

#include "adrcom/parser/co_parser_types.h"

#include <adrcom/shared/co_trace.h>
#include <adrcom/parser/co_srcmap.h>

#include <shared/sh_utils.h>

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

void tokens_sprint(cstr_t str, token_collection_t* collection) {
    for(size_t i = 0; i < collection->count; i++) {
        cstr_append_fmt(str, "(%s '", token_get_type_name(collection->tokens[i].type));
        srcref_sprint(str, collection->tokens[i].ref);
        cstr_append_fmt(str, "')\n");
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
    char*           buffer;
    size_t          buffer_size;
    char*           filepath;
    size_t          cursor;
    srcmap_t        kw_map_alpha;
    srcmap_t        kw_map_symbolic;
} tokenizer_state_t;

size_t sweep_while(tokenizer_state_t* state, size_t start_offset, lex_predicate_t lex) {
    size_t cursor = state->cursor + start_offset;
    size_t stop = state->buffer_size - 1;
    while ( cursor <= stop && lexer_match(lex, lexer_scan(state->buffer[cursor])) ) {
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

token_t sweep_make_token(
    tokenizer_state_t* state,
    size_t offset,
    size_t trailing,
    lex_predicate_t while_true,
    token_type_t token_type)
{
    size_t start = state->cursor;
    size_t len = sweep_while(state, offset, while_true);
    // assert(len >= 0 && "unexpected sweep_while progress"); empty strings
    len = len + offset + trailing;
    state->cursor = start + len;
    return (token_t) {
        .ref = srcref(state->buffer, start, len),
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
    //assert(len > 0 && "unexpected sweep_while progress");
    len = len + offset + trailing;
    state->cursor = start + len;
}

srcmap_t create_keyword_token_map(void) {
    srcmap_t map;
    srcmap_init(&map, 50);

    srcmap_insert(&map, srcref_const("if"),     sm_val(TT_KW_IF));
    srcmap_insert(&map, srcref_const("else"),   sm_val(TT_KW_ELSE));
    srcmap_insert(&map, srcref_const("for"),    sm_val(TT_KW_FOR));
    srcmap_insert(&map, srcref_const("in"),     sm_val(TT_KW_IN));
    srcmap_insert(&map, srcref_const("fun"),    sm_val(TT_KW_FUN_DEF));
    srcmap_insert(&map, srcref_const("return"), sm_val(TT_KW_RETURN));
    srcmap_insert(&map, srcref_const("break"),  sm_val(TT_KW_BREAK));
    srcmap_insert(&map, srcref_const("true"),   sm_val(TT_BOOLEAN));
    srcmap_insert(&map, srcref_const("false"),  sm_val(TT_BOOLEAN));
    srcmap_insert(&map, srcref_const("not"),    sm_val(TT_UNOP_NOT));
    srcmap_insert(&map, srcref_const("and"),    sm_val(TT_BINOP_AND));
    srcmap_insert(&map, srcref_const("or"),     sm_val(TT_BINOP_OR));
    srcmap_insert(&map, srcref_const("import"), sm_val(TT_IMPORT));
    srcmap_insert(&map, srcref_const("export"), sm_val(TT_EXPORT));
    
    return map;
}

token_t lookup_alpha_token(tokenizer_state_t* state) {
    size_t start = state->cursor;
    size_t len = sweep_while(state, 0,
        lp_is(LCAT_LETTER|LCAT_UNDERSCORE|LCAT_NUMBER));
    assert(len > 0 && "unexpected sweep_while progress");
    state->cursor = start + len;
    srcref_t ref = srcref(state->buffer, start, len);
    srcmap_value_t* lookup_result = srcmap_lookup(&state->kw_map_alpha, ref);
    return (token_t) {
        .ref = ref,
        .type = lookup_result != NULL ? lookup_result->data : TT_SYMBOL
    };
}

srcmap_t create_symbolic_token_map(void) {
    srcmap_t map;
    srcmap_init(&map, 50);

    srcmap_insert(&map, srcref_const("->"), sm_val(TT_ARROW));
    srcmap_insert(&map, srcref_const("=="), sm_val(TT_CMP_EQ));
    srcmap_insert(&map, srcref_const("!="), sm_val(TT_CMP_NEQ));
    srcmap_insert(&map, srcref_const("<="), sm_val(TT_CMP_LT_EQ));
    srcmap_insert(&map, srcref_const(">="), sm_val(TT_CMP_GT_EQ));
    srcmap_insert(&map, srcref_const("<"),  sm_val(TT_CMP_LT));
    srcmap_insert(&map, srcref_const(">"),  sm_val(TT_CMP_GT));
    srcmap_insert(&map, srcref_const("("),  sm_val(TT_OPEN_PAREN));
    srcmap_insert(&map, srcref_const(")"),  sm_val(TT_CLOSE_PAREN));
    srcmap_insert(&map, srcref_const("{"),  sm_val(TT_OPEN_CURLY));
    srcmap_insert(&map, srcref_const("}"),  sm_val(TT_CLOSE_CURLY));
    srcmap_insert(&map, srcref_const("["),  sm_val(TT_OPEN_SBRACKET));
    srcmap_insert(&map, srcref_const("]"),  sm_val(TT_CLOSE_SBRACKET));
    srcmap_insert(&map, srcref_const("="),  sm_val(TT_ASSIGN));
    srcmap_insert(&map, srcref_const(","),  sm_val(TT_SEPARATOR));
    srcmap_insert(&map, srcref_const("#"),  sm_val(TT_HASH_SIGN));
    srcmap_insert(&map, srcref_const(";"),  sm_val(TT_STATEMENT_END));
    srcmap_insert(&map, srcref_const("*"),  sm_val(TT_BINOP_MUL));
    srcmap_insert(&map, srcref_const("/"),  sm_val(TT_BINOP_DIV));
    srcmap_insert(&map, srcref_const("%"),  sm_val(TT_BINOP_MOD));
    srcmap_insert(&map, srcref_const("+"),  sm_val(TT_BINOP_PLUS));
    srcmap_insert(&map, srcref_const("-"),  sm_val(TT_BINOP_MINUS));

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
            lp_is(LCAT_EQUAL|LCAT_LESS_THAN|LCAT_GREATER_THAN|LCAT_BANG),
            lp_is(LCAT_EQUAL)))
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
    srcref_t ref = srcref(state->buffer, start, len);
    srcmap_value_t* lookup_result = srcmap_lookup(&state->kw_map_symbolic, ref);
    return (token_t) {
        .ref = ref,
        .type = lookup_result != NULL ? lookup_result->data : TT_SYMBOL
    };
}

void destroy_token_map(srcmap_t* map) {
    srcmap_destroy(map);
}

srcref_t get_current_srcref(tokenizer_state_t* state) {
    size_t next_index = min(state->cursor + 1, state->buffer_size-1);
    return srcref(state->buffer, state->cursor, next_index);
}

bool tokenizer_analyze(token_collection_t* collection, tokenizer_args_t* args) {

    tokenizer_state_t state = (tokenizer_state_t) {
        .buffer = args->text,
        .buffer_size = args->text_length,
        .cursor = 0,
        .kw_map_alpha = create_keyword_token_map(),
        .kw_map_symbolic = create_symbolic_token_map()
    };

    if( tokens_append(collection, (token_t){TT_INITIAL, srcref(args->text, 0, 0)}) == false ) {
        trace_out_of_memory_error(args->trace);
        return false;
    }

    while ( state.cursor < state.buffer_size && trace_get_error_count(args->trace) == 0 ) {
        
        size_t last_cursor_pos = state.cursor;
        bool alloc_ok = true;
        
        if(  match_cursor(&state, lp_is(LCAT_SPACE)) ) {                                           // SPACE
            if( args->include_spaces ) {
                alloc_ok = tokens_append(collection,
                    sweep_make_token(&state, 0, 0, lp_is(LCAT_SPACE), TT_SPACE));
            } else {
                sweep_discard_token(&state, 0, 0, lp_is(LCAT_SPACE));
            }
        } else if( match_cursor_and_next(&state, lp_is(LCAT_SLASH), lp_is(LCAT_SLASH)) ) {         // COMMENT
            if( args->include_comments ) {
                alloc_ok = tokens_append(collection,
                    sweep_make_token(&state, 2, 1, lp_is_not(LCAT_NEWLINE), TT_COMMENT));
            } else {
                sweep_discard_token(&state, 2, 1, lp_is_not(LCAT_NEWLINE));
            }
        } else if ( match_cursor(&state, lp_is(LCAT_QUOTE)) ) {                                     // STRING
            alloc_ok = tokens_append(collection,
                sweep_make_token(&state, 1, 1, lp_is_not(LCAT_QUOTE), TT_STRING));
        } else if ( match_cursor(&state, lp_is(LCAT_NUMBER)) ) {                                    // NUMBER
            alloc_ok = tokens_append(collection,
                sweep_make_token(&state, 0, 0, lp_is(LCAT_NUMBER|LCAT_DOT), TT_NUMBER));
        } else if ( match_cursor(&state, lp_is(LCAT_LETTER|LCAT_UNDERSCORE)) ) {
            alloc_ok = tokens_append(collection, lookup_alpha_token(&state));                       // IDENTIFIER
        } else if ( match_cursor(&state, lp_is(LCAT_SYMBOLIC)) ) {
            alloc_ok = tokens_append(collection, lookup_symbolic_token(&state));                    // SYMBOLIC
        }

        if( alloc_ok == false ) {
            trace_out_of_memory_error(args->trace);
        } else if( last_cursor_pos == state.cursor ) {
            trace_msg_t* msg = trace_create_message(args->trace, TM_ERROR, get_current_srcref(&state));
            trace_msg_append_costr(msg, "failed to make sense of input text.");
        }
    }

    destroy_token_map(&state.kw_map_alpha);
    destroy_token_map(&state.kw_map_symbolic);

    if( tokens_append(collection, (token_t) {TT_FINAL, get_current_srcref(&state)}) == false ) {
        trace_out_of_memory_error(args->trace);
    }

    return trace_get_error_count(args->trace) == 0;
}



int tokenizer_trace_msg_append_token_type_name(trace_msg_t* msg, token_type_t type) {
    switch (type) {
        case TT_INITIAL:        return trace_msg_append_costr(msg, "initial token");
        case TT_SPACE:          return trace_msg_append_costr(msg, "space");
        case TT_COMMENT:        return trace_msg_append_costr(msg, "comment");
        case TT_SYMBOL:         return trace_msg_append_costr(msg, "symbol");
        case TT_NUMBER:         return trace_msg_append_costr(msg, "number");
        case TT_BOOLEAN:        return trace_msg_append_costr(msg, "bool");
        case TT_STRING:         return trace_msg_append_costr(msg, "string");
        case TT_SEPARATOR:      return trace_msg_append_costr(msg, "separator");
        case TT_STATEMENT_END:  return trace_msg_append_costr(msg, "end of statement");
        case TT_ARROW:          return trace_msg_append_costr(msg, "function return type arrow");
        case TT_ASSIGN:         return trace_msg_append_costr(msg, "assignment");
        case TT_CMP_LT:         return trace_msg_append_costr(msg, "opening angle bracket alt. less than");
        case TT_CMP_GT:         return trace_msg_append_costr(msg, "closing angle bracket alt. greater than");
        case TT_OPEN_PAREN:     return trace_msg_append_costr(msg, "left parenthesis");
        case TT_CLOSE_PAREN:    return trace_msg_append_costr(msg, "right parenthesis");
        case TT_OPEN_CURLY:     return trace_msg_append_costr(msg, "opening brace");
        case TT_CLOSE_CURLY:    return trace_msg_append_costr(msg, "closing brace");
        case TT_OPEN_SBRACKET:  return trace_msg_append_costr(msg, "opening square bracket");
        case TT_CLOSE_SBRACKET: return trace_msg_append_costr(msg, "closing square bracket");
        case TT_BINOP_AND:      return trace_msg_append_costr(msg, "binary and-operator");
        case TT_BINOP_OR:       return trace_msg_append_costr(msg, "binary or-operator");
        case TT_UNOP_NOT:       return trace_msg_append_costr(msg, "unary not-operator");
        case TT_FINAL:          return trace_msg_append_costr(msg, "final token");
        case TT_NOTHING:        return trace_msg_append_costr(msg, "nothing");
        case TT_BINOP_MUL:      return trace_msg_append_costr(msg, "multiply operator");
        case TT_BINOP_DIV:      return trace_msg_append_costr(msg, "division operator");
        case TT_BINOP_MOD:      return trace_msg_append_costr(msg, "modulus operator");
        case TT_BINOP_PLUS:     return trace_msg_append_costr(msg, "binary plus-operator");
        case TT_BINOP_MINUS:    return trace_msg_append_costr(msg, "binary minus-operator");
        case TT_KW_IF:
        case TT_KW_ELSE:
        case TT_KW_FOR:
        case TT_KW_RETURN:
        case TT_KW_FUN_DEF:     return trace_msg_append_costr(msg, "keyword");
        case TT_CMP_EQ:
        case TT_CMP_NEQ:
        case TT_CMP_GT_EQ:
        case TT_CMP_LT_EQ:      return trace_msg_append_costr(msg, "comparison operator");
        case TT_IMPORT:         return trace_msg_append_costr(msg, "from host to script import");
        case TT_EXPORT:         return trace_msg_append_costr(msg, "from script to host export");
        default:                return trace_msg_append_costr(msg, "unknown");
    }
}
