#include "sh_ffi.h"
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#define NPRE_DEF_CONSTS 5
static ffi_type_t ffi_consts[NPRE_DEF_CONSTS] = {
    {
        .tag = FFI_TYPE_CONST,
        .u.cons.type_name.str = "void"
    },
    {
        .tag = FFI_TYPE_CONST,
        .u.cons.type_name.str = "int"
    },
    {
        .tag = FFI_TYPE_CONST,
        .u.cons.type_name.str = "float"
    },
    {
        .tag = FFI_TYPE_CONST,
        .u.cons.type_name.str = "bool"
    },
    {
        .tag = FFI_TYPE_CONST,
        .u.cons.type_name.str = "char"
    }
};

ffi_type_t* ffi_void(void) {
    assert(NPRE_DEF_CONSTS == (sizeof(ffi_consts)/sizeof(ffi_consts[0])));
    assert(sstr_equal_str(&ffi_consts[0].u.cons.type_name, "void"));
    return &ffi_consts[0];
}

ffi_type_t* ffi_int(void) {
    assert(NPRE_DEF_CONSTS == (sizeof(ffi_consts)/sizeof(ffi_consts[0])));
    assert(sstr_equal_str(&ffi_consts[1].u.cons.type_name, "int"));
    return &ffi_consts[1];
}

ffi_type_t* ffi_float(void) {
    assert(NPRE_DEF_CONSTS == (sizeof(ffi_consts)/sizeof(ffi_consts[0])));
    assert(sstr_equal_str(&ffi_consts[2].u.cons.type_name, "float"));
    return &ffi_consts[2];
}

ffi_type_t* ffi_bool(void) {
    assert(NPRE_DEF_CONSTS == (sizeof(ffi_consts)/sizeof(ffi_consts[0])));
    assert(sstr_equal_str(&ffi_consts[3].u.cons.type_name, "bool"));
    return &ffi_consts[3];
}

ffi_type_t* ffi_char(void) {
    assert(NPRE_DEF_CONSTS == (sizeof(ffi_consts)/sizeof(ffi_consts[0])));
    assert(sstr_equal_str(&ffi_consts[4].u.cons.type_name, "char"));
    return &ffi_consts[4];
}

ffi_type_t* ffi_custom(char* type_name) {
    ffi_type_t* ffi = malloc(sizeof(ffi_type_t));
    ffi->tag = FFI_TYPE_CONST;
    ffi->u.cons.type_name = sstr(type_name);
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
    ffi->u.func.arg_types = NULL;
    ffi->u.func.arg_count = 0;
    return ffi;
}

void ffi_func_add_arg(ffi_type_t* ffi_func, ffi_type_t* arg_type) {
    assert(ffi_func->tag == FFI_TYPE_FUNC);
    ffi_functor_t* func = &ffi_func->u.func;
    if(func->arg_types == NULL) {
        assert(func->arg_count == 0);
        func->arg_types = (ffi_type_t**) malloc(sizeof(ffi_type_t*));
    } else {
        assert(func->arg_count > 0);
        func->arg_types = (ffi_type_t**) realloc((void*) func->arg_types,
            sizeof(ffi_type_t*) * (func->arg_count + 1));
    }
    func->arg_types[func->arg_count] = arg_type;
    func->arg_count ++;
}

ffi_type_t* ffi_vfunc_nullterm(ffi_type_t* return_type, ...) {
    ffi_type_t* func = ffi_func(return_type);
    va_list args;
    va_start(args, return_type);
    ffi_type_t* arg_type = NULL;
    while( (arg_type = va_arg(args, ffi_type_t*)) != NULL ) {
        ffi_func_add_arg(func, arg_type);
    }
    va_end(args);
    return func;
}

bool ffi_is_custom_const(ffi_type_t* t) {
    if( t->tag != FFI_TYPE_CONST )
        return false;
    for(int i = 0; i < NPRE_DEF_CONSTS; i++) {
        if( t == &ffi_consts[i] )
            return false;
    }
    return true;
}

