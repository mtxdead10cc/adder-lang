#ifndef SH_FFI
#define SH_FFI

#include "sh_utils.h"
#include "sh_types.h"
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

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

ffi_type_t* ffi_void(void);
ffi_type_t* ffi_int(void);
ffi_type_t* ffi_float(void);
ffi_type_t* ffi_char(void);
ffi_type_t* ffi_bool(void);
ffi_type_t* ffi_custom(char* type_name);

ffi_type_t* ffi_list(ffi_type_t* content_type);
ffi_type_t* ffi_func(ffi_type_t* return_type);
void ffi_func_add_arg(ffi_type_t* func, ffi_type_t* arg_type);
void ffi_recfree(ffi_type_t* ffi);
void ffi_fprint(FILE* f, ffi_type_t* ffi);
bool ffi_equals(ffi_type_t* a, ffi_type_t* b);

ffi_type_t* ffi_vfunc_nullterm(ffi_type_t* return_type, ...);
#define ffi_vfunc(R, ...) ffi_vfunc_nullterm((R), __VA_ARGS__, NULL)

typedef struct vm_t vm_t;
typedef struct ffi_hndl_meta_t {
    void* local;
    vm_t* vm;
} ffi_hndl_meta_t;

typedef void  (*ffi_actcall_t)(ffi_hndl_meta_t, int, val_t*);
typedef val_t (*ffi_funcall_t)(ffi_hndl_meta_t, int, val_t*);

typedef enum ffi_handle_tag_t {
    FFI_HNDL_HOST_ACTION,
    FFI_HNDL_HOST_FUNCTION,
    FFI_PROGRAM_REQUIREMENT
} ffi_handle_tag_t;

typedef struct ffi_handle_t {
    ffi_handle_tag_t tag;
    void*            local;
    union {
        ffi_actcall_t host_action;   // defined by user
        ffi_funcall_t host_function; // defined by user
    } u;
} ffi_handle_t;

typedef struct ffi_host_t {
    int             capacity;
    int             count;
    sstr_t*         name;
    ffi_type_t**    type;
    ffi_handle_t*   handle;
    void*           shared;
} ffi_host_t;

bool ffi_host_init(ffi_host_t* host, int capacity);
int  ffi_host_index_of(ffi_host_t* host, sstr_t name);
ffi_type_t* ffi_host_get_type(ffi_host_t* host, sstr_t name);
bool ffi_host_add(ffi_host_t* host, sstr_t name, ffi_handle_t handle, ffi_type_t* type);
void ffi_host_destroy(ffi_host_t* host);
void ffi_host_fprint(FILE* f, ffi_host_t* host);
int  ffi_host_get_count(ffi_host_t* host, ffi_handle_tag_t tag);
int  ffi_host_find_entrypoint(ffi_host_t* host, sstr_t name);

#endif // SH_FFI
