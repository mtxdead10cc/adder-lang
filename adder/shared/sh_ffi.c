#include "sh_ffi.h"
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

ffi_type_t* ffi_const(sstr_t type_name) {
    ffi_type_t* ffi = malloc(sizeof(ffi_type_t));
    ffi->tag = FFI_TYPE_CONST;
    ffi->u.cons.type_name = type_name;
    return ffi;
}

ffi_type_t* ffi_list(ffi_type_t* content_type) {
    ffi_type_t* ffi = malloc(sizeof(ffi_type_t));
    ffi->tag = FFI_TYPE_LIST;
    ffi->u.list.content_type = content_type;
    return ffi;
}

ffi_type_t* ffi_func(ffi_type_t* return_type) {
    ffi_type_t* ffi = malloc(sizeof(ffi_type_t));
    ffi->tag = FFI_TYPE_FUNC;
    ffi->u.func.return_type = return_type;
    ffi->u.func.arg_types = malloc(sizeof(ffi_type_t*));
    ffi->u.func.arg_count = 0;
    return ffi;
}

void ffi_func_add_arg(ffi_type_t* func, ffi_type_t* arg_type) {
    assert(func->tag == FFI_TYPE_FUNC);
    ffi_type_t** extended = realloc(func->u.func.arg_types,
        sizeof(ffi_type_t) * (1 + func->u.func.arg_count));
    if( extended == NULL ) {
        printf("error: (ffi_func_add_arg) out of memory\n");
        return;
    }
    func->u.func.arg_types = extended;
    func->u.func.arg_types[func->u.func.arg_count] = arg_type;
    func->u.func.arg_count += 1;
}

ffi_type_t* ffi_vfunc_nullterm(ffi_type_t* return_type, ...) {
    va_list args;
    va_start(args, return_type);
    ffi_type_t* func = ffi_func(return_type);
    ffi_type_t* arg_type = NULL;
    while( (arg_type = va_arg(args, ffi_type_t*)) != NULL ) {
        ffi_func_add_arg(func, arg_type);
    }
    va_end(args);
    return func;
}

void ffi_recfree(ffi_type_t* ffi) {
    if( ffi == NULL )
        return;
    switch(ffi->tag) {
        case FFI_TYPE_CONST: {
            free(ffi);
        } break;
        case FFI_TYPE_LIST: {
            ffi_recfree(ffi->u.list.content_type);
            ffi->u.list.content_type = NULL;
            free(ffi);
        } break;
        case FFI_TYPE_FUNC: {
            int count = ffi->u.func.arg_count;
            for(int i = 0; i < count; i++) {
                ffi_recfree(ffi->u.func.arg_types[i]);
                ffi->u.func.arg_types[i] = NULL;
            }
            ffi_recfree(ffi->u.func.return_type);
            ffi->u.func.return_type = NULL;
            free(ffi);
        } break;
    }
}

void ffi_fprint(FILE* f, ffi_type_t* ffi) {
    if( ffi == NULL ) {
        fprintf(f, "NULL");
        return;
    }
    switch(ffi->tag) {
        case FFI_TYPE_CONST: {
            fprintf(f, "%.*s",
                sstr_len(&ffi->u.cons.type_name),
                sstr_ptr(&ffi->u.cons.type_name));
        } break;
        case FFI_TYPE_LIST: {
            fprintf(f, "array<");
            ffi_fprint(f, ffi->u.list.content_type);
            fprintf(f, ">");
        } break;
        case FFI_TYPE_FUNC: {
            fprintf(f, "(");
            int count = ffi->u.func.arg_count;
            for(int i = 0; i < count; i++) {
                if( i > 0 )
                    fprintf(f, ", ");
                ffi_fprint(f, ffi->u.func.arg_types[i]);
            }
            fprintf(f, ") -> ");
            ffi_fprint(f, ffi->u.func.return_type);
        } break;
    }
}

bool ffi_equals(ffi_type_t* a, ffi_type_t* b) {
    if( a == NULL || b == NULL )
        return false;
    if( a->tag != b->tag )
        return false;
    switch (a->tag) {
        case FFI_TYPE_CONST: {
            sstr_t* a_type_name = &a->u.cons.type_name;
            sstr_t* b_type_name = &b->u.cons.type_name;
            return sstr_equal(a_type_name, b_type_name);
        } break;
        case FFI_TYPE_LIST: {
            ffi_type_t* a_list_content = a->u.list.content_type;
            ffi_type_t* b_list_content = b->u.list.content_type;
            return ffi_equals(a_list_content, b_list_content);
        } break;
        case FFI_TYPE_FUNC: {
            int count = a->u.func.arg_count;
            if( count != b->u.func.arg_count )
                return false;
            for(int i = 0; i < count; i++) {
                ffi_type_t* a_arg_type = a->u.func.arg_types[i];
                ffi_type_t* b_arg_type = b->u.func.arg_types[i];
                if( ffi_equals(a_arg_type, b_arg_type) == false )
                    return false;
            }
            ffi_type_t* a_return_type = a->u.func.return_type;
            ffi_type_t* b_return_type = b->u.func.return_type;
            return ffi_equals(a_return_type, b_return_type);
        } break;
        default: {
            printf("unknown FFI type %d\n", a->tag);
            return false;
        }
    }
}

bool ffi_bundle_init(ffi_bundle_t* bundle, int capacity) {
    bundle->capacity = capacity;
    bundle->count = 0;
    bundle->name = malloc( capacity * sizeof(sstr_t) );
    bundle->type = malloc( capacity * sizeof(ffi_type_t*) );
    return bundle->name != NULL && bundle->type != NULL;
}

int ffi_bundle_index_of(ffi_bundle_t* bundle, sstr_t* name) {
    int count = bundle->count;
    for(int i = 0; i < count; i++) {
        if( sstr_equal(&bundle->name[i], name) )
            return i;
    }
    return -1;
}

bool ffi_bundle_add(ffi_bundle_t* bundle, sstr_t name, ffi_type_t* type) {
    int index = ffi_bundle_index_of(bundle, &name);
    if( index >= 0 )
        return ffi_equals(bundle->type[index], type);
    if( bundle->count >= bundle->capacity ) {
        int new_cap = bundle->count * 2;
        bundle->name = realloc(bundle->name, new_cap * sizeof(sstr_t));
        assert(bundle->name != NULL); // todo: handle fail
        bundle->type = realloc(bundle->type, new_cap * sizeof(ffi_type_t*));
        assert(bundle->type != NULL); // todo: handle fail
    }
    bundle->name[bundle->count] = name;
    bundle->type[bundle->count] = type;
    bundle->count ++;
    return true;
}

void ffi_bundle_destroy(ffi_bundle_t* bundle) {
    
    if(bundle->type != NULL) {
        int count = bundle->count;
        for(int i = 0; i < count; i++) {
            // note: this will break if types 
            // are reused (by the user).
            // should probably fix this at
            // some point
            ffi_recfree(bundle->type[i]);
            bundle->type[i] = NULL;
        }
        free(bundle->type);
    }

    if(bundle->name != NULL) {
        free(bundle->name);
    }
}

void ffi_bundle_fprint(FILE* f, ffi_bundle_t* bundle) {
    fprintf(f, "FFI BUNDLE\n");
    int count = bundle->count;
    for(int i = 0; i < count; i++) {
        fprintf(f, "\t%.*s: ",
            sstr_len(&bundle->name[i]),
            sstr_ptr(&bundle->name[i]));
        ffi_fprint(f, bundle->type[i]);
        fprintf(f, "\n");
    }
}


