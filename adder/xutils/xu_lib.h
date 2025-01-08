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
#include <assert.h>
#include <stdarg.h>
#include <sh_ift.h>

#include <dlfcn.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define XU_COUNT 16

typedef struct xu_source_t {
    char*       source_path;
    time_t      creation_time;
} xu_source_t;

typedef struct xu_interface_t {
    ffi_t       ffi;
} xu_interface_t;

typedef struct xu_runtime_t {
    program_t   program;
    vm_env_t    env;
} xu_runtime_t;

typedef enum xu_stage_t {
    UNINITIALIZED,
    DEFINED,
    COMPILED,
    READY
} xu_stage_t;

typedef struct xu_classlist_t xu_classlist_t;

typedef struct xu_class_t {
    int             classref;
    xu_classlist_t* classlist;
} xu_class_t;

typedef struct xu_classlist_t {
    int             count;
    xu_stage_t      stages[XU_COUNT];
    xu_source_t     sources[XU_COUNT];
    xu_interface_t  interfaces[XU_COUNT];
    xu_runtime_t    runtimes[XU_COUNT];
} xu_classlist_t;

typedef struct xu_caller_t {
    int             classref;
    entry_point_t   entrypoint;
    xu_classlist_t* classes;
} xu_caller_t;

/*
[xu_invokable_t]

    bool   invokeb(vm, invokable);
    int    invokei(vm, invokable);
    float  invokef(vm, invokable);
    sstr_t invokes(vm, invokable);
    bool   invokebii(vm, invokable, int a, int b);

*/

ffi_handle_t xu_ffi_action(ffi_actcall_t action, void* user);
ffi_handle_t xu_ffi_function(ffi_funcall_t function, void* user);

xu_class_t xu_class_create(xu_classlist_t* classes, char* source_path);

bool xu_class_is_defined(xu_class_t class);
bool xu_class_is_compiled(xu_class_t class);

void xu_class_extract(xu_class_t class, char* name, ift_t type);
void xu_class_inject(xu_class_t class, char* name, ift_t type, ffi_handle_t handle);

bool xu_class_compile(xu_class_t class);

xu_caller_t xu_class_find(xu_class_t class, char* name);

void xu_cleanup(xu_classlist_t* classes);

typedef struct xu_quickopts_t {
    bool        disassemble;
    bool        show_ast;
    bool        keep_alive;
} xu_quickopts_t;

bool xu_quick_run(char* filepath, xu_quickopts_t opts);

#endif // XUTILS_H_
