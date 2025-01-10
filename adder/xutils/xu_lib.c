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
#include <vm_value_tools.h>
#include <vm_heap.h>


#include "sh_program.h"

#include "xu_lib.h"


void xu_ffi_print(ffi_hndl_meta_t md, int argcount, val_t* args) {
    define_cstr(str, 512);
    for(int i = 0; i < argcount; i++) {
        if( i > 0 )
            cstr_append_fmt(str, " ");
        vm_sprint_val(str, md.vm, args[i]);
    }
    sh_log_info("> %s", str.ptr);
}


val_t xu_ffi_to_string(ffi_hndl_meta_t md, int argcount, val_t* args) {
    define_cstr(str, 512);

    for(int i = 0; i < argcount; i++) {
        if( i > 0 )
            cstr_append_fmt(str, " ");
        vm_sprint_val(str, md.vm, args[i]);
    }

    int len = strlen(str.ptr);
    array_t arr = heap_array_alloc(md.vm, len);
    val_t* ptr = array_get_ptr(md.vm, arr, 0);
    for(int i = 0; i < len; i++) {
        ptr[i] = val_char(str.ptr[i]);
    }
    return val_array(arr);
}

bool xu_setup_default_interface(ffi_t* ffi) {
    
    if( ffi_init(ffi) == false ) {
        sh_log_error("error: failed to init FFI.\n");
        return false;
    }

    int res = ffi_native_exports_define(&ffi->supplied,
        sstr("print"), 
        (ffi_handle_t) {
            .local = 0,
            .tag = FFI_HNDL_HOST_ACTION,
            .u.host_action = xu_ffi_print,
        },
        ift_func_1(ift_void(),
            ift_list(ift_char())));

    res += ffi_native_exports_define(&ffi->supplied,
        sstr("itos"), 
        (ffi_handle_t) {
            .local = 0,
            .tag = FFI_HNDL_HOST_FUNCTION,
            .u.host_function = xu_ffi_to_string,
        },
        ift_func_1(ift_list(ift_char()), ift_int()));

    res += ffi_native_exports_define(&ffi->supplied,
        sstr("ftos"), 
        (ffi_handle_t) {
            .local = 0,
            .tag = FFI_HNDL_HOST_FUNCTION,
            .u.host_function = xu_ffi_to_string,
        },
        ift_func_1(ift_list(ift_char()), ift_float()));

    res += ffi_native_exports_define(&ffi->supplied,
        sstr("btos"), 
        (ffi_handle_t) {
            .local = 0,
            .tag = FFI_HNDL_HOST_FUNCTION,
            .u.host_function = xu_ffi_to_string,
        },
        ift_func_1(ift_list(ift_char()), ift_bool()));

    if( res < 4 )
        sh_log_error("error: failed to register FFI function");
    
    // ffi_print(ffi);

    return res;
}


bool xu_quick_run(char* filepath, xu_quickopts_t opts) {

    time_t last_creation_time = 0x0L;
    bool compile_ok = true;
    ffi_t ffi = { 0 };

    if( program_file_exists(filepath) == false ) {
        sh_log_error("file not found: %s", filepath);
        return false;
    }

    if( xu_setup_default_interface(&ffi) == false ) {
        return false;
    }

    do {

        time_t creation_time = program_file_get_modtime(filepath);

        if( creation_time <= last_creation_time ) {
            usleep(100);
            continue;
        }

        last_creation_time = creation_time;
        source_code_t code = program_source_read_from_file(filepath);
        program_t program = program_compile(&code, opts.show_ast);
        compile_ok = program_is_valid(&program);
        sh_log_info("%s [%s]\n", filepath, compile_ok ? "OK" : "FAILED");
        
        if( compile_ok ) {

            if( opts.disassemble ) {
                program_disassemble(&program);
            }

            vm_env_t env = { 0 };
            vm_env_setup(&env, &program, &ffi);

            entry_point_t ep = program_entry_point_find(&program, "main", NULL);

            vm_t vm = { 0 };
            vm_create(&vm, 256);

            if( vm_env_is_ready(&env) ) {
                define_cstr(str, 2048);
                // execute script
                val_t result = vm_execute(&vm, &env, &ep, &program);
                vm_sprint_val(str, &vm, result);
                sh_log_info(" => %s", str.ptr);
            }

            vm_env_destroy(&env);
            program_destroy(&program);
            vm_destroy(&vm);
        }

    } while ( opts.keep_alive );

    ffi_destroy(&ffi);

    return compile_ok;
}

