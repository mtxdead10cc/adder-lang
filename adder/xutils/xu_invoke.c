#include "xu_invoke.h"
#include "xu_lib.h"

val_t xu_int(int v) {
    return val_number(v);
}

val_t xu_float(float v) {
    return val_number(v);
}

val_t xu_bool(bool v) {
    return val_bool(v);
}

val_t xu_char(char v) {
    return val_char(v);
}

val_t call0(vm_t* vm, xu_caller_t* caller) {
    assert(caller->entrypoint.argcount == 0);
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];
    return vm_execute(vm, env, &caller->entrypoint, program);
}

void vcall0(vm_t* vm, xu_caller_t* caller) {
    assert(caller->entrypoint.argcount == 0);
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];
    vm_execute(vm, env, &caller->entrypoint, program);
}

val_t call1(vm_t* vm, xu_caller_t* caller, val_t arg1) {
    assert(caller->entrypoint.argcount == 1);
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];
    program_entry_point_set_arg(&caller->entrypoint, 0, arg1);
    return vm_execute(vm, env, &caller->entrypoint, program);
}

void vcall1(vm_t* vm, xu_caller_t* caller, val_t arg1) {
    assert(caller->entrypoint.argcount == 1);
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg1);

    vm_execute(vm, env, &caller->entrypoint, program);
}

val_t call2(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2) {
    assert(caller->entrypoint.argcount == 2);
    
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg2);
    
    return vm_execute(vm, env, &caller->entrypoint, program);
}

void vcall2(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2) {
    assert(caller->entrypoint.argcount == 2);
    
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg2);
    
    vm_execute(vm, env, &caller->entrypoint, program);
}

val_t call3(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3) {
    assert(caller->entrypoint.argcount == 3);
    
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg3);
    
    return vm_execute(vm, env, &caller->entrypoint, program);
}

void vcall3(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3) {
    assert(caller->entrypoint.argcount == 3);
    
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg3);
    
    vm_execute(vm, env, &caller->entrypoint, program);
}

val_t call4(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4) {
    assert(caller->entrypoint.argcount == 4);
    
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg4);
    
    return vm_execute(vm, env, &caller->entrypoint, program);
}

void vcall4(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4) {
    assert(caller->entrypoint.argcount == 4);
    
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg4);
    
    vm_execute(vm, env, &caller->entrypoint, program);
}

val_t call5(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5) {
    assert(caller->entrypoint.argcount == 5);
    
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg5);
    
    return vm_execute(vm, env, &caller->entrypoint, program);
}

void vcall5(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5) {
    assert(caller->entrypoint.argcount == 5);
    
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg5);
    
    vm_execute(vm, env, &caller->entrypoint, program);
}

val_t call6(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6) {
    assert(caller->entrypoint.argcount == 6);
    
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    
    return vm_execute(vm, env, &caller->entrypoint, program);
}

void vcall6(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6) {
    assert(caller->entrypoint.argcount == 6);
    
    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    
    vm_execute(vm, env, &caller->entrypoint, program);
}

int icall0(vm_t* vm, xu_caller_t* caller) {
    return val_into_number(call0(vm, caller));
}

float fcall0(vm_t* vm, xu_caller_t* caller) {
    return val_into_number(call0(vm, caller));
}

bool bcall0(vm_t* vm, xu_caller_t* caller) {
    return val_into_bool(call0(vm, caller));
}

char ccall0(vm_t* vm, xu_caller_t* caller) {
    return val_into_char(call0(vm, caller));
}

char* scall0(vm_t* vm, xu_caller_t* caller) {
    return xu_val_to_string(vm, call0(vm, caller));
}

int icall1(vm_t* vm, xu_caller_t* caller, val_t arg1) {
    return val_into_number(call1(vm, caller, arg1));
}

float fcall1(vm_t* vm, xu_caller_t* caller, val_t arg1) {
    return val_into_number(call1(vm, caller, arg1));
}

bool  bcall1(vm_t* vm, xu_caller_t* caller, val_t arg1) {
    return val_into_bool(call1(vm, caller, arg1));
}

char  ccall1(vm_t* vm, xu_caller_t* caller, val_t arg1) {
    return val_into_char(call1(vm, caller, arg1));
}

char* scall1(vm_t* vm, xu_caller_t* caller, val_t arg1) {
    return xu_val_to_string(vm, call1(vm, caller, arg1));
}

int icall2(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2) {
    return val_into_number(call2(vm, caller, arg1, arg2));
}

float fcall2(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2) {
    return val_into_number(call2(vm, caller, arg1, arg2));
}

bool  bcall2(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2) {
    return val_into_bool(call2(vm, caller, arg1, arg2));
}

char ccall2(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2) {
    return val_into_char(call2(vm, caller, arg1, arg2));
}

char* scall2(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2) {
    return xu_val_to_string(vm, call2(vm, caller, arg1, arg2));
}

int icall3(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3) {
    return val_into_number(call3(vm, caller, arg1, arg2, arg3));
}

float fcall3(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3) {
    return val_into_number(call3(vm, caller, arg1, arg2, arg3));
}

bool  bcall3(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3) {
    return val_into_bool(call3(vm, caller, arg1, arg2, arg3));
}

char  ccall3(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3) {
    return val_into_char(call3(vm, caller, arg1, arg2, arg3));
}

char* scall3(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3) {
    return xu_val_to_string(vm, call3(vm, caller, arg1, arg2, arg3));
}

int   icall4(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4) {
    return val_into_number(call4(vm, caller, arg1, arg2, arg3, arg4));
}

float fcall4(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4) {
    return val_into_number(call4(vm, caller, arg1, arg2, arg3, arg4));
}

bool  bcall4(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4) {
    return val_into_bool(call4(vm, caller, arg1, arg2, arg3, arg4));
}

char  ccall4(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4) {
    return val_into_char(call4(vm, caller, arg1, arg2, arg3, arg4));
}

char* scall4(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4) {
    return xu_val_to_string(vm, call4(vm, caller, arg1, arg2, arg3, arg4));
}

int icall5(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5) {
    return val_into_number(call5(vm, caller, arg1, arg2, arg3, arg4, arg5));
}

float fcall5(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5) {
    return val_into_number(call5(vm, caller, arg1, arg2, arg3, arg4, arg5));
}

bool bcall5(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5) {
    return val_into_bool(call5(vm, caller, arg1, arg2, arg3, arg4, arg5));
}

char ccall5(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5) {
    return val_into_char(call5(vm, caller, arg1, arg2, arg3, arg4, arg5));
}

char* scall5(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5) {
    return xu_val_to_string(vm, call5(vm, caller, arg1, arg2, arg3, arg4, arg5));
}

int icall6(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6) {
    return val_into_number(call6(vm, caller, arg1, arg2, arg3, arg4, arg5, arg6));
}

float fcall6(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6) {
    return val_into_number(call6(vm, caller, arg1, arg2, arg3, arg4, arg5, arg6));
}

bool bcall6(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6) {
    return val_into_bool(call6(vm, caller, arg1, arg2, arg3, arg4, arg5, arg6));
}

char ccall6(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6) {
    return val_into_char(call6(vm, caller, arg1, arg2, arg3, arg4, arg5, arg6));
}

char* scall6(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6) {
    return xu_val_to_string(vm, call6(vm, caller, arg1, arg2, arg3, arg4, arg5, arg6));
}
