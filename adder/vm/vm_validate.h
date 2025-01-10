#ifndef VM_VALIDATE_H_
#define VM_VALIDATE_H_

#include "sh_types.h"
#include "sh_config.h"
#include "sh_value.h"
#include "sh_asminfo.h"
#include "vm_value_tools.h"
#include "vm_types.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <sh_log.h>

#if VM_RUNTIME_VALIDATION > 0

typedef struct validation_t {
    char message[257];
    vm_op_t last_opcode;
    unsigned int opcntr;
} validation_t;

inline static bool validation_init(vm_t* vm) {
    vm->validation = malloc(sizeof(validation_t));
    if( vm->validation != NULL ) {
        memset(vm->validation, 0, sizeof(validation_t));
        return true;
    }
    return false;
}

inline static void validation_destroy(vm_t* vm) {
    if( vm->validation != NULL ) {
        free((validation_t*) vm->validation);
    }
}

inline static bool validation_check_stack_arg_count(vm_t* vm, char* context, int nargs) {
    validation_t* validation = ((validation_t*)vm->validation);
    int stack_top = vm->mem.stack.top;
    if( (stack_top + 1) < nargs ) {
        snprintf(validation->message, 256,
            "'%s' requres %i args, but the stack is empty.\n",
            context, nargs);
        validation->message[256] = '\0';
        return false;
    }
    return true;
}

inline static bool validation_check_stack_args(vm_t* vm, char* context, int arg_count, ...) {
    validation_t* validation = ((validation_t*)vm->validation);
    if( validation_check_stack_arg_count(vm, context, arg_count) == false ) {
        return false;
    }
    va_list	arg_ptr;
    va_start(arg_ptr, arg_count);
    int stack_top = vm->mem.stack.top;
    for(int i = 0; i < arg_count; i++) {
        val_t arg = vm->mem.stack.values[stack_top - i];
        val_type_t arg_type = VAL_GET_TYPE(arg);
        val_type_t expected = va_arg(arg_ptr, val_type_t);
        if( expected != arg_type ) {
            snprintf(validation->message, 256,
                "'%s' arg #%i should have been %s but was %s.\n",
                context,
                i + 1,
                val_get_type_name(expected),
                val_get_type_name(arg_type));
            validation->message[256] = '\0';
            return false;
        }
    }
    return true;
}

inline static bool validation_check_stack(vm_t* vm, char* context) {
    validation_t* validation = ((validation_t*)vm->validation);
    if( vm->mem.stack.top >= vm->mem.stack.size ) {
        snprintf(validation->message, 256,
                "'%s' stack overflow.\n",
                context);
        validation->message[256] = '\0';
        return false;
    }
    if( vm->mem.stack.top < -1 ) {
        snprintf(validation->message, 256,
                "'%s' stack underflow.\n",
                context);
        validation->message[256] = '\0';
        return false;
    }
    int stack_frame = vm->mem.stack.frame;
    if( stack_frame < 0 ) {
        return true;
    }
    frame_t frame = val_into_frame(vm->mem.stack.values[stack_frame]);
    int frame_upper = stack_frame + frame.num_args + frame.num_locals;
    if( vm->mem.stack.top < frame_upper ) {
        snprintf(validation->message, 256,
            "'%s' stack call-frame compromised.\n",
            context);
        validation->message[256] = '\0';
        return false;
    }
    return true;
}

