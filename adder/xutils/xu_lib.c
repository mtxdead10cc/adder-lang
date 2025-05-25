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
    sh_log("> %s", str.ptr);
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

val_t xu_ffi_add_strings(ffi_hndl_meta_t md, int argcount, val_t* args) {

    assert(argcount == 2);
    (void)(argcount);

    array_t a = val_into_array(args[0]);
    array_t b = val_into_array(args[1]);

    array_t new_array = heap_array_alloc(md.vm, a.length + b.length);
    val_t* new_ptr = array_get_ptr(md.vm, new_array, 0);

    val_t* a_ptr = array_get_ptr(md.vm, a, 0);
    for(int i = 0; i < a.length; i++) {
        new_ptr[i] = a_ptr[i];
    }

    val_t* b_ptr = array_get_ptr(md.vm, b, 0);
    for(int i = 0; i < b.length; i++) {
        new_ptr[a.length + i] = b_ptr[i];
    }

    return val_array(new_array);
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

    res += ffi_native_exports_define(&ffi->supplied,
        sstr("stradd"), 
        (ffi_handle_t) {
            .local = 0,
            .tag = FFI_HNDL_HOST_FUNCTION,
            .u.host_function = xu_ffi_add_strings,
        },
        ift_func_2(ift_list(ift_char()),
            ift_list(ift_char()),
            ift_list(ift_char())));

    if( res < 4 )
        sh_log_error("error: failed to register FFI function");
    
    // ffi_print(ffi);

    return res;
}

#define XU_PARSE_MAX_SIZE 512
#define XU_PARSE_MAX_ARGS 8

int xu_callstr_segment_length(char* str, int maxlen) {
    bool qouted = false;
    for(int i = 0; i < maxlen; i++) {
        if( str[i] == '"' )
            qouted = !qouted;
        if( qouted == false && (str[i] == ',' || str[i] == ')') )
            return i;
    }
    return -1;
}

int xu_callstr_length(char* callstr) {
    int len = strnlen(callstr, XU_PARSE_MAX_SIZE + 1);
    if( len >= XU_PARSE_MAX_SIZE ) {
        return -1;
    }
    return len;
}

int xu_callstr_name_length(char* callstr) {
    int len = xu_callstr_length(callstr);
    len = str_index_of(callstr, len, '(');
    len = str_rstrip_whitespace(callstr, len);
    if( len < 0 )
        len = 0;
    return len;
}

ift_t xu_ift_from_callstring(char* callstr) {

    if( callstr == NULL )
        return ift_unknown();

    int len = xu_callstr_length(callstr);
    int start = str_index_of(callstr, len, '(') + 1;
    if( start >= len || start < 0 ) {
        sh_log_error("error: malformed callstring");
        return ift_unknown();
    }

    ift_t ftype = ift_func(ift_unknown());

    while(true) {

        int seglen = xu_callstr_segment_length(callstr + start, len - start);
        
        if( seglen < 0 )
            break;
        
        if( str_is_string(callstr + start, seglen) ) {
            ftype = ift_func_add_arg(ftype, ift_list(ift_char()));
        } else if(str_is_bool(callstr + start, seglen)) {
            ftype = ift_func_add_arg(ftype, ift_bool());
        } else if(str_is_float(callstr + start, seglen)) {
            ftype = ift_func_add_arg(ftype, ift_float());
        } else if(str_is_int(callstr + start, seglen)) {
            ftype = ift_func_add_arg(ftype, ift_int());
        } else {
            break;
        }
        
        start += seglen + 1;
    }

    return ftype;
}

val_t xu_alloc_user_string(vm_t* vm, int len, char* str) {

    int start = str_index_of(str, len, '"') + 1;
    int end = str_index_of(str + start, len, '"');

    int arraylen = end - start;

    if( start < 0 || end < 0 || arraylen < 0 )
        arraylen = 0;

    array_t array = heap_array_alloc(vm, arraylen);
    val_t* ptr = array_get_ptr(vm, array, 0);
    for(int i = 0; i < arraylen; i++) {
        ptr[i] = (val_t) {
            .type = VAL_CHAR,
            .u.character = str[i + start]
        };
    }

    return val_array(array);
}

