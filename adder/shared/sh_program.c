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

    if( val.type == VAL_ARRAY ) {
        array_t array = val_into_array(val);
        val_t* buffer = memory + MEM_ADDR_TO_INDEX(array.address);
        if( buffer == NULL ) {
            cstr_append_fmt(str, "<null buffer>");
            return;
        }
        int length = array.length;
        bool is_list = buffer[0].type != VAL_CHAR;
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
        switch (val.type)
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

int program_find_entrypoint_by_name_and_type(program_t* prog, sstr_t name, ift_t expected, bool retcheck) {

    if( prog == NULL )
        return -1;

    for(int i = 0; i < prog->exports.count; i++) {

        ffi_definition_t def = prog->exports.def[i];
        if( sstr_equal(&name, &def.name) == false )
            continue;

        if( retcheck ) {
            if( ift_type_equals(&expected, &def.type) == false )
                return -2;
        } else {
            if( ift_func_arglist_equals(&expected, &def.type) == false )
                return -2;
        }

        return i;
    }

    return -1;
}

int program_find_entrypoint_by_name(program_t* prog, sstr_t name) {
    if( prog == NULL )
        return -1;
    for(int i = 0; i < prog->exports.count; i++) {
        ffi_definition_t def = prog->exports.def[i];
        if( sstr_equal(&name, &def.name) == false )
            continue;
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

int entry_point_find_any(program_t* prog, char* name, ift_t type, entry_point_t* result) {
    
    *result = program_entry_point_invalid();

    if( prog == NULL ) {
        sh_log_error("get_entry_point: program was NULL");
        return PEP_INVALID_PROGRAM;
    }

    int max_args = sizeof(result->argvals) / sizeof(result->argvals[0]);
    int provided_argcount = ift_func_arg_count(type);
    if( provided_argcount > max_args ) {
        // call requires unsupported arg count
        return PEP_INVALID_ENTRY_POINT_ARG_COUNT;
    }
    
    if( name == NULL ) {
        // default entry point addr = 0
        *result = (entry_point_t) {
            .address = 0,
            .argcount = provided_argcount,
            .type = type
        };
        return PEP_OK;
    }

    if( prog->exports.count <= 0 ) {
        // invalid program, no entry points
        return PEP_INVALID_PROGRAM;
    }

    int ep_index = ift_is_unknown(type)
        ? program_find_entrypoint_by_name(prog, sstr(name))
        : program_find_entrypoint_by_name_and_type(prog, sstr(name), type, false);

    if( ep_index == -1 )
        return PEP_NAME_NOT_FOUND;    // name not found

    if( ep_index == -2 ) {
        int idx = program_find_entrypoint_by_name(prog, sstr(name));
        if( idx >= 0 ) // set the actual type (for error reporting)
            result->type = prog->exports.def[idx].type;
        return PEP_TYPE_NOT_MATCHING; // type not matching
    }

    uint32_t uaddress = prog->expaddr[ep_index];
    ift_t ep_type = prog->exports.def[ep_index].type;
    int ep_argcount = ift_func_arg_count(ep_type);

    if( ift_is_unknown(type) == false && (ep_argcount != provided_argcount) ) {
        // arg count not matching
        return PEP_INVALID_ENTRY_POINT_ARG_COUNT;
    }
    
    if( uaddress >= prog->inst.size ) {
        // invalid entry point address
        return PEP_INVALID_PROGRAM_ENTRY_POINT;
    }

    if( prog->inst.buffer[uaddress] != OP_MAKE_FRAME ) {
        // entry point address is not pointing to a frame
        return PEP_INVALID_PROGRAM_ENTRY_POINT;
    }

    (*result) = (entry_point_t) {
        .address = uaddress,
        .argcount = ep_argcount,
        .type = ep_type
    };

    return PEP_OK;
}

entry_point_t program_entry_point_invalid() {
    return (entry_point_t) {
        .argvals = {{ 0 }},
        .argcount = -1,
        .address = -1,
        .type = ift_unknown()
    };
}

int program_entry_point_find(program_t* prog, char* name, ift_t type, entry_point_t* result) {
    if( ift_is_unknown(type) ) {
        return PEP_TYPE_NOT_MATCHING;
    }
    if( name == NULL ) {
        return PEP_NAME_NOT_FOUND;
    }
    return entry_point_find_any(prog, name, type, result);
}

int program_entry_point_find_any(program_t* prog, char* name, entry_point_t* result) {
    if( name == NULL ) {
        return PEP_NAME_NOT_FOUND;
    }
    return entry_point_find_any(prog, name, ift_unknown(), result);
}

int program_entry_point_find_default(program_t* prog, entry_point_t* result) {
    return entry_point_find_any(prog, NULL, ift_unknown(), result);
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



