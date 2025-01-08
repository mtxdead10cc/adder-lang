#include "sh_ffi.h"
#include "sh_ift.h"
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

bool ffi_native_exports_init(ffi_native_exports_t* hosted, int capacity) {
    hosted->capacity = capacity;
    hosted->count = 0;
    hosted->def = malloc( capacity * sizeof(ffi_definition_t) );
    hosted->handle = malloc( capacity * sizeof(ffi_handle_t) );
    return hosted->handle != NULL && hosted->def != NULL;
}

bool ffi_definition_set_init(ffi_definition_set_t* set, int capacity) {
    set->capacity = capacity;
    set->count = 0;
    set->def = malloc( capacity * sizeof(ffi_definition_t) );
    return set->def != NULL;
}

bool ffi_init(ffi_t* ffi) {

    const int capacity = 8;

    if(ffi_native_exports_init(&ffi->supplied, capacity) == false ) {
        ffi_native_exports_destroy(&ffi->supplied);
        return false;
    }

    return true;
}

int ffi_native_exports_index_of(ffi_native_exports_t* host, sstr_t name) {
    int count = host->count;
    for(int i = 0; i < count; i++) {
        if( sstr_equal(&host->def[i].name, &name) )
            return i;
    }
    return -1;
}

bool ffi_native_exports_define(ffi_native_exports_t* hosted, sstr_t name, ffi_handle_t handle, ift_t type) {
    
    int index = ffi_native_exports_index_of(hosted, name);
    if( index >= 0 )
        return false; // already defined

    if( hosted->count >= hosted->capacity ) {
        int new_cap = hosted->count * 2;
        ffi_definition_t* defs = (ffi_definition_t*) realloc(hosted->def, new_cap * sizeof(ffi_definition_t) );
        ffi_handle_t* handles = (ffi_handle_t*) realloc(hosted->handle, new_cap * sizeof(ffi_handle_t));
        
        if( defs != NULL )
            hosted->def = defs;
        
        if( handles != NULL )
            hosted->handle = handles;

        if( defs == NULL || handles == NULL)
            return false;
        
        hosted->capacity = new_cap;
    }

    hosted->def[hosted->count] = (ffi_definition_t) {
        .name = name,
        .type = type
    };

    hosted->handle[hosted->count] = handle;
    hosted->count ++;
    return true;
}

ift_t* ffi_native_exports_get_type(ffi_native_exports_t* host, sstr_t name) {
    int index = ffi_native_exports_index_of(host, name);
    if( index >= 0 )
        return &host->def[index].type;
    return NULL;
}

int ffi_definition_set_index_of(ffi_definition_set_t* set, sstr_t name) {
    int count = set->count;
    for(int i = 0; i < count; i++) {
        if( sstr_equal(&set->def[i].name, &name) )
            return i;
    }
    return -1;
}

bool ffi_definition_set_add(ffi_definition_set_t* set, sstr_t name, ift_t type) {
    // TODO: Verify that handle and type matches
    int index = ffi_definition_set_index_of(set, name);
    if( index >= 0 )
        return false;

    if( set->count >= set->capacity ) {
        int new_cap = set->count * 2;
        ffi_definition_t* defs = realloc(set->def, new_cap * sizeof(ffi_definition_t));
        if( defs != NULL ) {
            set->capacity = new_cap;
            set->def = defs;
        } else {
            return false;
        }
    }

    set->def[set->count] = (ffi_definition_t) {
        .name = name,
        .type = type
    };

    set->count ++;
    return true;
}

ift_t* ffi_definition_set_get_type(ffi_definition_set_t* set, sstr_t name) {
    int index = ffi_definition_set_index_of(set, name);
    if( index >= 0 )
        return &set->def[index].type;
    return NULL;
}

void ffi_native_exports_destroy(ffi_native_exports_t* hosted) {
    if(hosted->def != NULL) {
        free(hosted->def);
        hosted->def = NULL;
    }
    if( hosted->handle != NULL ) {
        free(hosted->handle);
        hosted->handle = NULL;
    }
}

void ffi_definition_set_destroy(ffi_definition_set_t* set) {
    if(set->def != NULL) {
        free(set->def);
        set->def = NULL;
    }
}

void ffi_destroy(ffi_t* ffi) {
    ffi_native_exports_destroy(&ffi->supplied);
}

void ffi_fprint(FILE* f, ffi_t* ffi) {
    fprintf(f, "FFI\n");
    fprintf(f, " Supplied (by host)\n");
    for(int i = 0; i < ffi->supplied.count; i++) {
        sstr_t v = ift_type_to_sstr(ffi->supplied.def[i].type);
        fprintf(f, "\t%.*s: %.*s\n",
            sstr_len(&ffi->supplied.def[i].name),
            sstr_ptr(&ffi->supplied.def[i].name),
            sstr_len(&v),
            sstr_ptr(&v));
    }
}
