#include "sh_ffi.h"
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
    ffi->u.func.arg_types = malloc(sizeof(ffi_type_t));
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


