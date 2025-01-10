#ifndef GVM_SRCMAP_H_
#define GVM_SRCMAP_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "co_utils.h"

typedef struct srcmap_value_t {
    uint32_t    data;
} srcmap_value_t;

typedef struct srcmap_t {
    size_t              count;
    size_t              capacity;
    srcref_t*           keys;
    srcmap_value_t*     values;
} srcmap_t;

inline static srcmap_value_t sm_val(uint32_t data) {
    return (srcmap_value_t) {
        .data = data
    };
}

bool srcmap_init(srcmap_t* map, size_t initial_capacity);
void srcmap_destroy(srcmap_t* map);
bool srcmap_insert(srcmap_t* map, srcref_t key, srcmap_value_t val);
void srcmap_clear(srcmap_t* map);
void srcmap_print(cstr_t str, srcmap_t* map);
srcmap_value_t* srcmap_lookup(srcmap_t* map, srcref_t key);

#endif // GVM_SRCMAP_H_
