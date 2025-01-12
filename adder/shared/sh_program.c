#include "sh_program.h"
#include "sh_value.h"
#include "sh_asminfo.h"
#include "sh_utils.h"
#include "sh_ffi.h"
#include "sh_ift.h"
#include <stdlib.h>
#include <string.h>
#include "sh_log.h"
#include "sh_program.h"

void sprint_value(cstr_t str, val_t* memory, val_t val) {

    if( VAL_GET_TYPE(val) == VAL_ARRAY ) {
        array_t array = val_into_array(val);
        val_t* buffer = memory + MEM_ADDR_TO_INDEX(array.address);
        if( buffer == NULL ) {
            cstr_append_fmt(str, "<null buffer>");
            return;
        }
        int length = array.length;
        bool is_list = VAL_GET_TYPE(buffer[0]) != VAL_CHAR;
        if(is_list) {
            cstr_append_fmt(str, "[ ");
            for(int i = 0; i < length; i++) {
                sprint_value(str, memory, buffer[i]);
                cstr_append_fmt(str, " ");
            }
            cstr_append_fmt(str, "]");
        } else { // string
            for(int i = 0; i < length; i++) {
                sprint_value(str, memory, buffer[i]);
            }
        }
    } else {
        switch (VAL_GET_TYPE(val))
        {
        case VAL_NUMBER:
            cstr_append_fmt(str, "%f", val_into_number(val));
            break;
        case VAL_CHAR:
            cstr_append_fmt(str, "%c", val_into_char(val));
            break;
        case VAL_BOOL:
            cstr_append_fmt(str, "%s", val_into_bool(val) ? "TRUE" : "FALSE");
            break;
        case VAL_IVEC2: {
            ivec2_t v = val_into_ivec2(val);
            cstr_append_fmt(str, "(%i, %i)", v.x, v.y);
        } break;
        case VAL_ITER: {
            iter_t v = val_into_iter(val);
            cstr_append_fmt(str, "{curr:0x%08X, rem:%i}", v.current, v.remaining);
        } break;
        case VAL_FRAME: {
            frame_t frame = val_into_frame(val);
            cstr_append_fmt(str, "<pc: %i, nargs: %i, nlocals: %i>",
                frame.return_pc,
                frame.num_args,
                frame.num_locals);
        } break;
        case VAL_ARRAY: {
            array_t a = val_into_array(val);
            cstr_append_fmt(str, "[addr: 0x%08X, len: %d]",
                a.address, a.length);
            break;
        } break;
        default:
            cstr_append_fmt(str, "<unk>");
            break;
        }
    }
}

bool program_is_valid(program_t* prog) {
    return prog->inst.size > 0;
}

#define DASM_LEN (2048*2)

void program_disassemble(program_t* program) {
    
    define_cstr(str, DASM_LEN);
    cstr_append_fmt(str, "[DISASSEMBLY]\n");

    int current_byte = 0;
    val_t* consts = program->cons.buffer;
    uint8_t* instructions = program->inst.buffer;
    int instr_byte_count = program->inst.size;
    while( current_byte < instr_byte_count ) {
        vm_op_t opcode = instructions[current_byte];
        int arg_count = get_op_arg_count(opcode);
        if( arg_count < 0 ) {
            cstr_append_fmt(str, "<op %i not found>", opcode);
            current_byte ++;
            continue;
        }
        char* name = get_op_name(opcode);
        op_argtype_t* argtypes = get_op_arg_types(opcode);
        cstr_append_fmt(str, "#%5i| %-16s", current_byte, name);
        current_byte ++;
        for (int i = 0; i < arg_count; i++) {
            int val = (int) READ_U32(instructions, current_byte);
            cstr_append_fmt(str, " %-9i", val);
            current_byte += 4;
            if( argtypes[i] == OP_ARG_CONSTANT ) {
                cstr_append_fmt(str, " (");
                sprint_value(str, consts, consts[val]);
                cstr_append_fmt(str, ")");
            }
        }
        cstr_append_fmt(str, "\n");
    }

    sh_log_info(str.ptr);
}

int program_find_entrypoint(program_t* prog, sstr_t name, ift_t* expected) {
    if( prog == NULL )
        return -1;
    for(int i = 0; i < prog->exports.count; i++) {
        ffi_definition_t def = prog->exports.def[i];
        if( sstr_equal(&name, &def.name) == false )
            continue;
        if( expected != NULL ) {
            if( ift_type_equals(expected, &def.type) == false )
                return -2;
        }
        return i;
    }
    return -1;
}

