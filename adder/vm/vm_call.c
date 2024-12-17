#include "vm_call.h"
#include <stdlib.h>
#include <string.h>
#include "vm_msg_buffer.h"
#include "sh_program.h"

void vm_call_init(vm_call_t* inv, vm_program_t* program) {
    *inv = (vm_call_t) { 0 }; 
    inv->program = program;
    inv->ep.address = -1;
    vm_msg_buffer_init(&inv->msgbuf, "vm_call_t");
}

bool vm_call_set_arg(vm_call_t* inv, int arg_index, val_t val) {
    if( arg_index >= 0 && arg_index < inv->args.count ) {
        inv->args.vals[arg_index] = val;
        return true;
    }
    return false;
}

bool vm_call_set_arg_count(vm_call_t* inv, int arg_count) {
    int max_args = sizeof(inv->args.vals) / sizeof(inv->args.vals[0]);
    if( arg_count >= max_args ) {
        return false;
    }
    inv->args.count = arg_count;
    return true;
}

void vm_call_set_entry_unchecked(vm_call_t* inv, uint32_t address, int argcount) {
    inv->ep.address = address;
    inv->ep.argcount = argcount;
}

bool vm_call_set_entry(vm_call_t* inv, int index) {

    if( inv->program == NULL ) {
        vm_msg_buffer_append(&inv->msgbuf,
            sstr("program was NULL"));
        return false;
    }

    if( index < 0 || index >= inv->program->exports.count ) {
        sstr_t s = {0};
        sstr_append_fmt(&s,
            "invalid entry point index"
            " %i, expected %d to %d", index,
            0, inv->program->exports.count);
        vm_msg_buffer_append(&inv->msgbuf, s);
        return false;
    }

    uint32_t uaddress = inv->program->expaddr[index];
    if( uaddress >= inv->program->inst.size ) {
        sstr_t s = {0};
        sstr_append_fmt(&s, "invalid entry point address %d", uaddress);
        vm_msg_buffer_append(&inv->msgbuf, s);
        return false;
    }

    if( inv->program->inst.buffer[uaddress] != OP_MAKE_FRAME ) {
        vm_msg_buffer_append(&inv->msgbuf,
            sstr("entry point address is not pointing to a frame"));
        return false;
    }

    ffi_type_t* t = inv->program->exports.def[index].type;
    vm_call_set_entry_unchecked(inv, uaddress,
        ffi_get_func_arg_count(t));
    return true;
}

bool vm_call_lookup_entry(vm_call_t* inv, char* name, ffi_type_t* type_decr) {
    int index = program_find_entrypoint(inv->program, sstr(name), type_decr);

    if( index == -1 ) {
        sstr_t s = {0};
        sstr_append_fmt(&s, "'%s' not found", name);
        vm_msg_buffer_append(&inv->msgbuf, s);
        return false;
    }
    
    if ( index == -2 ) {
        sstr_t s = sstr("no match for type ");
        sstr_append_fmt(&s, "%s ", name);
        if( type_decr != NULL ) {
            sstr_t t = ffi_type_to_sstr(type_decr);
            sstr_append(&s, &t);
        }
        vm_msg_buffer_append(&inv->msgbuf, s);
        return false;
    }

    return vm_call_set_entry(inv, index);
}

bool vm_call_validate(vm_call_t* inv) {

    if( inv->program == NULL ) {
        vm_msg_buffer_append(&inv->msgbuf,
            sstr("program was NULL"));
        return false;
    }

    int given = inv->args.count;
    int required = inv->ep.argcount;

    if( given != required ) {
        sstr_t s = {0};
        sstr_append_fmt(&s, "program entry point expects %d args, but %d was provided",
            required, given);
        vm_msg_buffer_append(&inv->msgbuf, s);
        return false;
    }

    return true;
}