inline static bool validation_pre_exec(vm_t* vm, vm_op_t opcode) {
    validation_t* validation = ((validation_t*)vm->validation);
    char* op_name = get_op_name(opcode);
    bool no_error = true;
    if( validation->last_opcode == OP_CALL && opcode != OP_MAKE_FRAME  ) {
        snprintf(validation->message, 256,
                "the OP_CALL instruction must be immediatly followed by OP_MAKE_FRAME.\n");
        validation->message[256] = '\0';
        no_error = false;
    } else {
        switch (opcode) {
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_MOD:
            case OP_CMP_LESS_THAN:
            case OP_CMP_MORE_THAN:
            case OP_CMP_LESS_THAN_OR_EQUAL:
            case OP_CMP_MORE_THAN_OR_EQUAL:
            case OP_CMP_NOT_EQUAL:
            case OP_CMP_EQUAL: {
                no_error = validation_check_stack_args(vm, op_name, 2, VAL_NUMBER, VAL_NUMBER);
            } break;
            case OP_AND:
            case OP_OR: {
                no_error = validation_check_stack_args(vm, op_name, 2, VAL_BOOL, VAL_BOOL);
            } break;
            case OP_DUP_1:
            case OP_POP_1:
            case OP_RETURN_VALUE:
            case OP_PRINT: {
                no_error = validation_check_stack_arg_count(vm, op_name, 1);
            } break;
            case OP_POP_2:
            case OP_DUP_2:
            case OP_ROT_2: {
                no_error = validation_check_stack_arg_count(vm, op_name, 2);
            } break;
            case OP_NOT:
            case OP_JUMP_IF_FALSE: {
                no_error = validation_check_stack_args(vm, op_name, 1, VAL_BOOL);
            } break;
            case OP_NEG:
            case OP_MAKE_FRAME: {
                no_error = validation_check_stack_args(vm, op_name, 1, VAL_NUMBER);
            } break;
            case OP_STORE_LOCAL:
            case OP_LOAD_LOCAL: {
                val_t val = vm->mem.stack.values[vm->mem.stack.frame];
                frame_t frame = val_into_frame(val);
                int nreserved = (frame.num_locals + frame.num_args);
                int id = READ_U32(vm->run.instructions, vm->run.pc);
                if( id < 0 || id >= nreserved ) {
                    if( nreserved < 1 ) {
                        snprintf(validation->message, 256,
                            "'%s' tried to load local/arg with index %i "
                            "but the current frame has no reserved values.",
                            op_name, id);
                    } else {
                        snprintf(validation->message, 256,
                            "'%s' tried to load local with index %i "
                            "but the current frame has reserved #0 to #%i.",
                             op_name, id, frame.num_locals - 1);
                    }
                    validation->message[256] = '\0';
                    no_error = false;
                }
            } break;
            default: {
                /* nothing */
            } break;
        }
    }

    validation->last_opcode = opcode;
    validation->opcntr ++;

    if( no_error == false ) {
        sh_log_error("%s\n",
            validation->message);
        return false;
    }
    return true;
}

inline static bool validation_post_exec(vm_t* vm, vm_op_t opcode) {
    assert(OP_OPCODE_COUNT == 37 && "Opcode count changed.");
    char* op_name = get_op_name(opcode);
    validation_t* validation = ((validation_t*)vm->validation);
    bool no_error = true;
    switch (opcode) {
            case OP_PUSH_VALUE:
            case OP_POP_1:
            case OP_POP_2:
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_MOD:
            case OP_NEG:
            case OP_CMP_LESS_THAN_OR_EQUAL:
            case OP_CMP_MORE_THAN_OR_EQUAL:
            case OP_CMP_LESS_THAN:
            case OP_CMP_MORE_THAN:
            case OP_CMP_EQUAL:
            case OP_CMP_NOT_EQUAL:
            case OP_AND:
            case OP_OR:
            case OP_NOT:
            case OP_DUP_1:
            case OP_DUP_2:
            case OP_JUMP_IF_FALSE:
            case OP_CALL:
            case OP_MAKE_FRAME:
            case OP_PRINT:
            case OP_STORE_LOCAL:
            case OP_LOAD_LOCAL: {
                no_error = validation_check_stack(vm, op_name);
            } break;
            case OP_ROT_2:
            case OP_JUMP:
            case OP_HALT:
            case OP_EXIT: 
            case OP_RETURN_NOTHING:
            case OP_RETURN_VALUE:
            default: {
                /* nothing */
            } break;
    }
    if( no_error == false ) {
        sh_log_error("%s\n",
            validation->message);
        return false;
    }
    return true;
}
# define VALIDATION_INIT(VM) validation_init(VM)
# define VALIDATION_DESTROY(VM) validation_destroy(VM)
# define RETURN_ON_FAIL(COND) do { if( (COND) == false ) { return val_number(-9999); } } while(false)
# define VALIDATE_PRE(VM, OP) RETURN_ON_FAIL(validation_pre_exec(VM, OP))
# define VALIDATE_POST(VM, OP) RETURN_ON_FAIL(validation_post_exec(VM, OP))

#else

# define VALIDATION_INIT(VM)
# define VALIDATION_DESTROY(VM)
# define VALIDATE_PRE(VM, OP)
# define VALIDATE_POST(VM, OP)

#endif

#endif // VM_VALIDATE_H_