void program_destroy(program_t* prog) {

    if( prog == NULL ) {
        return;
    }

    if( prog->cons.buffer != NULL ) {
        free(prog->cons.buffer);
        prog->cons.count = 0;
        prog->cons.buffer = NULL;
    }

    if( prog->inst.buffer != NULL ) {
        free(prog->inst.buffer);
        prog->inst.size = 0;
        prog->inst.buffer = NULL;
    }

    ffi_definition_set_destroy(&prog->exports);
    ffi_definition_set_destroy(&prog->imports);

    if( prog->expaddr != NULL ) {
        free(prog->expaddr);
        prog->expaddr = NULL;
    }
}

entry_point_t entry_point_find_any(program_t* prog, char* name, ift_t* type) {
    
    entry_point_t ep = {
        .argvals = { 0 },
        .argcount = -1,
        .address = -1
    };

    if( prog == NULL ) {
        sh_log_error("get_entry_point: program was NULL");
        return ep;
    }

    int max_args = sizeof(ep.argvals) / sizeof(ep.argvals[0]);

    if( name == NULL ) {
        // default entry point addr = 0
        int argc = 0;
        if( type != NULL )
            argc = ift_func_arg_count(*type);
        if( argc > max_args ) {
            sh_log_error("get_entry_point: program requires unsupported arg count");
            return ep;
        }
        ep.address = 0;
        ep.argcount = argc;
        return ep;
    }

    if( prog->exports.count <= 0 ) {
        sh_log_error("get_entry_point: invalid program.");
        return ep;
    }

    int ep_index = program_find_entrypoint(prog, sstr(name), type);

    if( ep_index == -1 ) {
        sh_log_error("'%s' not found", name);
        return ep;
    }
    
    if ( ep_index == -2 ) {
        sstr_t s = sstr("");
        sstr_append_fmt(&s, "%s ", name);
        if( type != NULL ) {
            sstr_t t = ift_type_to_sstr(*type);
            sstr_append(&s, &t);
        }
        sh_log_error("no match for type %.*s", sstr_len(&s), sstr_ptr(&s));
        return ep;
    }

    uint32_t uaddress = prog->expaddr[ep_index];
    
    if( uaddress >= prog->inst.size ) {
        sh_log_error("invalid entry point address %d", uaddress);
        return ep;
    }

    if( prog->inst.buffer[uaddress] != OP_MAKE_FRAME ) {
        sh_log_error("entry point address is not pointing to a frame");
        return ep;
    }

    int argc = 0;
    if( type != NULL )
        argc = ift_func_arg_count(*type);
    if( argc > max_args ) {
        sh_log_error("get_entry_point: program requires unsupported arg count");
        return ep;
    }

    ift_t t = prog->exports.def[ep_index].type;
    ep.address = uaddress;
    ep.argcount = ift_func_arg_count(t);
    return ep;
}

entry_point_t program_entry_point_find(program_t* prog, char* name, ift_t type) {
    if( name == NULL ) {
        sh_log_error("program_entry_point_find: name was NULL");
        return (entry_point_t) {
            .argvals = { 0 },
            .argcount = -1,
            .address = -1
        };
    }
    return entry_point_find_any(prog, name, &type);
}

entry_point_t program_entry_point_find_any(program_t* prog, char* name) {
    if( name == NULL ) {
        sh_log_error("program_entry_point_find: name was NULL");
        return (entry_point_t) {
            .argvals = { 0 },
            .argcount = -1,
            .address = -1
        };
    }
    return entry_point_find_any(prog, name, NULL);
}

entry_point_t program_entry_point_find_default(program_t* prog) {
    return entry_point_find_any(prog, NULL, NULL);
}

bool program_entry_point_is_valid(entry_point_t entry_point) {
    return entry_point.address >= 0 && entry_point.argcount >= 0;
}

bool program_entry_point_set_arg(entry_point_t* ep, int index, val_t arg) {
    if( index >= ep->argcount ) {
        sh_log_error("program_entry_point_set_arg: unsupported arg index %d (max %d)\n",
            index, ep->argcount-1);
        return false;
    }
    ep->argvals[index] = arg;
    return true;
}

void program_entry_point_set_arg_unsafe(entry_point_t* ep, int index, val_t arg) {
    ep->argvals[index] = arg;
    ep->argcount = max(ep->argcount, index + 1);
}