void ffi_type_recfree(ffi_type_t* ffi) {
    if( ffi == NULL )
        return;
    switch(ffi->tag) {
        case FFI_TYPE_CONST: {
            if( ffi_is_custom_const(ffi) )
                free(ffi);
        } break;
        case FFI_TYPE_LIST: {
            ffi_type_recfree(ffi->u.list.content_type);
            ffi->u.list.content_type = NULL;
            free(ffi);
        } break;
        case FFI_TYPE_FUNC: {
            if( ffi->u.func.arg_types != NULL ) {
                int count = ffi->u.func.arg_count;
                for(int i = 0; i < count; i++) {
                    ffi_type_recfree(ffi->u.func.arg_types[i]);
                    ffi->u.func.arg_types[i] = NULL;
                }
                free(ffi->u.func.arg_types);
            }
            ffi->u.func.arg_types = NULL;
            ffi->u.func.arg_count = 0;
            ffi_type_recfree(ffi->u.func.return_type);
            ffi->u.func.return_type = NULL;
            free(ffi);
        } break;
    }
}

void ffi_type_fprint(FILE* f, ffi_type_t* ffi) {
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
            ffi_type_fprint(f, ffi->u.list.content_type);
            fprintf(f, ">");
        } break;
        case FFI_TYPE_FUNC: {
            fprintf(f, "(");
            int count = ffi->u.func.arg_count;
            for(int i = 0; i < count; i++) {
                if( i > 0 )
                    fprintf(f, ", ");
                ffi_type_fprint(f, ffi->u.func.arg_types[i]);
            }
            fprintf(f, ") -> ");
            ffi_type_fprint(f, ffi->u.func.return_type);
        } break;
    }
}

bool ffi_type_equals(ffi_type_t* a, ffi_type_t* b) {
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
            return ffi_type_equals(a_list_content, b_list_content);
        } break;
        case FFI_TYPE_FUNC: {
            int count = a->u.func.arg_count;
            if( count != b->u.func.arg_count )
                return false;
            for(int i = 0; i < count; i++) {
                ffi_type_t* a_arg_type = a->u.func.arg_types[i];
                ffi_type_t* b_arg_type = b->u.func.arg_types[i];
                if( ffi_type_equals(a_arg_type, b_arg_type) == false )
                    return false;
            }
            ffi_type_t* a_return_type = a->u.func.return_type;
            ffi_type_t* b_return_type = b->u.func.return_type;
            return ffi_type_equals(a_return_type, b_return_type);
        } break;
        default: {
            printf("unknown FFI type %d\n", a->tag);
            return false;
        }
    }
}

bool ffi_init(ffi_t* ffi) {

    const int capacity = 8;

    // HOST
    ffi->host.capacity = capacity;
    ffi->host.count = 0;
    ffi->host.name = malloc( capacity * sizeof(sstr_t) );
    ffi->host.type = malloc( capacity * sizeof(ffi_type_t*) );
    ffi->host.handle = malloc( capacity * sizeof(ffi_handle_t) );

    // EXE
    ffi->exe.capacity = capacity;
    ffi->exe.count = 0;
    ffi->exe.name = malloc( capacity * sizeof(sstr_t) );
    ffi->exe.type = malloc( capacity * sizeof(ffi_type_t*) );

    return ffi->host.name != NULL
        && ffi->host.type != NULL
        && ffi->host.handle != NULL
        && ffi->exe.name != NULL
        && ffi->exe.type != NULL;
}

int ffi_host_index_of(ffi_host_if_t* host, sstr_t name) {
    int count = host->count;
    for(int i = 0; i < count; i++) {
        if( sstr_equal(&host->name[i], &name) )
            return i;
    }
    return -1;
}

