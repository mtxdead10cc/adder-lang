#ifndef CO_TYPECHECK_H_
#define CO_TYPECHECK_H_

#include <stdint.h>
#include <stdbool.h>
#include "sh_types.h"
#include "co_types.h"
#include "co_cres.h"

/*

type := int
      | float
      | char
      | bool
      | array<type>
      | tuple<type, ..., type>
      | NAME (type, ..., type) -> type

string is array<char>
coord is tuple<int, int>

The fundamental types are: 
    int, char, bool, float,
    tuple, list and function.

*/

typedef enum kind_tag_t {
    KNONE,
    KINT,
    KCHAR,
    KBOOL,
    KFLOAT,
    KLIST,
    KTUPLE,
    KFUNCTION
} kind_tag_t;

typedef struct kind_t kind_t;

typedef struct klist_t {
    kind_t* kind;
} klist_t;

typedef struct ktuple_t {
    size_t field_count;
    kind_t** field_kind;
} ktuple_t;

typedef struct kfunction_t {
    size_t arg_count;
    kind_t** arg_kind;
    kind_t* kind;
} kfunction_t;

typedef struct kind_t {
    kind_tag_t  content_tag;
    srcref_t    ref;
    union {
        klist_t     list;
        ktuple_t    tuple;
        kfunction_t function;
    } u;
} kind_t;

inline static bool kind_check(kind_t* a, kind_t* b) {
    
    if( a->content_tag != b->content_tag ) {
        return false;
    }

    kind_tag_t tag = a->content_tag;
    
    switch(tag) {
        case KLIST: {
            if( kind_check(a->u.list.kind, b->u.list.kind) == false ) {
                return false;
            }
            return true;
        } break;
        case KTUPLE: {
            ktuple_t a_tuple = a->u.tuple;
            ktuple_t b_tuple = b->u.tuple;
            if( a_tuple.field_count != b_tuple.field_count ) {
                return false;
            }
            size_t count = a_tuple.field_count;
            for (size_t i = 0; i < count; i++) {
                kind_t* tup_a_i = a_tuple.field_kind[i];
                kind_t* tup_b_i = b_tuple.field_kind[i];
                if( kind_check(tup_a_i, tup_b_i) == false ) {
                    return false;
                }
            }
            return true;
        } break;
        case KFUNCTION: {
            kfunction_t af = a->u.function;
            kfunction_t bf = b->u.function;
            if( af.arg_count != bf.arg_count ) {
                return false;
            }
            size_t arg_count = af.arg_count;
            for (size_t i = 0; i < arg_count; i++) {
                kind_t* af_i = af.arg_kind[i];
                kind_t* bf_i = bf.arg_kind[i];
                if( kind_check(af_i, bf_i) == false ) {
                    return false;
                }
            }
            if( kind_check(af.kind, bf.kind) == false ) {
                return false;
            }
            return true;
        } break;
        default: break;
    }

    return true;
}

#endif // CO_TYPECHECK_H_