#include "gvm_srcmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    map->keys = (srcref_t*) malloc(sizeof(srcref_t) * map->capacity);
    if( map->keys == NULL ) {
        srcmap_destroy(map);
        return false;
    }
    map->values = (srcmap_value_t*) malloc(sizeof(srcmap_value_t) * map->capacity);
    if( map->values == NULL ) {
        srcmap_destroy(map);
        return false;
    }
    memset(map->keys, 0, sizeof(srcref_t) * map->capacity);
    return true;
}

size_t srcmap_hash(srcref_t ref) {
    size_t len = srcref_len(ref);
    size_t hash_code = len + 5;
    char* keystr = srcref_ptr(ref); 
    for(size_t i = 0; i < len; i++) {
        hash_code += (hash_code + keystr[i]) * 7919U;
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
            if( map->keys[i].source == NULL ) {
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

        map->keys = new_map.keys;
        map->values = new_map.values;
        map->capacity = new_map.capacity;
        map->count = new_map.count;
    }

    return true;
}

bool srcmap_insert(srcmap_t* map, srcref_t key, srcmap_value_t val) {
    if( srcmap_ensure_capacity(map, 1) == false ) {
        return false;
    }
    size_t hk = srcmap_hash(key);
    size_t start_index = hk % map->capacity;
    for(size_t i = 0; i < map->capacity; i++) {
        size_t tab_index = (i + start_index) % map->capacity;
        if( map->keys[tab_index].source == NULL ) {
            map->values[tab_index] = val;
            map->keys[tab_index] = key;
            map->count ++;
            return true;
        } else if ( srcref_equals(map->keys[tab_index], key) ) {
            return false;
        }
    }
    return false;
}

void srcmap_clear(srcmap_t* map) {
    memset(map->keys, 0, sizeof(srcref_t) * map->capacity);
    map->count = 0;
}

void srcmap_print(srcmap_t* map) {
    printf("[srcmap_t (size=%d)]\n", (uint32_t) map->count);
    for(size_t i = 0; i < map->capacity; i++) {
        printf("%i > ", (uint32_t) i);
        if( map->keys[i].source == NULL ) {
            printf("<empty>");
        } else {
            srcref_print(map->keys[i]);
        }
        printf("\n");
    }
}

srcmap_value_t* srcmap_lookup(srcmap_t* map, srcref_t key) {
    size_t hk = srcmap_hash(key);
    size_t start_index = hk % map->capacity;
    for(size_t i = 0; i < map->capacity; i++) {
        size_t tab_index = (i + start_index) % map->capacity;
        if( map->keys[tab_index].source == NULL ) {
            return NULL;
        }
        if( srcref_equals(map->keys[tab_index], key) ) {
            return &map->values[tab_index];
        }
    }
    return NULL;
}