bool ffi_host_define(ffi_host_if_t* host, sstr_t name, ffi_handle_t handle, ffi_type_t* type) {
    // TODO: Verify that handle and type matches
    int index = ffi_host_index_of(host, name);
    if( index >= 0 )
        return ffi_type_equals(host->type[index], type);
    if( host->count >= host->capacity ) {
        int new_cap = host->count * 2;
        host->name = realloc(host->name, new_cap * sizeof(sstr_t));
        assert(host->name != NULL); // todo: handle fail
        host->type = realloc(host->type, new_cap * sizeof(ffi_type_t*));
        assert(host->type != NULL); // todo: handle fail
        host->handle = realloc(host->handle, new_cap * sizeof(ffi_handle_t));
        assert(host->handle != NULL); // todo: handle fail
    }
    host->name[host->count] = name;
    host->type[host->count] = type;
    host->handle[host->count] = handle;
    host->count ++;
    return true;
}

ffi_type_t* ffi_host_get_type(ffi_host_if_t* host, sstr_t name) {
    int index = ffi_host_index_of(host, name);
    if( index >= 0 )
        return host->type[index];
    return NULL;
}

int ffi_exe_index_of(ffi_exe_if_t* exe, sstr_t name) {
    int count = exe->count;
    for(int i = 0; i < count; i++) {
        if( sstr_equal(&exe->name[i], &name) )
            return i;
    }
    return -1;
}

bool ffi_exe_set_required_by_host(ffi_exe_if_t* exe, sstr_t name, ffi_type_t* type) {
    // TODO: Verify that handle and type matches
    int index = ffi_exe_index_of(exe, name);
    if( index >= 0 )
        return ffi_type_equals(exe->type[index], type);
    if( exe->count >= exe->capacity ) {
        int new_cap = exe->count * 2;
        exe->name = realloc(exe->name, new_cap * sizeof(sstr_t));
        assert(exe->name != NULL); // todo: handle fail
        exe->type = realloc(exe->type, new_cap * sizeof(ffi_type_t*));
        assert(exe->type != NULL); // todo: handle fail
    }
    exe->name[exe->count] = name;
    exe->type[exe->count] = type;
    exe->count ++;
    return true;
}

ffi_type_t* ffi_exe_get_type(ffi_exe_if_t* exe, sstr_t name) {
    int index = ffi_exe_index_of(exe, name);
    if( index >= 0 )
        return exe->type[index];
    return NULL;
}

void ffi_destroy(ffi_t* ffi) {
    
    if(ffi->host.type != NULL) {
        int count = ffi->host.count;
        for(int i = 0; i < count; i++) {
            ffi_type_recfree(ffi->host.type[i]);
            ffi->host.type[i] = NULL;
        }
        free(ffi->host.type);
        ffi->host.type = NULL;
    }

    if(ffi->host.name != NULL) {
        free(ffi->host.name);
        ffi->host.name = NULL;
    }

    if(ffi->host.handle != NULL) {
        free(ffi->host.handle);
        ffi->host.handle = NULL;
    }

    if( ffi->exe.type != NULL ) {
        int count = ffi->exe.count;
        for(int i = 0; i < count; i++) {
            ffi_type_recfree(ffi->exe.type[i]);
            ffi->exe.type[i] = NULL;
        }
        free(ffi->exe.type);
        ffi->exe.type = NULL;
    }

    if(ffi->exe.name != NULL) {
        free(ffi->exe.name);
        ffi->exe.name = NULL;
    }

}

void ffi_fprint(FILE* f, ffi_t* ffi) {
    fprintf(f, "FFI\n");
    fprintf(f, " Host\n");
    for(int i = 0; i < ffi->host.count; i++) {
        fprintf(f, "\t%.*s: ",
            sstr_len(&ffi->host.name[i]),
            sstr_ptr(&ffi->host.name[i]));
        ffi_type_fprint(f, ffi->host.type[i]);
        fprintf(f, "\n");
    }
    fprintf(f, " Executable\n");
    for(int i = 0; i < ffi->exe.count; i++) {
        fprintf(f, "\t%.*s: ",
            sstr_len(&ffi->exe.name[i]),
            sstr_ptr(&ffi->exe.name[i]));
        ffi_type_fprint(f, ffi->exe.type[i]);
        fprintf(f, "\n");
    }
}
