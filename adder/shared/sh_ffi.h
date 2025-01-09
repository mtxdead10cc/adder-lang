#ifndef SH_FFI
#define SH_FFI

#include "sh_utils.h"
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include "sh_types.h"

bool ffi_init(ffi_t* ffi);
void ffi_destroy(ffi_t* ffi);
void ffi_print(ffi_t* ffi);

bool        ffi_definition_set_init(ffi_definition_set_t* set, int capacity);
int         ffi_definition_set_index_of(ffi_definition_set_t* set, sstr_t name);
ift_t*      ffi_definition_set_get_type(ffi_definition_set_t* set, sstr_t name);
bool        ffi_definition_set_add(ffi_definition_set_t* set, sstr_t name, ift_t type);
void        ffi_definition_set_destroy(ffi_definition_set_t* set);

bool        ffi_native_exports_init(ffi_native_exports_t* hosted, int capacity);
int         ffi_native_exports_index_of(ffi_native_exports_t* hostif, sstr_t name);
ift_t*      ffi_native_exports_get_type(ffi_native_exports_t* hostif, sstr_t name);
bool        ffi_native_exports_define(ffi_native_exports_t* hostif, sstr_t name, ffi_handle_t handle, ift_t type);
void        ffi_native_exports_destroy(ffi_native_exports_t* hosted);

#endif // SH_FFI
