#ifndef GVM_STRMATCH_H_
#define GVM_STRMATCH_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

inline static bool _sm_any_of(int c, int count, ...) {
    va_list ap;
    va_start(ap, count);
    while (count--) {
        if(va_arg(ap, int) == c) {
            return true;
        }
    }
    va_end(ap);
    return false;
}

inline static bool _sm_any_of_array(char c, int count, char* args) {
    for(int i = 0; i < count; i++){
        if( args[i] == c ) {
            return true;
        }
    }
    return false;
}

#define NUM_VA_ARGS(...)  (sizeof((int[]){__VA_ARGS__})/sizeof(int))
#define SM_IS_IN_RANGE(C, MIN, MAX) ((C) >= (MIN) && (C) <= (MAX))
#define SM_IS_ANY_OF(C, ...) _sm_any_of((C), NUM_VA_ARGS(__VA_ARGS__), __VA_ARGS__)
#define SM_IS_LETTER(C) (SM_IS_IN_RANGE(C, 'a', 'z') || SM_IS_IN_RANGE(C, 'A', 'Z'))
#define SM_IS_NUMBER(C) SM_IS_IN_RANGE(C, '0', '9')
#define SM_IS_NEWLINE(C) ((C) == '\n')
#define SM_IS_SPACE(C) (   (C) == ' '   \
                        || (C) == '\t'  \
                        || (C) == '\n'  \
                        || (C) == '\r' )

#endif // GVM_STRMATCH_H_