#define UNUSED(X) (void)(X)

ffi_handle_t xu_ffi_action(ffi_actcall_t action, void* user) {
    return (ffi_handle_t) {
        .local = user,
        .tag = FFI_HNDL_HOST_ACTION,
        .u.host_action = action
    };
}

ffi_handle_t xu_ffi_function(ffi_funcall_t function, void* user) {
    return (ffi_handle_t) {
        .local = user,
        .tag = FFI_HNDL_HOST_FUNCTION,
        .u.host_function = function
    };
}

xu_class_t mk_invalid_class(void) {
    return (xu_class_t) {
        .classlist = NULL,
        .classref = -1
    };
}


xu_class_t xu_class_create(xu_classlist_t* classes, source_code_t* code) {

    if( program_source_is_valid(code) == false ) {
        sh_log_error("xu_class_create: received invalid source code");
        return mk_invalid_class();
    }

    if(classes->count >= XU_COUNT) {
        sh_log_error("xu_class_create: maximum number of class objects reached (%d).",
            XU_COUNT);
        return mk_invalid_class();
    }

    int ref = classes->count;
    ffi_t* ffi = &classes->interfaces[ref];
    if( xu_setup_default_interface(ffi) == false ) {
        sh_log_error("xu_class_create: failed to initialize FFI.");
        return mk_invalid_class();
    }

    if( program_is_valid(&classes->programs[ref]) ) { // destroy the old program
        program_destroy(&classes->programs[ref]);
        classes->programs[ref] = (program_t) { 0 };
    }

    classes->programs[ref] = program_compile(code, false);
    if( program_is_valid(&classes->programs[ref]) == false )
        return mk_invalid_class();

    //program_disassemble(&classes->programs[ref]);

    if( program_file_exists(code->file_path) ) {
        // check if source from a real file
        classes->paths[ref] = code->file_path;
    } else {
        // or memory buffer
        classes->paths[ref] = NULL;
    }

    classes->modtimes[ref] = code->modtime;
    classes->envs[ref] = (vm_env_t) {0};
    classes->count ++;

    return (xu_class_t) {
        .classlist = classes,
        .classref = ref
    };
}

bool xu_class_is_valid(xu_class_t class) {
    if(class.classlist == NULL)
        return false;
    if(class.classref < 0 || class.classref >= XU_COUNT)
        return false;
    return true;
}

bool xu_class_is_compiled(xu_class_t class) {
    if(xu_class_is_valid(class) == false)
        return false;
    program_t* program = &class.classlist->programs[class.classref];
    return program_is_valid(program);
}

xu_caller_t mk_invalid_caller(void) {
    return (xu_caller_t) {
        .class = mk_invalid_class(),
        .entrypoint = (entry_point_t) {
            .argvals = { 0 },
            .argcount = -1,
            .address = -1
        }
    };
}

xu_caller_t xu_class_extract(xu_class_t class, char* name, ift_t type) {

    // TODO: Verify that the type is function / action etc.

    if(xu_class_is_valid(class) == false) {
        sh_log_error("xu_class_extract: received invalid class data when extracting '%s'", name);
        return mk_invalid_caller();
    }

    xu_classlist_t* list = class.classlist;
    if(xu_class_is_compiled(class) == false) {
        char* path = list->paths[class.classref];
        sh_log_error("xu_class_extract: the class has not been compiled: %s",
            (path != NULL) ? path : "(from memory buffer)");
        return mk_invalid_caller();
    }

    program_t* program = &list->programs[class.classref];
    entry_point_t ep = program_entry_point_find(program, name, &type);

    if( ep.address < 0 || ep.argcount < 0 ) {
        sh_log_error("xu_class_extract: entrypoint '%s' was not found", name);
        return mk_invalid_caller();
    }

    return (xu_caller_t) {
        .class = class,
        .entrypoint = ep
    };
}

