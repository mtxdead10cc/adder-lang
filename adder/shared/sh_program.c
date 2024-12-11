#include "sh_program.h"
#include "sh_value.h"
#include "sh_asminfo.h"
#include "sh_utils.h"
#include <stdlib.h>

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

void program_disassemble(FILE* stream, vm_program_t* program) {
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

void program_destroy(vm_program_t* prog) {
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
}
