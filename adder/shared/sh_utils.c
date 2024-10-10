#include "sh_utils.h"
#include "sh_config.h"
#include "sh_value.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

sstr_t sstr(char* str) {
    sstr_t sstr = { 0 };
    size_t len = strlen(str);
    assert(len < GVM_DEFAULT_STRLEN);
    len = min(len, GVM_DEFAULT_STRLEN);
    strncpy(sstr.str, str, len);
    return sstr;
}

bool sstr_equal_str(sstr_t* sstr, char* str) {
    if( sstr->str[0] != str[0] )
        return false;
    return strncmp(sstr->str, str, GVM_DEFAULT_STRLEN) == 0;
}

bool sstr_equal(sstr_t* a, sstr_t* b) {
    if( a->str[0] != b->str[0] )
        return false;
    return strncmp(a->str, b->str, GVM_DEFAULT_STRLEN) == 0;
}