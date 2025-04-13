#ifndef XUTILS_H_
#define XUTILS_H_

#include <vm.h>
#include <vm_heap.h>
#include <sh_value.h>
#include <sh_arena.h>
#include <co_ast.h>
#include <co_trace.h>
#include <co_parser.h>
#include <co_compiler.h>
#include <co_program.h>
#include <co_bty.h>
#include <sh_program.h>
#include <sh_log.h>
#include <vm_env.h>
#include <sh_ffi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <sh_ift.h>

#include <dlfcn.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define XU_COUNT 16

typedef struct xu_classlist_t xu_classlist_t;

typedef struct xu_class_t {
    int             classref;
    xu_classlist_t* classlist;
} xu_class_t;

typedef struct xu_classlist_t {
    int             count;
    char*           paths[XU_COUNT];
    int             user_ids[XU_COUNT];
    time_t          modtimes[XU_COUNT];
    program_t       programs[XU_COUNT];
    ffi_t           interfaces[XU_COUNT];
    vm_env_t        envs[XU_COUNT];
} xu_classlist_t;

typedef struct xu_caller_t {
    xu_class_t      class;
    entry_point_t   entrypoint;
} xu_caller_t;

ffi_handle_t xu_ffi_action(ffi_actcall_t action, void* user);
ffi_handle_t xu_ffi_function(ffi_funcall_t function, void* user);

xu_class_t xu_class_read_and_create(xu_classlist_t* classes, char* file_path, int class_id);
xu_class_t xu_class_create(xu_classlist_t* classes, source_code_t* code, int class_id);

bool xu_class_is_ready(xu_class_t class);
bool xu_class_is_compiled(xu_class_t class);
int  xu_class_get_id(xu_class_t class, int not_found_default);

xu_caller_t xu_class_extract(xu_class_t class, char* name, ift_t type);
bool xu_class_caller_is_valid(xu_caller_t caller);

bool xu_class_inject(xu_class_t class, char* name, ift_t type, ffi_handle_t handle);

bool xu_class_finalize(xu_class_t class);

typedef struct xu_iterator_t {
    int current;
    xu_classlist_t* classes;
} xu_iterator_t;

xu_iterator_t xu_iterator(xu_classlist_t* classes);
void xu_iterator_reset(xu_iterator_t* it);
bool xu_iterator_next(xu_iterator_t* it);
xu_class_t xu_iterator_current(xu_iterator_t* it);

typedef enum xu_result_t {
    XU_OK                   = 0x00,
    XU_NO_CHANGE            = 0x01,
    XU_ERROR_INVALID_PARAM  = 0x10,
    XU_ERROR_COMPILATION    = 0x20,
    XU_ERROR_ENV            = 0x30
} xu_result_t;

#define xu_result_is_error(rescode) ((rescode & 0xF0) > 0)

xu_result_t xu_refresh_class(xu_class_t class);

bool xu_finalize_all(xu_classlist_t* classes);
void xu_cleanup_all(xu_classlist_t* classes);

typedef struct xu_quickopts_t {
    bool        disassemble;
    bool        show_ast;
    bool        keep_alive;
    int         vm_memory;
    char*       callstr; // fname(0, false, 1.3, "hello")
} xu_quickopts_t;


bool xu_quick_run(char* filepath, xu_quickopts_t opts);

char* xu_val_to_string(vm_t* vm, val_t val);
val_t xu_string_to_val(vm_t* vm, char* val);

#endif // XUTILS_H_