int xu_args_from_callstring(vm_t* vm, char* callstr, int maxlen, val_t* args) {

    if( callstr == NULL )
        return 0;

    int len = xu_callstr_length(callstr);
    int start = str_index_of(callstr, len, '(') + 1;
    if( start >= len ) {
        return 0;
    }

    int argc = 0;

    while(argc < maxlen) {

        int seglen = xu_callstr_segment_length(callstr + start, len - start);
        
        if( seglen < 0 )
            break;
        
        if( str_is_string(callstr + start, seglen) ) {
            args[argc++] = xu_alloc_user_string(vm, seglen, callstr + start);
        } else if(str_is_bool(callstr + start, seglen)) {
            args[argc++] = val_bool(strncmp(callstr + start, "true", 4) == 0);
        } else if(str_is_float(callstr + start, seglen)) {
            float v = 0.0f;
            sscanf(callstr + start, "%f", &v);
            args[argc++] = val_number(v);
        } else if (str_is_int(callstr + start, seglen)) {
            int v = 0;
            sscanf(callstr + start, "%d", &v);
            args[argc++] = val_number(v);
        } else {
            break;
        }
        
        start += seglen + 1;
    }

    return argc;
}

int xu_call_string_to_entry_point(program_t* program, char* callstr, entry_point_t* result) {

    *result = program_entry_point_invalid();

    if( callstr == NULL )
        return PEP_NAME_NOT_FOUND;

    int nlen = xu_callstr_name_length(callstr);
    if( nlen <= 0 )
        return PEP_NAME_NOT_FOUND;

    char name[nlen + 1];
    for(int i = 0; i < nlen; i++)
        name[i] = callstr[i];
    name[nlen] = '\0';

    ift_t ftype = xu_ift_from_callstring(callstr);
    return program_entry_point_find(program, name, ftype, result);
}

bool xu_quick_run(char* filepath, xu_quickopts_t opts) {

    time_t last_creation_time = 0x0L;
    bool all_checks_passed = true;
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
        program_source_free(&code);
        all_checks_passed = program_is_valid(&program);
        sh_log("%s [%s]\n", filepath, all_checks_passed ? "OK" : "FAILED");

        entry_point_t entrypoint = { 0 };

        char* callstr = opts.callstr == NULL ? "main()" : opts.callstr;
        int find_result = xu_call_string_to_entry_point(&program, callstr, &entrypoint);
        
        switch(find_result) {
            case PEP_INVALID_ENTRY_POINT_ARG_COUNT: {
                sh_log_error(
                    "the argument count of call string "
                    "%s does not match the entry point",
                    callstr);
            } break;
            case PEP_INVALID_PROGRAM_ENTRY_POINT: {
                sh_log_error("invalid program entry point");
            } break;
            case PEP_NAME_NOT_FOUND: {
                int nlen = xu_callstr_name_length(callstr);
                sh_log_error("no exported function called '%.*s' was found",
                    nlen, callstr);
            } break;
            case PEP_TYPE_NOT_MATCHING: {
                ift_t ftype = xu_ift_from_callstring(callstr);
                sstr_t calltype_str = ift_type_to_sstr(ftype);
                sstr_t progtype_str = ift_type_to_sstr(entrypoint.type);
                calltype_str = sstr_substr(&calltype_str, 0, sstr_index_of(&calltype_str, ')') + 1);
                int nlen = xu_callstr_name_length(callstr);
                sh_log_error(
                    "the function '%.*s' was found, but "
                    "its type\n|  %s\ndoes not match the "
                    "call string\n|  %s // %s",
                    nlen, callstr,
                    progtype_str.str,
                    callstr,
                    calltype_str.str);
            } break;
            case PEP_INVALID_PROGRAM: {
                sh_log_error(
                    "invalid source code (the string was "
                    "NULL or no exports/main defined)");
            } break;
            default: break;
        }

        if( program_entry_point_is_valid(entrypoint) == false ) {
            all_checks_passed = false;
        }
        
        if( all_checks_passed ) {

            if( opts.disassemble ) {
                program_disassemble(&program);
            }

            vm_env_t env = { 0 };
            vm_env_setup(&env, &program, &ffi);

            vm_t vm = { 0 };
            vm_create(&vm, opts.vm_memory);

            if( vm_env_is_ready(&env) ) {
                xu_args_from_callstring(&vm, opts.callstr, entrypoint.argcount, entrypoint.argvals);
                // execute script
                val_t result = vm_execute(&vm, &env, &entrypoint, &program);
                ift_t return_type = ift_func_get_return_type(entrypoint.type);
                if( ift_is_void(return_type) == false ) {
                    define_cstr(str, 2048);
                    vm_sprint_val(str, &vm, result);
                    sh_log(" => %s", str.ptr);
                }
            }

            vm_env_destroy(&env);
            vm_destroy(&vm);
        }

        program_destroy(&program);

    } while ( opts.keep_alive );

    ffi_destroy(&ffi);

    return all_checks_passed;
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

