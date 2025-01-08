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

#include "sh_program.h"

#include "xu_lib.h"

time_t get_creation_time(char *path) {
    struct stat attr;
    stat(path, &attr);
    return attr.st_mtim.tv_sec;
}

void xu_ffi_print(ffi_hndl_meta_t md, int argcount, val_t* args) {
    char buf[512] = {0};
    for(int i = 0; i < argcount; i++) {
        if( i > 0 )
            cstr_append_fmt(buf, 512, " ");
        vm_sprint_val(buf, 512, md.vm, args[i]);
    }
    sh_log_info("> %s", buf);
}

bool xu_setup_default_env(ffi_t* ffi) {
    bool res = ffi_init(ffi);
    if( res == false ) {
        sh_log_error("error: failed to init FFI.\n");
        return false;
    }

    res = ffi_native_exports_define(&ffi->supplied,
        sstr("print"), 
        (ffi_handle_t) {
            .local = 0,
            .tag = FFI_HNDL_HOST_ACTION,
            .u.host_action = xu_ffi_print,
        },
        ift_func_1(ift_void(),
            ift_list(ift_char())));

    if( res == false ) {
        sh_log_error("error: failed to register FFI function: print\n");
    }

    return res;
}

bool file_exists(char *path) {

    FILE *file;
    
    if ((file = fopen(path, "r")))
    {
        fclose(file);
        return true;
    }

    return false;
}


bool xu_quick_run(char* filepath, xu_quickopts_t opts) {

    time_t last_creation_time = 0x0L;
    bool compile_ok = true;
    ffi_t ffi = { 0 };

    if( file_exists(filepath) == false ) {
        sh_log_error("file not found: %s", filepath);
        return false;
    }

    if( xu_setup_default_env(&ffi) == false ) {
        return false;
    }

    do {

        time_t creation_time = get_creation_time(filepath);

        if( creation_time <= last_creation_time ) {
            usleep(100);
            continue;
        }

        last_creation_time = creation_time;
        program_t program = program_read_and_compile(filepath, opts.show_ast);
        compile_ok = program.inst.size > 0;
        sh_log_info("%s [%s]\n", filepath, compile_ok ? "OK" : "FAILED");
        
        if( compile_ok ) {

            if( opts.disassemble ) {
                program_disassemble(&program);
            }

            vm_env_t env = { 0 };
            vm_env_init(&env);

            vm_env_setup(&env, &program, &ffi);

            entry_point_t ep = program_get_entry_point(&program, "main", NULL);

            vm_t vm = { 0 };
            vm_create(&vm, 256);

            if( vm_env_is_ready(&env) ) {
                char buf[2048] = {0};
                // execute script
                val_t result = vm_execute(&vm, &env, &ep, &program);
                vm_sprint_val(buf, 2048, &vm, result);
                sh_log_info(" => %s", buf);
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

xu_class_t xu_class_create(xu_classlist_t* classes, char* source_path) {

    if( file_exists(source_path) == false ) {
        sh_log_error("xu_class_create: file not found\n\t%s", source_path);
        return mk_invalid_class();
    }

    if(classes->count >= XU_COUNT) {
        sh_log_error("xu_class_create: maximum number of class objects reached (%d).",
            XU_COUNT);
        return mk_invalid_class();
    }

    int ref = classes->count;
    ffi_t* ffi = &classes->interfaces[ref].ffi;
    if(ffi_init(ffi) == false) {
        sh_log_error("xu_class_create: failed to initialize FFI.");
        return mk_invalid_class();
    }

    classes->count ++;

    classes->sources[ref] = (xu_source_t) {
        .creation_time = get_creation_time(source_path),
        .source_path = source_path
    };

    classes->runtimes[ref] = (xu_runtime_t) { 0 }; 

    classes->stages[ref] = XU_CLASS_DEFINED;

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

bool xu_class_is_defined(xu_class_t class) {
    if(xu_class_is_valid(class) == false)
        return false;
    return class.classlist->stages[class.classref] >= XU_CLASS_DEFINED;
}

bool xu_class_is_compiled(xu_class_t class) {
    if(xu_class_is_valid(class) == false)
        return false;
    return class.classlist->stages[class.classref] == XU_CLASS_COMPILED;
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
        sh_log_error("xu_class_extract: the class has not been compiled.\n\t%s",
            list->sources[class.classref].source_path);
        return mk_invalid_caller();
    }

    program_t* program = &list->runtimes[class.classref].program;
    entry_point_t ep = program_get_entry_point(program, name, &type);

    if( ep.address < 0 || ep.argcount < 0 ) {
        sh_log_error("xu_class_extract: entrypoint '%s' was not found", name);
        return mk_invalid_caller();
    }

    return (xu_caller_t) {
        .class = class,
        .entrypoint = ep
    };
}

bool xu_class_inject(xu_class_t class, char* name, ift_t type, ffi_handle_t handle) {

    // TODO: Verify that the type is function / action etc.

    if(xu_class_is_valid(class) == false) {
        sh_log_error("xu_class_inject: received invalid class data when extracting '%s'", name);
        return false;
    }

    xu_classlist_t* list = class.classlist;
    if(xu_class_is_compiled(class) == false) {
        sh_log_error("xu_class_inject: the class has not been compiled.\n\t%s",
            list->sources[class.classref].source_path);
        return false;
    }

    ffi_t* ffi = &list->interfaces[class.classref].ffi;
    if(ffi_native_exports_define(&ffi->supplied, sstr(name), handle, type) == false) {
        sh_log_error("xu_class_inject: failed to add native handler '%s'", name);
        return false;
    }

    return true;
}

bool xu_class_compile(xu_class_t class) {
    
    if(xu_class_is_valid(class) == false) {
        sh_log_error("xu_class_compile: received invalid class data");
        return false;
    }

    if(xu_class_is_defined(class) == false) {
        sh_log_error("xu_class_compile: class not defined");
        return false;
    }

    xu_classlist_t* list = class.classlist;
    program_t* program = &list->runtimes[class.classref].program;
    if( program->inst.size > 0 ) { // destroy the old program
        program_destroy(program);
        list->runtimes[class.classref].program = (program_t) { 0 };
    }

    (*program) = program_read_and_compile(list->sources->source_path, false);

    if( program->inst.size == 0 ) {
        list->stages[class.classref] = XU_CLASS_DEFINED;
        return false;
    }

    list->stages[class.classref] = XU_CLASS_COMPILED;

    ffi_t* ffi = &list->interfaces[class.classref].ffi;
    vm_env_t* env = &list->runtimes[class.classref].env;

    if(vm_env_setup(env, program, ffi) == false) {
        vm_env_destroy(env);
        sh_log_error("xu_class_compile: env setup failed");
        return false;
    }

    list->stages[class.classref] = XU_CLASS_READY;
    return true;
}

bool xu_compile(xu_classlist_t* classes) {
    int failed_count = 0;
    for(int i = 0; i < classes->count; i++) {
        xu_class_t class = (xu_class_t) {
            .classlist = classes,
            .classref = i
        };
        if(xu_class_compile(class) == false)
            failed_count ++;
    }
    return failed_count == 0;
}

void xu_cleanup(xu_classlist_t* classes) {
    for(int i = 0; i < classes->count; i++) {
        // destroy
    }
}
