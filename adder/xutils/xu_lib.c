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

