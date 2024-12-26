#include "vm_env.h"
#include "sh_msg_buffer.h"
#include "sh_program.h"
#include <stdlib.h>
#include <string.h>

void vm_env_init(vm_env_t* env) {
    *env = (vm_env_t) { 0 };
    sh_msg_buffer_init(&env->msgbuf, "env_t");
}

void vm_env_destroy(vm_env_t* env) {
    sh_msg_buffer_clear(&env->msgbuf);
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

bool vm_env_setup(vm_env_t* env, program_t* program, ffi_t* ffi) {

    vm_env_destroy(env);
    vm_env_init(env);

    int missing = 0;

    for(int i = 0; i < program->imports.count; i++) {

        ffi_definition_t def = program->imports.def[i];

        int index = ffi_native_exports_index_of(&ffi->supplied, def.name);

        if( index < 0 ) {
            sstr_t s = { 0 };
            sstr_append_fmt(&s, "'%.*s' could not be found in FFI.",
                sstr_len(&def.name), sstr_ptr(&def.name));
            sh_msg_buffer_append(&env->msgbuf, s);
            missing ++;
            continue;
        }

        if( ffi_type_equals(def.type, ffi->supplied.def[i].type) == false ) {
            sstr_t s = sstr("type not matching for '");
            sstr_append(&s, &def.name);
            sstr_append_str(&s, "' FFI: '");
            sstr_t tmp = ffi_type_to_sstr(ffi->supplied.def[i].type);
            sstr_append(&s, &tmp);
            sstr_append_str(&s, "' program: '");
            tmp = ffi_type_to_sstr(def.type);
            sstr_append(&s, &tmp);
            sstr_append_str(&s, "'");
            sh_msg_buffer_append(&env->msgbuf, s);
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

        sh_msg_buffer_append(&env->msgbuf,
            sstr("failed to allocate memory, out of memory?"));
        
        return false;
    }

    for(int i = 0; i < program->imports.count; i++) {
        ffi_definition_t def = program->imports.def[i];
        int supp_index = ffi_native_exports_index_of(&ffi->supplied, def.name);
        assert( supp_index >= 0 );
        mapping[i] = ffi->supplied.handle[supp_index];
        argc[i] = ffi_get_func_arg_count(ffi->supplied.def[i].type);
    }

    env->count = program->imports.count;
    env->argcounts = argc;
    env->handles = mapping;

    return true;
}