bool xu_class_caller_is_valid(xu_caller_t caller) {
    return xu_class_is_compiled(caller.class)
        && program_entry_point_is_valid(caller.entrypoint);
}

bool xu_class_inject(xu_class_t class, char* name, ift_t type, ffi_handle_t handle) {

    // TODO: Verify that the type is function / action etc.

    if(xu_class_is_valid(class) == false) {
        sh_log_error("xu_class_inject: received invalid class data when extracting '%s'", name);
        return false;
    }

    xu_classlist_t* list = class.classlist;
    if(xu_class_is_compiled(class) == false) {
        char* path = list->paths[class.classref];
        sh_log_error("xu_class_inject: the class has not been compiled: %s",
            (path != NULL) ? path : "(from memory buffer)");
        return false;
    }

    ffi_t* ffi = &list->interfaces[class.classref];
    if(ffi_native_exports_define(&ffi->supplied, sstr(name), handle, type) == false) {
        sh_log_error("xu_class_inject: failed to add native handler '%s'", name);
        return false;
    }

    return true;
}

bool xu_class_finalize(xu_class_t class) {
    
    if(xu_class_is_valid(class) == false) {
        sh_log_error("xu_class_compile: received invalid class data");
        return false;
    }

    if(xu_class_is_compiled(class) == false) {
        sh_log_error("xu_class_compile: class not compiled");
        return false;
    }

    xu_classlist_t* list = class.classlist;

    ffi_t* ffi = &list->interfaces[class.classref];
    vm_env_t* env = &list->envs[class.classref];
    program_t* program = &list->programs[class.classref];

    if(vm_env_setup(env, program, ffi) == false) {
        vm_env_destroy(env);
        sh_log_error("xu_class_compile: env setup failed");
        return false;
    }

    return true;
}

bool xu_finalize(xu_classlist_t* classes) {
    int failed_count = 0;
    for(int i = 0; i < classes->count; i++) {
        xu_class_t class = (xu_class_t) {
            .classlist = classes,
            .classref = i
        };
        if(xu_class_finalize(class) == false)
            failed_count ++;
    }
    return failed_count == 0;
}

void xu_cleanup(xu_classlist_t* classes) {
    for(int i = 0; i < classes->count; i++) {
        ffi_destroy(&classes->interfaces[i]);
        program_destroy(&classes->programs[i]);
        vm_env_destroy(&classes->envs[i]);
        classes->modtimes[i] = 0UL;
        classes->paths[i] = NULL;
        classes->programs[i] = (program_t) {0};
        classes->interfaces[i] = (ffi_t) {0};
        classes->envs[i] = (vm_env_t) {0};
    }
    classes->count = 0;
}

val_t xu_string_to_val(vm_t* vm, char* val) {
    int len = strnlen(val, 2048);
    assert(len < 2048);
    array_t array = heap_array_alloc(vm, len);
    val_t* ptr = array_get_ptr(vm, array, 0);
    for(int i = 0; i < len; i++) {
        ptr[i] = val_char(val[i]);
    }
    return val_array(array);
}

char* xu_val_to_string(vm_t* vm, val_t val) {
    // note: this will get messy if 
    // called from multiple threads
    static char buf[2048];
    buf[0] = '\n'; // reset previous
    cstr_t str = {
        .maxlen = 2048,
        .ptr = &buf
    };
    vm_sprint_val(str, vm, val);
    return buf;
}

int xu_calli(vm_t* vm, xu_caller_t* c) {
    assert(c->entrypoint.argcount == 0);
    xu_classlist_t* list = c->class.classlist;
    int ref = c->class.classref;
    vm_env_t* env = &list->envs[ref];
    program_t* program = &list->programs[ref];
    val_t result = vm_execute(vm, env, &c->entrypoint, program);
    return val_into_number(result);
}

bool xu_callib(vm_t* vm, xu_caller_t* c, int arg) {
    assert(c->entrypoint.argcount == 1);
    xu_classlist_t* classes = c->class.classlist;
    int ref = c->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];
    program_entry_point_set_arg(&c->entrypoint, 0, val_number(arg));
    val_t result = vm_execute(vm, env, &c->entrypoint, program);
    return val_into_bool(result);
}