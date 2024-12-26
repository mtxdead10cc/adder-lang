#include "sh_program.h"
#include "sh_value.h"
#include "sh_asminfo.h"
#include "sh_utils.h"
#include "sh_ffi.h"
#include <stdlib.h>
#include <string.h>
#include "sh_msg_buffer.h"
#include "sh_program.h"

void fprint_value(FILE* stream, val_t* memory, val_t val) {

    if( VAL_GET_TYPE(val) == VAL_ARRAY ) {
        array_t array = val_into_array(val);
        val_t* buffer = memory + MEM_ADDR_TO_INDEX(array.address);
        if( buffer == NULL ) {
            fprintf(stream, "<null buffer>");
            return;
        }
        int length = array.length;
        bool is_list = VAL_GET_TYPE(buffer[0]) != VAL_CHAR;
        if(is_list) {
            fprintf(stream, "[ ");
            for(int i = 0; i < length; i++) {
                fprint_value(stream, memory, buffer[i]);
                fprintf(stream, " ");
            }
            fprintf(stream, "]");
        } else { // string
            for(int i = 0; i < length; i++) {
                fprint_value(stream, memory, buffer[i]);
            }
        }
    } else {
        switch (VAL_GET_TYPE(val))
        {
        case VAL_NUMBER:
            fprintf(stream, "%f", val_into_number(val));
            break;
        case VAL_CHAR:
            fprintf(stream, "%c", val_into_char(val));
            break;
        case VAL_BOOL:
            fprintf(stream, "%s", val_into_bool(val) ? "TRUE" : "FALSE");
            break;
        case VAL_IVEC2: {
            ivec2_t v = val_into_ivec2(val);
            fprintf(stream, "(%i, %i)", v.x, v.y);
        } break;
        case VAL_ITER: {
            iter_t v = val_into_iter(val);
            fprintf(stream, "{curr:0x%08X, rem:%i}", v.current, v.remaining);
        } break;
        case VAL_FRAME: {
            frame_t frame = val_into_frame(val);
            fprintf(stream, "<pc: %i, nargs: %i, nlocals: %i>",
                frame.return_pc,
                frame.num_args,
                frame.num_locals);
        } break;
        case VAL_ARRAY: {
            array_t a = val_into_array(val);
            fprintf(stream, "[addr: 0x%08X, len: %d]",
                a.address, a.length);
            break;
        } break;
        default:
            fprintf(stream, "<unk>");
            break;
        }
    }
}

void program_disassemble(FILE* stream, program_t* program) {
    int current_byte = 0;
    val_t* consts = program->cons.buffer;
    uint8_t* instructions = program->inst.buffer;
    int instr_byte_count = program->inst.size;
    while( current_byte < instr_byte_count ) {
        vm_op_t opcode = instructions[current_byte];
        int arg_count = get_op_arg_count(opcode);
        if( arg_count < 0 ) {
            fprintf(stream, "<op %i not found>", opcode);
            current_byte ++;
            continue;
        }
        char* name = get_op_name(opcode);
        op_argtype_t* argtypes = get_op_arg_types(opcode);
        fprintf(stream, "#%5i| %-16s", current_byte, name);
        current_byte ++;
        for (int i = 0; i < arg_count; i++) {
            int val = (int) READ_U32(instructions, current_byte);
            fprintf(stream, " %-9i", val);
            current_byte += 4;
            if( argtypes[i] == OP_ARG_CONSTANT ) {
                fprintf(stream, " (");
                fprint_value(stream, consts, consts[val]);
                fprintf(stream, ")");
            }
        }
        fprintf(stream, "\n");
    }
}

int program_find_entrypoint(program_t* prog, sstr_t name, ffi_type_t* expected) {
    if( prog == NULL )
        return -1;
    for(int i = 0; i < prog->exports.count; i++) {
        ffi_definition_t def = prog->exports.def[i];
        if( sstr_equal(&name, &def.name) == false )
            continue;
        if( expected != NULL ) {
            if( ffi_type_equals(expected, def.type) == false )
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

entry_point_t program_get_entry_point(program_t* prog, char* name, ffi_type_t* type, sh_msg_buffer_t* msgbuf) {
    
    entry_point_t ep = {
        .argvals = { 0 },
        .argcount = -1,
        .address = -1
    };

    sh_msg_buffer_init(msgbuf, "[program - get entry point]");

    if( prog == NULL ) {
        sh_msg_buffer_append(msgbuf, sstr("get_entry_point: program was NULL"));
        return ep;
    }

    int max_args = sizeof(ep.argvals) / sizeof(ep.argvals[0]);

    if( name == NULL ) {
        // default entry point addr = 0
        int argc = 0;
        if( type != NULL )
            argc = ffi_get_func_arg_count(type);
        if( argc > max_args ) {
            sh_msg_buffer_append(msgbuf, sstr("get_entry_point: program requires unsupported arg count"));
            return ep;
        }
        ep.address = 0;
        ep.argcount = argc;
        return ep;
    }

    if( prog->exports.count <= 0 ) {
        sh_msg_buffer_append(msgbuf, sstr("get_entry_point: invalid program."));
        return ep;
    }

    int ep_index = program_find_entrypoint(prog, sstr(name), type);

    if( ep_index == -1 ) {
        sstr_t s = {0};
        sstr_append_fmt(&s, "'%s' not found", name);
        sh_msg_buffer_append(msgbuf, s);
        return ep;
    }
    
    if ( ep_index == -2 ) {
        sstr_t s = sstr("no match for type ");
        sstr_append_fmt(&s, "%s ", name);
        if( type != NULL ) {
            sstr_t t = ffi_type_to_sstr(type);
            sstr_append(&s, &t);
        }
        sh_msg_buffer_append(msgbuf, s);
        return ep;
    }

    uint32_t uaddress = prog->expaddr[ep_index];
    
    if( uaddress >= prog->inst.size ) {
        sstr_t s = {0};
        sstr_append_fmt(&s, "invalid entry point address %d", uaddress);
        sh_msg_buffer_append(msgbuf, s);
        return ep;
    }

    if( prog->inst.buffer[uaddress] != OP_MAKE_FRAME ) {
        sh_msg_buffer_append(msgbuf,
            sstr("entry point address is not pointing to a frame"));
        return ep;
    }

    int argc = 0;
    if( type != NULL )
        argc = ffi_get_func_arg_count(type);
    if( argc > max_args ) {
        sh_msg_buffer_append(msgbuf, sstr("get_entry_point: program requires unsupported arg count"));
        return ep;
    }

    ffi_type_t* t = prog->exports.def[ep_index].type;
    ep.address = uaddress;
    ep.argcount = ffi_get_func_arg_count(t);
    return ep;
}

bool entry_point_set_arg(entry_point_t* ep, int index, val_t arg) {
    if( index >= ep->argcount ) {
        printf("entry_point_set_arg: unsupported arg index %d (max %d)\n",
            index, ep->argcount-1);
        return false;
    }
    ep->argvals[index] = arg;
    return true;
}

void entry_point_set_arg_unsafe(entry_point_t* ep, int index, val_t arg) {
    ep->argvals[index] = arg;
    ep->argcount = max(ep->argcount, index + 1);
}



