#ifndef SH_FFI
#define SH_FFI

#include "sh_utils.h"
#include <stdbool.h>
#include <stdarg.h>

typedef enum ffi_tag_t {
    FFI_TYPE_CONST,
    FFI_TYPE_FUNC,
    FFI_TYPE_LIST
} ffi_tag_t;

typedef struct ffi_type_t ffi_type_t;

typedef struct ffi_constant_t {
    sstr_t type_name;
} ffi_constant_t;

typedef struct ffi_list_t {
    ffi_type_t* content_type;
} ffi_list_t;

typedef struct ffi_functor_t {
    int arg_count;
    ffi_type_t** arg_types;
    ffi_type_t* return_type;
} ffi_functor_t;

typedef struct ffi_type_t {
    ffi_tag_t tag;
    union {
        ffi_constant_t  cons;
        ffi_list_t      list;
        ffi_functor_t   func;
    } u;
} ffi_type_t;

ffi_type_t* ffi_const(sstr_t type_name);
ffi_type_t* ffi_list(ffi_type_t* content_type);
ffi_type_t* ffi_func(ffi_type_t* return_type);
void ffi_func_add_arg(ffi_type_t* func, ffi_type_t* arg_type);
void ffi_recfree(ffi_type_t* ffi);
void ffi_fprint(FILE* f, ffi_type_t* ffi);
bool ffi_equals(ffi_type_t* a, ffi_type_t* b);

ffi_type_t* ffi_vfunc_nullterm(ffi_type_t* return_type, ...);
#define ffi_vfunc(R, ...) ffi_vfunc_nullterm((R), __VA_ARGS__, NULL)

typedef struct ffi_bundle_t {
    int          capacity;
    int          count;
    sstr_t*      name;
    ffi_type_t** type;
} ffi_bundle_t;

bool ffi_bundle_init(ffi_bundle_t* bundle, int capacity);
int  ffi_bundle_index_of(ffi_bundle_t* bundle, sstr_t* name);
bool ffi_bundle_add(ffi_bundle_t* bundle, sstr_t name, ffi_type_t* type);
void ffi_bundle_destroy(ffi_bundle_t* bundle);
void ffi_bundle_fprint(FILE* f, ffi_bundle_t* bundle);

#endif // SH_FFI