#include "adrcom/parser/co_srcmap.h"

#include <shared/sh_utils.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void srcmap_destroy(srcmap_t* map) {
    if( map == NULL ) {
        return;
    }
    if( map->keys != NULL ) {
        free(map->keys);
        map->keys = NULL;
    }
    if( map->values != NULL ) {
        free(map->values);
        map->values = NULL;
    }
    map->capacity = 0;
    map->count = 0;
}

bool srcmap_init(srcmap_t* map, size_t initial_capacity) {
    map->capacity = initial_capacity;
    map->count = 0;
    map->keys = (sstr_t*) calloc(map->capacity, sizeof(sstr_t));
    if( map->keys == NULL ) {
        srcmap_destroy(map);
        return false;
    }
    map->values = (srcmap_value_t*) calloc(map->capacity, sizeof(srcmap_value_t));
    if( map->values == NULL ) {
        srcmap_destroy(map);
        return false;
    }
    return true;
}

size_t srcmap_hash(sstr_t* sstr) {
    size_t len = sstr_plen(sstr);
    size_t hash_code = len + 5;
    for(size_t i = 0; i < len; i++) {
        hash_code += (hash_code + sstr->str[i]) * 7919U;
    }
    return hash_code;
}

bool srcmap_ensure_capacity(srcmap_t* map, size_t additional) {

    // this is a map and not a list so we try to
    // have some headroom.
    
    size_t required = (map->count + additional); 
    if( map->capacity <= (required + (required / 4)) ) {
        size_t new_capacity = required * 2;

        srcmap_t new_map;
        if( srcmap_init(&new_map, new_capacity) == false ) {
            return false;
        }

        bool error_occurred = false;
        for (size_t i = 0; i < map->capacity; i++) {
            if( sstr_is_empty(map->keys[i]) ) {
                continue;
            }
            bool insert_ok = srcmap_insert(&new_map, map->keys[i], map->values[i]);
            bool count_ok = new_map.count < required;
            if( insert_ok == false || count_ok == false ) {
                error_occurred = true;
                break;
            }
        }

        if( error_occurred ) {
            srcmap_destroy(&new_map);
            return false;
        }
        
        free(map->keys);
        free(map->values);

        assert(new_map.count == map->count);

        map->keys = new_map.keys;
        map->values = new_map.values;
        map->capacity = new_map.capacity;
        map->count = new_map.count;
    }

    return true;
}

bool srcmap_insert(srcmap_t* map, sstr_t key, srcmap_value_t val) {
    if( srcmap_ensure_capacity(map, 1) == false ) {
        return false;
    }
    size_t hk = srcmap_hash(&key);
    size_t start_index = hk % map->capacity;
    for(size_t i = 0; i < map->capacity; i++) {
        size_t tab_index = (i + start_index) % map->capacity;
        if( sstr_is_empty(map->keys[tab_index]) ) {
            map->values[tab_index] = val;
            map->keys[tab_index] = key;
            map->count ++;
            return true;
        } else if ( sstr_compare(map->keys[tab_index], key) == 0 ) {
            return false;
        }
    }
    return false;
}

void srcmap_clear(srcmap_t* map) {
    memset(map->keys, 0, sizeof(sstr_t) * map->capacity);
    map->count = 0;
}

void srcmap_print(cstr_t str, srcmap_t* map) {
    cstr_append_fmt(str, "[srcmap_t (size=%d)]\n", (uint32_t) map->count);
    for(size_t i = 0; i < map->capacity; i++) {
        cstr_append_fmt(str, "%i > ", (uint32_t) i);
        if( sstr_is_empty(map->keys[i]) ) {
            cstr_append_fmt(str, "<empty>");
        } else {
            cstr_append_fmt(str, "%s", sstr_ptr(map->keys[i]));
        }
        cstr_append_fmt(str, "\n");
    }
}

srcmap_value_t* srcmap_lookup(srcmap_t* map, sstr_t key) {
    size_t hk = srcmap_hash(&key);
    size_t start_index = hk % map->capacity;
    for(size_t i = 0; i < map->capacity; i++) {
        size_t tab_index = (i + start_index) % map->capacity;
        if( sstr_is_empty(map->keys[tab_index]) ) {
            return NULL;
        }
        if( sstr_compare(key, map->keys[tab_index]) == 0 ) {
            return &map->values[tab_index];
        }
    }
    return NULL;
}
