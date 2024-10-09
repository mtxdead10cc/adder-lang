#include "co_program.h"
#include "co_parser.h"
#include "co_compiler.h"
#include "sh_asminfo.h"
#include "sh_types.h"
#include "sh_value.h"
#include <unistd.h>

gvm_program_t gvm_program_compile_source(char* source, size_t source_len, char* filepath) {

    parser_t parser = { 0 };
    pa_result_t result = pa_init(&parser, source, source_len, filepath);
    if( par_is_error(result) ) {
        cres_fprint(stdout, (cres_t*) par_extract_error(result), filepath);
        pa_destroy(&parser);
        return (gvm_program_t) { 0 };
    }
    result = pa_parse_program(&parser);
    if( par_is_error(result) ) {
        cres_fprint(stdout, (cres_t*) par_extract_error(result), filepath);
        pa_destroy(&parser);
        return (gvm_program_t) { 0 };
    }
    if( par_is_nothing(result) ) {
        printf("error: empty program.\n");
        pa_destroy(&parser);
        return (gvm_program_t) { 0 };
    }

    ast_node_t* program_node = par_extract_node(result);
    cres_t status = {0};
    gvm_program_t program = gvm_compile(program_node, &status);
    if( cres_has_error(&status) ) {
        cres_fprint(stdout, &status, filepath);
    }
    ast_free(program_node);
    pa_destroy(&parser);
    return program;
}

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

void gvm_program_disassemble(FILE* stream, gvm_program_t* program) {
    int current_byte = 0;
    int current_instruction = 0;
    val_t* consts = program->cons.buffer;
    uint8_t* instructions = program->inst.buffer;
    int instr_byte_count = program->inst.size;
    while( current_byte < instr_byte_count ) {
        gvm_op_t opcode = instructions[current_byte];
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
        current_instruction ++;
    }
}

void gvm_program_destroy(gvm_program_t* prog) {
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

gvm_program_t gvm_program_read_and_compile(char* path) {

    FILE* f = fopen(path, "r");
    
    if( f == NULL ) {
        printf("error: %s not found.\n", path);
        return (gvm_program_t) { 0 };
    }

    char *source_text = malloc(1);
    int retry_counter = 100; 
    while( retry_counter > 0 ) {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        source_text = realloc(source_text, fsize + 1);
        if( fread(source_text, fsize, 1, f) > 0 ) {
            retry_counter = -10;
            source_text[fsize] = '\0';
        } else {
            usleep(100000);
            retry_counter --;
        }
    }

    fclose(f);

    if( retry_counter == 0 ) {
        printf("error: failed to read file: %s\n", path);
    }

    gvm_program_t program = gvm_program_compile_source(
        source_text,
        strlen(source_text),
        path);

    free(source_text);

    return program;
}