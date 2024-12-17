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

ffi_type_t* ffi_custom_sstr(sstr_t type_name) {
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

int ffi_get_func_arg_count(ffi_type_t* type) {
    if( type->tag != FFI_TYPE_FUNC )
        return 0;
    return type->u.func.arg_count;
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

int ffi_type_snprint(char* buf, int len, ffi_type_t* ffi) {
    if( ffi == NULL ) {
        return snprintf(buf, len, "NULL");
    }
    switch(ffi->tag) {
        case FFI_TYPE_CONST: {
            return snprintf(buf, len, "%.*s",
                sstr_len(&ffi->u.cons.type_name),
                sstr_ptr(&ffi->u.cons.type_name));
        } break;
        case FFI_TYPE_LIST: {
            int w = snprintf(buf, len, "array<");
            w += ffi_type_snprint(buf + w, len-w, ffi->u.list.content_type);
            w += snprintf(buf + w, len-w, ">");
            return w;
        } break;
        case FFI_TYPE_FUNC: {
            int w = snprintf(buf, len, "(");
            int count = ffi->u.func.arg_count;
            for(int i = 0; i < count; i++) {
                if( i > 0 )
                    w += snprintf(buf + w, len-w, ", ");
                w += ffi_type_snprint(buf + w, len-w, ffi->u.func.arg_types[i]);
            }
            w += snprintf(buf + w, len, ") -> ");
            w += ffi_type_snprint(buf + w, len-w, ffi->u.func.return_type);
            return w;
        } break;
    }
    return 0;
}

sstr_t ffi_type_to_sstr(ffi_type_t* ffi) {
    char buf[SSTR_MAX_LEN] = { 0 };
    ffi_type_snprint(buf, SSTR_MAX_LEN, ffi);
    return sstr(buf);
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

ffi_type_t* ffi_type_clone(ffi_type_t* type) {
    if( type == NULL )
        return NULL;

    switch (type->tag) {
        case FFI_TYPE_CONST: {
            return ffi_custom_sstr(type->u.cons.type_name);
        } break;
        case FFI_TYPE_LIST: {
            return ffi_list(ffi_type_clone(type->u.list.content_type));
        } break;
        case FFI_TYPE_FUNC: {
            int count = type->u.func.arg_count;
            ffi_type_t* res = ffi_func(ffi_type_clone(type->u.func.return_type));
            for(int i = 0; i < count; i++) {
                ffi_func_add_arg(res,
                    ffi_type_clone(type->u.func.arg_types[i]));
            }
            return res;
        } break;
        default: {
            printf("unknown FFI type %d\n", type->tag);
            return NULL;
        }
    }
}

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

bool ffi_native_exports_define(ffi_native_exports_t* hosted, sstr_t name, ffi_handle_t handle, ffi_type_t* type) {
    // TODO: Verify that handle and type matches
    int index = ffi_native_exports_index_of(hosted, name);

    if( index >= 0 )
        return false;

    if( hosted->count >= hosted->capacity ) {
        int new_cap = hosted->count * 2;
        hosted->def = realloc(hosted->def, new_cap * sizeof(ffi_definition_t) );
        assert(hosted->handle != NULL); // todo: handle fail
        hosted->handle = realloc(hosted->handle, new_cap * sizeof(ffi_handle_t));
        assert(hosted->handle != NULL); // todo: handle fail
    }

    hosted->def[hosted->count] = (ffi_definition_t) {
        .name = name,
        .type = type
    };

    hosted->handle[hosted->count] = handle;
    hosted->count ++;
    return true;
}

ffi_type_t* ffi_native_exports_get_type(ffi_native_exports_t* host, sstr_t name) {
    int index = ffi_native_exports_index_of(host, name);
    if( index >= 0 )
        return host->def[index].type;
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

bool ffi_definition_set_add(ffi_definition_set_t* set, sstr_t name, ffi_type_t* type) {
    // TODO: Verify that handle and type matches
    int index = ffi_definition_set_index_of(set, name);
    if( index >= 0 )
        return false;

    if( set->count >= set->capacity ) {
        int new_cap = set->count * 2;
        set->def = realloc(set->def, new_cap * sizeof(ffi_definition_t));
        assert(set->def != NULL); // todo: handle fail
    }

    set->def[set->count] = (ffi_definition_t) {
        .name = name,
        .type = type
    };

    set->count ++;
    return true;
}

ffi_type_t* ffi_definition_set_get_type(ffi_definition_set_t* set, sstr_t name) {
    int index = ffi_definition_set_index_of(set, name);
    if( index >= 0 )
        return set->def[index].type;
    return NULL;
}

void ffi_native_exports_destroy(ffi_native_exports_t* hosted) {
    if(hosted->def != NULL) {
        int count = hosted->count;
        for(int i = 0; i < count; i++) {
            ffi_type_recfree(hosted->def[i].type);
            hosted->def[i].type = NULL;
        }
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
        int count = set->count;
        for(int i = 0; i < count; i++) {
            ffi_type_recfree(set->def[i].type);
            set->def[i].type = NULL;
        }
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
        fprintf(f, "\t%.*s: ",
            sstr_len(&ffi->supplied.def[i].name),
            sstr_ptr(&ffi->supplied.def[i].name));
        ffi_type_fprint(f, ffi->supplied.def[i].type);
        fprintf(f, "\n");
    }
}