xu_class_t xu_class_read_and_create(xu_classlist_t* classes, char* file_path, int class_id) {
    source_code_t code = program_source_read_from_file(file_path);
    xu_class_t class = xu_class_create(classes, &code, class_id);
    program_source_free(&code);
    return class;
}


xu_class_t xu_class_create(xu_classlist_t* classes, source_code_t* code, int class_id) {

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
        int len = strnlen(code->file_path, 2048-1);
        memcpy(classes->paths[ref], code->file_path, len);
        classes->paths[ref][len] = '\0';
    } else {
        // or memory buffer
        memset(&classes->paths[ref], 0, 2048);
    }

    classes->modtimes[ref] = code->modtime;
    classes->user_ids[ref] = class_id;
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

int xu_class_get_id(xu_class_t class, int not_found_default) {
    if( xu_class_is_valid(class) )
        return class.classlist->user_ids[class.classref];
    return not_found_default;
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
            .argvals = {{ 0 }},
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
            (path[0] != 0) ? path : "(from memory buffer)");
        return mk_invalid_caller();
    }

    program_t* program = &list->programs[class.classref];

    entry_point_t ep = {0};
    
    if( program_entry_point_find(program, name, type, &ep) != PEP_OK ) {
        sh_log_error("xu_class_extract: entrypoint '%s' could not be extracted", name);
        return mk_invalid_caller();
    }

    if( ep.address < 0 || ep.argcount < 0 ) {
        sh_log_error("xu_class_extract: entrypoint '%s' was invalid", name);
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
        sh_log_error("xu_class_inject: received invalid class data when injecting '%s'", name);
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

xu_iterator_t xu_iterator(xu_classlist_t* classes) {
    return (xu_iterator_t) {
        .classes = classes,
        .current = -1
    };
}

void xu_iterator_reset(xu_iterator_t* it) {
    it->current = -1;
}

bool xu_iterator_next(xu_iterator_t* it) {
    int next = it->current + 1;
    if( next < it->classes->count ) {
        it->current = next;
        return true;
    }
    return false;
}

xu_class_t xu_iterator_current(xu_iterator_t* it) {
    xu_class_t result = { 0 };
    if( it->current < it->classes->count && it->current >= 0) {
        result.classlist = it->classes;
        result.classref = it->current;
    }
    return result;
}

xu_result_t xu_refresh_class(xu_class_t class) {

    xu_classlist_t* classes = class.classlist;
    int classref = class.classref;

    char* srcpath = classes->paths[classref];
    if( srcpath[0] == 0 )
        return XU_NO_CHANGE; // source from memory buffer

    if( program_file_exists(srcpath) == false )
        return XU_ERROR_INVALID_PARAM;

    time_t new_modtime = program_file_get_modtime(srcpath);
    if( classes->modtimes[classref] >= new_modtime )
        return XU_NO_CHANGE;

    classes->modtimes[classref] = new_modtime;

    source_code_t code = program_source_read_from_file(srcpath);
    program_t new_program = program_compile(&code, false);
    program_source_free(&code);

    // keep the old program if we fail to compile
    if( program_is_valid(&new_program) == false )
        return XU_ERROR_COMPILATION;

    // finalize / setup env
    ffi_t* ffi = &classes->interfaces[classref];   // reuse previous FFI
    vm_env_t new_env = { 0 };               // new ENV

    if(vm_env_setup(&new_env, &new_program, ffi) == false) {
        vm_env_destroy(&new_env);
        sh_log_error("xu_refresh: env setup failed");
        return XU_ERROR_ENV;
    }

    // destroy the old env
    vm_env_destroy(&classes->envs[classref]);

    // destroy the old program
    if( program_is_valid(&classes->programs[classref]) )
        program_destroy(&classes->programs[classref]);

    // assign the new version
    classes->envs[classref] = new_env;
    classes->programs[classref] = new_program;
    return XU_OK;
}


bool xu_finalize_all(xu_classlist_t* classes) {
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

void xu_cleanup_all(xu_classlist_t* classes) {
    for(int i = 0; i < classes->count; i++) {
        ffi_destroy(&classes->interfaces[i]);
        program_destroy(&classes->programs[i]);
        vm_env_destroy(&classes->envs[i]);
        classes->modtimes[i] = 0UL;
        memset(classes->paths[i], 0, 2048);
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
    buf[0] = '\0'; // reset previous
    cstr_t str = {
        .maxlen = 2048,
        .ptr = buf
    };
    vm_sprint_val(str, vm, val);
    return buf;
}