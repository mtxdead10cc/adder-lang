#ifndef SH_FFI
#define SH_FFI

#include "sh_utils.h"
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include "sh_types.h"

ffi_type_t* ffi_void(void);
ffi_type_t* ffi_int(void);
ffi_type_t* ffi_float(void);
ffi_type_t* ffi_char(void);
ffi_type_t* ffi_bool(void);
ffi_type_t* ffi_custom(char* type_name);

ffi_type_t* ffi_list(ffi_type_t* content_type);
ffi_type_t* ffi_func(ffi_type_t* return_type);
void ffi_func_add_arg(ffi_type_t* func, ffi_type_t* arg_type);
int ffi_get_func_arg_count(ffi_type_t* type);
void ffi_type_recfree(ffi_type_t* ffi);
void ffi_type_fprint(FILE* f, ffi_type_t* ffi);
int ffi_type_snprint(char* buf, int len, ffi_type_t* ffi);
sstr_t ffi_type_to_sstr(ffi_type_t* ffi);
bool ffi_type_equals(ffi_type_t* a, ffi_type_t* b);
ffi_type_t* ffi_type_clone(ffi_type_t* type);

ffi_type_t* ffi_vfunc_nullterm(ffi_type_t* return_type, ...);
#define ffi_vfunc(R, ...) ffi_vfunc_nullterm((R), __VA_ARGS__, NULL)

bool ffi_init(ffi_t* ffi);
void ffi_destroy(ffi_t* ffi);
void ffi_fprint(FILE* f, ffi_t* ffi);

bool        ffi_definition_set_init(ffi_definition_set_t* set, int capacity);
int         ffi_definition_set_index_of(ffi_definition_set_t* set, sstr_t name);
ffi_type_t* ffi_definition_set_get_type(ffi_definition_set_t* set, sstr_t name);
bool        ffi_definition_set_add(ffi_definition_set_t* set, sstr_t name, ffi_type_t* type);
void        ffi_definition_set_destroy(ffi_definition_set_t* set);

bool        ffi_native_exports_init(ffi_native_exports_t* hosted, int capacity);
int         ffi_native_exports_index_of(ffi_native_exports_t* hostif, sstr_t name);
ffi_type_t* ffi_native_exports_get_type(ffi_native_exports_t* hostif, sstr_t name);
bool        ffi_native_exports_define(ffi_native_exports_t* hostif, sstr_t name, ffi_handle_t handle, ffi_type_t* type);
void        ffi_native_exports_destroy(ffi_native_exports_t* hosted);



#endif // SH_FFI
