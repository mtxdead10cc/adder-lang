#include "vm_env.h"
#include "sh_log.h"
#include "sh_program.h"
#include "sh_ift.h"
#include <stdlib.h>
#include <string.h>

void vm_env_init(vm_env_t* env) {
    *env = (vm_env_t) { 0 };
}

void vm_env_destroy(vm_env_t* env) {
    if( env->argcounts != NULL ) {
        free(env->argcounts);
        env->argcounts = NULL;
    }
    if( env->handles != NULL ) {
        free(env->handles);
        env->handles = NULL;
    }
    env->count = 0;
}

void vm_env_reset(vm_env_t* env) {
    vm_env_destroy(env);
    vm_env_init(env);
}

bool vm_env_is_ready(vm_env_t* env) {
    return env->isready;
}

bool vm_env_setup(vm_env_t* env, program_t* program, ffi_t* ffi) {

    vm_env_reset(env);

    if( ffi == NULL ) {
        if( program->imports.count == 0 ) {
            env->count = 0;
            env->argcounts = NULL;
            env->handles = NULL;
            env->isready = true;
            return true;
        } else {
            sh_log_error("program expects %d imports "
                "but no FFI was provided (ffi was NULL).",
                program->imports.count);
            return false;
        }
    }

    int missing = 0;

    for(int i = 0; i < program->imports.count; i++) {

        ffi_definition_t def = program->imports.def[i];

        int index = ffi_native_exports_index_of(&ffi->supplied, def.name);

        if( index < 0 ) {
            sh_log_error("'%.*s' could not be found in FFI.",
                sstr_len(&def.name), sstr_ptr(&def.name));
            missing ++;
            continue;
        }

        if( ift_type_equals(&def.type, &ffi->supplied.def[index].type) == false ) {
            sstr_t s = sstr("type not matching for '");
            sstr_append(&s, &def.name);
            sstr_append_str(&s, "' FFI: '");
            sstr_t tmp = ift_type_to_sstr(ffi->supplied.def[index].type);
            sstr_append(&s, &tmp);
            sstr_append_str(&s, "' program: '");
            tmp = ift_type_to_sstr(def.type);
            sstr_append(&s, &tmp);
            sstr_append_str(&s, "'");
            sh_log_error("%.*s", sstr_len(&s), sstr_ptr(&s));
            missing ++;
        }
    }

    if( missing > 0 ) {
        return false;
    }

    ffi_handle_t* mapping = (ffi_handle_t*) malloc( sizeof(ffi_handle_t) * program->imports.count );
    int* argc = (int*) malloc( sizeof(int) * program->imports.count );

    if( mapping == NULL || argc == NULL ) {

        if( mapping != NULL )
            free(mapping);

        if( argc != NULL )
            free(argc);

        sh_log_error("failed to allocate memory, out of memory?");
        
        return false;
    }

    for(int i = 0; i < program->imports.count; i++) {
        ffi_definition_t def = program->imports.def[i];
        int supp_index = ffi_native_exports_index_of(&ffi->supplied, def.name);
        assert( supp_index >= 0 );
        mapping[i] = ffi->supplied.handle[supp_index];
        argc[i] = ift_func_arg_count(ffi->supplied.def[supp_index].type);
    }

    env->count = program->imports.count;
    env->argcounts = argc;
    env->handles = mapping;
    env->isready = true;

    return true;
}




