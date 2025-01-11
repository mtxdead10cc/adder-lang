
// GENERATED FILE

#include "xu_invoke.h"
#include "xu_lib.h"
#include <stdbool.h>
#include <sh_value.h>

bool bcall0(vm_t* vm, xu_caller_t* caller) {
    assert(caller->entrypoint.argcount == 0);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall0(vm_t* vm, xu_caller_t* caller) {
    assert(caller->entrypoint.argcount == 0);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall0(vm_t* vm, xu_caller_t* caller) {
    assert(caller->entrypoint.argcount == 0);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall0(vm_t* vm, xu_caller_t* caller) {
    assert(caller->entrypoint.argcount == 0);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall0(vm_t* vm, xu_caller_t* caller) {
    assert(caller->entrypoint.argcount == 0);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall0(vm_t* vm, xu_caller_t* caller) {
    assert(caller->entrypoint.argcount == 0);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall1(vm_t* vm, xu_caller_t* caller, val_t arg0) {
    assert(caller->entrypoint.argcount == 1);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall1(vm_t* vm, xu_caller_t* caller, val_t arg0) {
    assert(caller->entrypoint.argcount == 1);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall1(vm_t* vm, xu_caller_t* caller, val_t arg0) {
    assert(caller->entrypoint.argcount == 1);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall1(vm_t* vm, xu_caller_t* caller, val_t arg0) {
    assert(caller->entrypoint.argcount == 1);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall1(vm_t* vm, xu_caller_t* caller, val_t arg0) {
    assert(caller->entrypoint.argcount == 1);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall1(vm_t* vm, xu_caller_t* caller, val_t arg0) {
    assert(caller->entrypoint.argcount == 1);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall2(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1) {
    assert(caller->entrypoint.argcount == 2);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall2(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1) {
    assert(caller->entrypoint.argcount == 2);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall2(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1) {
    assert(caller->entrypoint.argcount == 2);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall2(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1) {
    assert(caller->entrypoint.argcount == 2);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall2(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1) {
    assert(caller->entrypoint.argcount == 2);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall2(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1) {
    assert(caller->entrypoint.argcount == 2);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall3(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2) {
    assert(caller->entrypoint.argcount == 3);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall3(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2) {
    assert(caller->entrypoint.argcount == 3);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall3(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2) {
    assert(caller->entrypoint.argcount == 3);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall3(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2) {
    assert(caller->entrypoint.argcount == 3);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall3(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2) {
    assert(caller->entrypoint.argcount == 3);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall3(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2) {
    assert(caller->entrypoint.argcount == 3);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall4(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3) {
    assert(caller->entrypoint.argcount == 4);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall4(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3) {
    assert(caller->entrypoint.argcount == 4);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall4(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3) {
    assert(caller->entrypoint.argcount == 4);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall4(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3) {
    assert(caller->entrypoint.argcount == 4);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall4(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3) {
    assert(caller->entrypoint.argcount == 4);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall4(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3) {
    assert(caller->entrypoint.argcount == 4);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall5(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4) {
    assert(caller->entrypoint.argcount == 5);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall5(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4) {
    assert(caller->entrypoint.argcount == 5);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall5(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4) {
    assert(caller->entrypoint.argcount == 5);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall5(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4) {
    assert(caller->entrypoint.argcount == 5);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall5(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4) {
    assert(caller->entrypoint.argcount == 5);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall5(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4) {
    assert(caller->entrypoint.argcount == 5);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall6(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5) {
    assert(caller->entrypoint.argcount == 6);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall6(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5) {
    assert(caller->entrypoint.argcount == 6);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall6(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5) {
    assert(caller->entrypoint.argcount == 6);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall6(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5) {
    assert(caller->entrypoint.argcount == 6);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall6(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5) {
    assert(caller->entrypoint.argcount == 6);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall6(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5) {
    assert(caller->entrypoint.argcount == 6);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall7(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6) {
    assert(caller->entrypoint.argcount == 7);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall7(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6) {
    assert(caller->entrypoint.argcount == 7);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall7(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6) {
    assert(caller->entrypoint.argcount == 7);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall7(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6) {
    assert(caller->entrypoint.argcount == 7);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall7(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6) {
    assert(caller->entrypoint.argcount == 7);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall7(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6) {
    assert(caller->entrypoint.argcount == 7);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall8(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7) {
    assert(caller->entrypoint.argcount == 8);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall8(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7) {
    assert(caller->entrypoint.argcount == 8);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall8(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7) {
    assert(caller->entrypoint.argcount == 8);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall8(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7) {
    assert(caller->entrypoint.argcount == 8);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall8(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7) {
    assert(caller->entrypoint.argcount == 8);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall8(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7) {
    assert(caller->entrypoint.argcount == 8);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall9(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8) {
    assert(caller->entrypoint.argcount == 9);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall9(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8) {
    assert(caller->entrypoint.argcount == 9);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall9(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8) {
    assert(caller->entrypoint.argcount == 9);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall9(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8) {
    assert(caller->entrypoint.argcount == 9);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall9(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8) {
    assert(caller->entrypoint.argcount == 9);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall9(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8) {
    assert(caller->entrypoint.argcount == 9);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall10(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9) {
    assert(caller->entrypoint.argcount == 10);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall10(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9) {
    assert(caller->entrypoint.argcount == 10);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall10(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9) {
    assert(caller->entrypoint.argcount == 10);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall10(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9) {
    assert(caller->entrypoint.argcount == 10);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall10(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9) {
    assert(caller->entrypoint.argcount == 10);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall10(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9) {
    assert(caller->entrypoint.argcount == 10);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall11(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10) {
    assert(caller->entrypoint.argcount == 11);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall11(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10) {
    assert(caller->entrypoint.argcount == 11);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall11(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10) {
    assert(caller->entrypoint.argcount == 11);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall11(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10) {
    assert(caller->entrypoint.argcount == 11);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall11(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10) {
    assert(caller->entrypoint.argcount == 11);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall11(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10) {
    assert(caller->entrypoint.argcount == 11);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall12(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11) {
    assert(caller->entrypoint.argcount == 12);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall12(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11) {
    assert(caller->entrypoint.argcount == 12);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall12(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11) {
    assert(caller->entrypoint.argcount == 12);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall12(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11) {
    assert(caller->entrypoint.argcount == 12);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall12(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11) {
    assert(caller->entrypoint.argcount == 12);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall12(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11) {
    assert(caller->entrypoint.argcount == 12);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall13(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12) {
    assert(caller->entrypoint.argcount == 13);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall13(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12) {
    assert(caller->entrypoint.argcount == 13);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall13(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12) {
    assert(caller->entrypoint.argcount == 13);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall13(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12) {
    assert(caller->entrypoint.argcount == 13);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall13(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12) {
    assert(caller->entrypoint.argcount == 13);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall13(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12) {
    assert(caller->entrypoint.argcount == 13);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall14(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13) {
    assert(caller->entrypoint.argcount == 14);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall14(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13) {
    assert(caller->entrypoint.argcount == 14);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall14(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13) {
    assert(caller->entrypoint.argcount == 14);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall14(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13) {
    assert(caller->entrypoint.argcount == 14);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall14(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13) {
    assert(caller->entrypoint.argcount == 14);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall14(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13) {
    assert(caller->entrypoint.argcount == 14);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall15(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13, val_t arg14) {
    assert(caller->entrypoint.argcount == 15);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);
    program_entry_point_set_arg(&caller->entrypoint, 14, arg14);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall15(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13, val_t arg14) {
    assert(caller->entrypoint.argcount == 15);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);
    program_entry_point_set_arg(&caller->entrypoint, 14, arg14);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall15(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13, val_t arg14) {
    assert(caller->entrypoint.argcount == 15);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);
    program_entry_point_set_arg(&caller->entrypoint, 14, arg14);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall15(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13, val_t arg14) {
    assert(caller->entrypoint.argcount == 15);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);
    program_entry_point_set_arg(&caller->entrypoint, 14, arg14);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall15(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13, val_t arg14) {
    assert(caller->entrypoint.argcount == 15);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);
    program_entry_point_set_arg(&caller->entrypoint, 14, arg14);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall15(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13, val_t arg14) {
    assert(caller->entrypoint.argcount == 15);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);
    program_entry_point_set_arg(&caller->entrypoint, 14, arg14);

    vm_execute(vm, env, &caller->entrypoint, program);
}

bool bcall16(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13, val_t arg14, val_t arg15) {
    assert(caller->entrypoint.argcount == 16);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);
    program_entry_point_set_arg(&caller->entrypoint, 14, arg14);
    program_entry_point_set_arg(&caller->entrypoint, 15, arg15);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_bool(result);
}

int icall16(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13, val_t arg14, val_t arg15) {
    assert(caller->entrypoint.argcount == 16);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);
    program_entry_point_set_arg(&caller->entrypoint, 14, arg14);
    program_entry_point_set_arg(&caller->entrypoint, 15, arg15);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

float fcall16(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13, val_t arg14, val_t arg15) {
    assert(caller->entrypoint.argcount == 16);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);
    program_entry_point_set_arg(&caller->entrypoint, 14, arg14);
    program_entry_point_set_arg(&caller->entrypoint, 15, arg15);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_number(result);
}

char ccall16(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13, val_t arg14, val_t arg15) {
    assert(caller->entrypoint.argcount == 16);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);
    program_entry_point_set_arg(&caller->entrypoint, 14, arg14);
    program_entry_point_set_arg(&caller->entrypoint, 15, arg15);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return val_into_char(result);
}

char* scall16(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13, val_t arg14, val_t arg15) {
    assert(caller->entrypoint.argcount == 16);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);
    program_entry_point_set_arg(&caller->entrypoint, 14, arg14);
    program_entry_point_set_arg(&caller->entrypoint, 15, arg15);

    val_t result = vm_execute(vm, env, &caller->entrypoint, program);
    return xu_val_to_string(vm, result);
}

void vcall16(vm_t* vm, xu_caller_t* caller, val_t arg0, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6, val_t arg7, val_t arg8, val_t arg9, val_t arg10, val_t arg11, val_t arg12, val_t arg13, val_t arg14, val_t arg15) {
    assert(caller->entrypoint.argcount == 16);

    xu_classlist_t* classes = caller->class.classlist;
    int ref = caller->class.classref;
    vm_env_t* env = &classes->envs[ref];
    program_t* program = &classes->programs[ref];

    program_entry_point_set_arg(&caller->entrypoint, 0, arg0);
    program_entry_point_set_arg(&caller->entrypoint, 1, arg1);
    program_entry_point_set_arg(&caller->entrypoint, 2, arg2);
    program_entry_point_set_arg(&caller->entrypoint, 3, arg3);
    program_entry_point_set_arg(&caller->entrypoint, 4, arg4);
    program_entry_point_set_arg(&caller->entrypoint, 5, arg5);
    program_entry_point_set_arg(&caller->entrypoint, 6, arg6);
    program_entry_point_set_arg(&caller->entrypoint, 7, arg7);
    program_entry_point_set_arg(&caller->entrypoint, 8, arg8);
    program_entry_point_set_arg(&caller->entrypoint, 9, arg9);
    program_entry_point_set_arg(&caller->entrypoint, 10, arg10);
    program_entry_point_set_arg(&caller->entrypoint, 11, arg11);
    program_entry_point_set_arg(&caller->entrypoint, 12, arg12);
    program_entry_point_set_arg(&caller->entrypoint, 13, arg13);
    program_entry_point_set_arg(&caller->entrypoint, 14, arg14);
    program_entry_point_set_arg(&caller->entrypoint, 15, arg15);

    vm_execute(vm, env, &caller->entrypoint, program);
}



