#ifndef SH_PARSE_TOOLS_H_
#define SH_PARSE_TOOLS_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct pt_token_t {
    char*       text;
    ptrdiff_t   length;
    int64_t     type;
} pt_token_t;

typedef struct pt_state_t {
    pt_token_t* tokens;
    ptrdiff_t   index;
    ptrdiff_t   token_count;
} pt_state_t;

typedef ptrdiff_t (*pt_token_gen_t)(char* text, ptrdiff_t remaining, int64_t* type_out);

bool pt_string_equal(char* a, ptrdiff_t a_len, const char* b, ptrdiff_t b_len);
bool pt_char_is_any_of(char c, const char* chars, ptrdiff_t len);
bool pt_string_contains_char(char* str, ptrdiff_t str_len, char c);

#define pt_str_const_len(S) (sizeof(S)/sizeof(S[0])-1)
#define pt_str_eq(A, AL, B) pt_string_equal((A), (AL), (B), pt_str_const_len(B))
#define pt_char_any_of(C, S) pt_char_is_any_of((C), (S), pt_str_const_len(S))
#define pt_char_whitespace(C) pt_char_any_of(C, " \n\t\r\f")
#define pt_char_number(C) ((C) >= '0' && (C) <= '9')

ptrdiff_t pt_seek_next_non_whitespace(char* text, ptrdiff_t remaining);
ptrdiff_t pt_scan_string(char* text, ptrdiff_t remaining);
ptrdiff_t pt_scan_number(char* text, ptrdiff_t remaining);

typedef enum pt_result_code_t {
    PT_RES_INCOMPLETE       = -1,
    PT_RES_OK               =  0,
    PT_RES_ERROR_MEMORY     =  1,
    PT_RES_ERROR_PARAMETER  =  2
} pt_result_code_t;

pt_result_code_t pt_state_init(pt_state_t* state, pt_token_gen_t tokr, char* text, ptrdiff_t text_length);
pt_token_t pt_state_peek(pt_state_t* state, ptrdiff_t offs);
bool pt_state_has_tokens(pt_state_t* state);
bool pt_state_advance(pt_state_t* state);
bool pt_state_advance_if(pt_state_t* state, int64_t tt);
bool pt_state_match_any(pt_state_t* state, int64_t tts);
void pt_state_free(pt_state_t* state);


#endif // SH_PARSE_TOOLS_H_