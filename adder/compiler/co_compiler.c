#include "sh_asminfo.h"
#include "co_utils.h"
#include "co_compiler.h"
#include "co_srcmap.h"
#include "co_trace.h"
#include "co_bty.h"
#include <assert.h>

typedef struct ir_inst_t {
    vm_op_t opcode;
    uint32_t args[2];
} ir_inst_t;

typedef struct ir_list_t {
    uint32_t count;
    uint32_t capacity;
    ir_inst_t* irs;
} ir_list_t;

typedef enum ir_index_tag_t {
    IRID_INVALID = 0,
    IRID_VAR,
    IRID_INS,
    IRID_EXT
} ir_index_tag_t;

typedef struct ir_index_t {
    ir_index_tag_t tag;
    uint32_t    idx;
} ir_index_t;

bool irl_init(ir_list_t* list, uint32_t capacity) {
    list->count = 0;
    list->irs = (ir_inst_t*) malloc( sizeof(ir_inst_t) * capacity );
    if( list->irs == NULL ) {
        return false;
    }
    list->capacity = capacity;
    return true;
}

void irl_destroy(ir_list_t* list) {
    if( list->irs != NULL ) {
        free(list->irs);
    }
    list->irs = NULL;
    list->capacity = 0;
    list->count = 0;
}

bool irl_reserve(ir_list_t* list, uint32_t additional) {
    uint32_t required = (list->count + additional);
    if( list->capacity <= required ) {
        ir_inst_t* ptr = (ir_inst_t*) realloc(list->irs, sizeof(ir_inst_t) * required);
        if( ptr == NULL ) {
            return false;
        }
        list->irs = ptr;
    }
    return true;
}

ir_index_t irl_add(ir_list_t* list, ir_inst_t instr) {
    if( irl_reserve(list, 1) == false ) {
        printf("error: out of memory, can't realloc.");
        return (ir_index_t) {0};
    }
    list->irs[list->count++] = instr;
    return (ir_index_t) {
        .idx = list->count - 1,
        .tag = IRID_INS
    };
}

ir_inst_t* irl_get(ir_list_t* list, ir_index_t index) {
    assert(index.tag == IRID_INS && "index has mismatching tag");
    return list->irs + index.idx;
}

ir_inst_t* irl_get_last(ir_list_t* list) {
    assert(list->count > 0 && "cant get last instruction (empty instruction list)");
    return list->irs + (list->count - 1);
}

void irl_dump(ir_list_t* list) {
    for(uint32_t i = 0; i < list->count; i++) {
        int argcount = get_op_arg_count(list->irs[i].opcode);
        printf("%03d #  ('%s'", i, get_op_name(list->irs[i].opcode));
        for (int j = 0; j < argcount; j++) {
            printf(" %d", list->irs[i].args[j]);
        }
        printf(")\n");
    }
}

size_t get_node_content_length(ast_node_t* args) {
    switch (args->type) {
        case AST_ARGLIST:   return args->u.n_args.count;
        case AST_ARRAY:     return args->u.n_array.count;
        case AST_BLOCK:     return args->u.n_block.count;
        case AST_FUN_CALL:
        case AST_UNOP:
        case AST_BINOP:
        case AST_VALUE:     return 1;
        default:            return 0;
    }
}

typedef struct compiler_state_t {
    srcmap_t                localvars;
    srcmap_t                functions;
    ffi_host_t*           ffi;
    ir_list_t               instrs;
    valbuffer_t             consts;
    trace_t*                trace;
    bty_ctx_t*              tyctx;
} compiler_state_t;

#define ABORT_ON_ERROR(STATE) do { if(trace_get_error_count((STATE)->trace) > 0) return; } while(false)

bool state_add_localvar(compiler_state_t* state, srcref_t name) {
    srcmap_value_t value = (srcmap_value_t) {
        .data = (uint32_t) state->localvars.count
    };
    return srcmap_insert(&state->localvars, name, value);
}

ir_index_t state_get_localvar(compiler_state_t* state, srcref_t name) {
    srcmap_value_t* val = srcmap_lookup(&state->localvars, name);
    if( val != NULL ) {
        return (ir_index_t) {
            .idx = val->data,
            .tag = IRID_VAR
        };
    }
    return (ir_index_t) {
        .idx = 0,
        .tag = IRID_INVALID
    };
}

bool state_add_funcaddr(compiler_state_t* state, srcref_t name, ir_index_t index) {
    assert(index.tag == IRID_INS && "received incorrect index type");
    srcmap_value_t value = (srcmap_value_t) {
        .data = (uint32_t) index.idx
    };
    return srcmap_insert(&state->functions, name, value);
}

ir_index_t state_get_funcaddr(compiler_state_t* state, srcref_t name) {
    srcmap_value_t* val = srcmap_lookup(&state->functions, name);
    if( val != NULL ) {
        return (ir_index_t) {
            .idx = val->data,
            .tag = IRID_INS
        };
    }
    return (ir_index_t) {
        .idx = 0,
        .tag = IRID_INVALID
    };
}

void codegen(ast_node_t* node, compiler_state_t* state);

void codegen_binop(ast_binop_t node, compiler_state_t* state) {
    
    ABORT_ON_ERROR(state);

    codegen(node.right, state);
    codegen(node.left, state);
    switch(node.type) {
        case AST_BIN_ADD: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_ADD,
                .args = { 0 }
            });
        } break;
        case AST_BIN_SUB: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_SUB,
                .args = { 0 }
            });
        } break;
        case AST_BIN_MUL: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_MUL,
                .args = { 0 }
            });
        } break;
        case AST_BIN_DIV: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_DIV,
                .args = { 0 }
            });
        } break;
        case AST_BIN_MOD: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_MOD,
                .args = { 0 }
            });
        } break;
        case AST_BIN_AND: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_AND,
                .args = { 0 }
            });
        } break;
        case AST_BIN_OR: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_OR,
                .args = { 0 }
            });
        } break;
        case AST_BIN_EQ: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_CMP_EQUAL,
                .args = { 0 }
            });
        } break;
        case AST_BIN_NEQ: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_CMP_NOT_EQUAL,
                .args = { 0 }
            });
        } break;
        case AST_BIN_LT: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_CMP_LESS_THAN,
                .args = { 0 }
            });
        } break;
        case AST_BIN_GT: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_CMP_MORE_THAN,
                .args = { 0 }
            });
        } break;
         case AST_BIN_LT_EQ: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_CMP_LESS_THAN_OR_EQUAL,
                .args = { 0 }
            });
        } break;
        case AST_BIN_GT_EQ: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_CMP_MORE_THAN_OR_EQUAL,
                .args = { 0 }
            });
        } break;
        default: {
            trace_msg_t* msg = trace_create_message(state->trace, TM_ERROR, trace_no_ref());            
            trace_msg_append_costr(msg, "unhandled binary operation: ");
            char* m = ast_binop_type_as_string(node.type);
            trace_msg_append(msg, m, strlen(m));
        } break;
    }
}

void codegen_unop(ast_unop_t node, compiler_state_t* state) {
    
    ABORT_ON_ERROR(state);

    codegen(node.inner, state);
    
    switch(node.type) {
        case AST_UN_NEG: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_NEG,
                .args = { 0 }
            });
        } break;
        case AST_UN_NOT: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_NOT,
                .args = { 0 }
            });
        } break;
        default: {
            trace_msg_t* msg = trace_create_message(state->trace, TM_ERROR, trace_no_ref());            
            trace_msg_append_costr(msg, "unhandled unary operation: ");
            char* m = ast_unop_type_as_string(node.type);
            trace_msg_append(msg, m, strlen(m));
        } break;
    }
}

void codegen_value(ast_value_t node, compiler_state_t* state) {

    ABORT_ON_ERROR(state);

    vb_result_t append_result = (vb_result_t) { 0 };

    switch(node.type) {
        case AST_VALUE_BOOL: {
            append_result = valbuffer_insert_bool(&state->consts, node.u._bool);
        } break;
        case AST_VALUE_FLOAT: {
            append_result = valbuffer_insert_float(&state->consts, node.u._float);
        } break;
        case AST_VALUE_INT: {
            append_result = valbuffer_insert_int(&state->consts, node.u._int);
        } break;
        case AST_VALUE_CHAR: {
            append_result = valbuffer_insert_char(&state->consts, node.u._char);
        } break;
        default: {
            trace_msg_t* msg = trace_create_message(state->trace, TM_ERROR, trace_no_ref());
            trace_msg_append_costr(msg, "unsupported value type: ");
            char* typename = ast_value_type_string(node.type);
            trace_msg_append(msg,
                typename, strlen(typename));
            return;
        } break;
    }

    if( append_result.out_of_memory ) {
        trace_out_of_memory_error(state->trace);
        return;
    }
    
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_PUSH_VALUE,
        .args = { append_result.index, 0 }
    });
}

void codegen_fundecl(ast_fundecl_t node, compiler_state_t* state) {

    ABORT_ON_ERROR(state);

    srcref_t funcname = node.name;

    if ( state->localvars.count > 0 ) {
        trace_msg_t* msg = trace_create_message(state->trace, TM_ERROR, funcname);
        trace_msg_append_costr(msg,
            "functions may not be declared "
            "inside other functions.");
        return;
    }

    ir_index_t frame_index = irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_MAKE_FRAME,
        .args = { 0 }
    });

    bool ok = state_add_funcaddr(state, funcname, frame_index);

    (void)(ok); // unused in release builds

    assert(ok && "the function already exists");
    
    srcmap_clear(&state->localvars);

    codegen(node.argspec, state); // in order to "add" arg names

    uint32_t arg_count = (uint32_t) state->localvars.count;
    codegen(node.body, state); // adds locals to frame

    // if the last instruction is not a return statement
    // we insert a value less return at the end.
    vm_op_t last_op_code = irl_get_last(&state->instrs)->opcode;
    if( last_op_code != OP_RETURN_VALUE && last_op_code != OP_RETURN_NOTHING ) {
        irl_add(&state->instrs, (ir_inst_t){
            .opcode = OP_RETURN_NOTHING,
            .args = { 0 }
        });
    }

    uint32_t locals_count = ((uint32_t) state->localvars.count) - arg_count;
    irl_get(&state->instrs, frame_index)->args[0] = arg_count;
    irl_get(&state->instrs, frame_index)->args[1] = locals_count;
    srcmap_clear(&state->localvars);
}

void codegen_funcall(ast_funcall_t node, compiler_state_t* state) {

    ABORT_ON_ERROR(state);

    codegen(node.args, state);

    ir_index_t ir_index = state_get_funcaddr(state, node.name);

    if( ir_index.tag == IRID_INS ) {
        // if tag invalid: could not find index
        // of function name (not defined)
        irl_add(&state->instrs, (ir_inst_t){
            .opcode = OP_CALL,
            .args = { ir_index.idx, 0 }
        });
        return;
    }

    int ext_index = ffi_host_index_of(state->ffi,
        srcref_as_sstr(node.name));

    if ( ext_index >= 0 ) {

        ffi_handle_tag_t tag = state->ffi->handle[ext_index].tag;
        if( tag != FFI_HNDL_HOST_ACTION && tag != FFI_HNDL_HOST_FUNCTION ) {
            trace_not_implemented(state->trace,
                "Value properties (FFI_HNDL_VALUE_READ_ONLY, FFI_HNDL_VALUE)");
            return;
        }

        irl_add(&state->instrs, (ir_inst_t){
            .opcode = OP_CALL_NATIVE,
            .args = { ext_index, 0 }
        });
        return;
    }

    /* 
       ASSUMPTION: we only end up here if the user
       is trying to call an extern function that is
       not present in the FFI bundle.
       Since a missing "normal" function should have 
       been caught by the type checker.
    */

    trace_msg_t* msg = trace_create_message(state->trace, TM_ERROR, node.name);
    trace_msg_append_costr(msg, "the function '");
    trace_msg_append_srcref(msg, node.name);
    trace_msg_append_costr(msg, "' could not be found.");
}

void codegen_assignment(ast_assign_t node, compiler_state_t* state) {

    ABORT_ON_ERROR(state);

    codegen(node.right_value, state);

    srcref_t varname = ast_try_extract_name(node.left_var);
    

    ast_node_type_t left_node_type = node.left_var->type;

    assert( left_node_type == AST_TYANNOT || left_node_type == AST_VAR_REF );
    assert(srcref_is_valid(varname));

    // if variable with type annotation:
    //    it is a new var, so add it to known locals
    // if just a variable; already known (do not add)

    if( left_node_type == AST_TYANNOT ) {
        assert(node.left_var->u.n_tyannot.expr->type == AST_VAR_REF);
        codegen(node.left_var, state); // add var to known locals
    }

    assert(state->localvars.count > 0 && "local vars was empty");

    ir_index_t index = state_get_localvar(state, varname);
    assert(index.tag == IRID_VAR && "varname not found");
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_STORE_LOCAL,
        .args = { (uint32_t) index.idx, 0 }
    });
}

void codegen_foreach(ast_foreach_t node, compiler_state_t* state) {
    
    ABORT_ON_ERROR(state);

    codegen(node.collection, state);
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_MAKE_ITER,
        .args = { 0 }
    });
    ir_index_t loop_start_index = irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_ITER_NEXT,
        .args = { 0 }
    });
    codegen(node.vardecl, state); // add varname
    srcref_t varname = ast_try_extract_name(node.vardecl);
    assert(srcref_is_valid(varname));
    ir_index_t varindex = state_get_localvar(state, varname);
    assert(varindex.tag == IRID_VAR && "variable not found");
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_STORE_LOCAL,
        .args = { varindex.idx, 0 }
    });
    codegen(node.during, state);
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_JUMP,
        .args = { loop_start_index.idx, 0 }
    });
    irl_get(&state->instrs, loop_start_index)->args[0] = state->instrs.count;
}

int get_if_chain_length(ast_node_t* current) {
    int count = 0;
    while(current != NULL) {
        if( current->type == AST_IF_CHAIN ) {
            count ++;
            current = current->u.n_if.next;
        } else {
            break;
        }
    }
    return count;
}

void codegen_if_chain(ast_node_t* node, compiler_state_t* state) {
    
    ABORT_ON_ERROR(state);

    int chain_len = get_if_chain_length(node);
    ir_index_t exit_indices[chain_len];

    ast_node_t* current = node;
    int count = 0;
    ir_index_t if_next_index;

    while( current->type == AST_IF_CHAIN ) {

        // 1. if not cond <jump: next>      (or jump to end if no else or else if)
        // 2. body                          (always generate)
        // 3. <jump: end>                   (skip if no trailing else or else if)
        // 4. next ...                      (go to next if or exit to else or nothing)
        // N. [else] ... 
        // N+1. end

        // 1)
        codegen(current->u.n_if.cond, state);

        if_next_index = irl_add(
            &state->instrs,
            (ir_inst_t){
                .opcode = OP_JUMP_IF_FALSE,
                .args = { 0 }
            });

        // 2)
        codegen(current->u.n_if.iftrue, state);

        current = current->u.n_if.next;

        // 3)
        if( current->type == AST_IF_CHAIN || ast_is_valid_else_block(current) ) {
            // if we're at the last block 
            // we make sure to not jump 
            // since we get a corrupt jump 
            // index (same position).  
            exit_indices[count++] = irl_add(
                &state->instrs,
                (ir_inst_t){
                    .opcode = OP_JUMP,
                    .args = { 0 }
                });
        }

        // 4)
        ir_inst_t* instr = irl_get(&state->instrs, if_next_index);
        instr->args[0] = state->instrs.count;
    }

    // N)
    if( ast_is_valid_else_block(current) )
        codegen(current, state);
    
    // N+1) set the exit jump points
    for(int i = 0; i < count; i++) {
        irl_get(&state->instrs, exit_indices[i])->args[0] = state->instrs.count;
    }
}

void codegen_return_stmt(ast_return_t stmt, compiler_state_t* state) {
    uint32_t ret_size = 0;
    switch (stmt.result->type) {
        case AST_VALUE:
        case AST_VAR_REF:
        case AST_ARRAY:
        case AST_FUN_CALL:
        case AST_BINOP:
        case AST_UNOP: {
            ret_size = 1;
        } break;
        case AST_BLOCK: {
            ret_size = stmt.result->u.n_block.count;
        } break;
        case AST_IF_CHAIN:
        case AST_FOREACH:
        case AST_ASSIGN:
        case AST_TYANNOT:
        case AST_FUN_DECL:
        case AST_FUN_EXDECL:
        case AST_RETURN:
        case AST_ARGLIST:
        case AST_BREAK: {
            ret_size = 0;
        } break;
    }
    if( ret_size == 0 ) {
        irl_add(&state->instrs, (ir_inst_t){
            .opcode = OP_RETURN_NOTHING,
            .args = { 0 }
        });
    } else { 
        codegen(stmt.result, state);
        irl_add(&state->instrs, (ir_inst_t){
            .opcode = OP_RETURN_VALUE,
            .args = { 0 }
        });
    }
}

ffi_type_t* bty_to_ffi_type(bty_type_t* t) {
    if( t == NULL )
        return NULL;
    switch(t->tag) {
        case BTY_VOID:  return ffi_void();
        case BTY_BOOL:  return ffi_bool();
        case BTY_INT:   return ffi_int();
        case BTY_FLOAT: return ffi_float();
        case BTY_CHAR:  return ffi_char();
        case BTY_LIST:  return ffi_list(bty_to_ffi_type(t->u.con));
        case BTY_FUNC:  {
            ffi_type_t* ot = ffi_func(bty_to_ffi_type(t->u.fun.ret));
            for(int i = 0; i < t->u.fun.argc; i++) {
                ffi_func_add_arg(ot, bty_to_ffi_type(t->u.fun.args[i]));
            }
            return ot;
        }
        default: {
            printf("error: bty_to_ffi_type unhandled bty_type %d\n", t->tag);
            return ffi_custom("unknown_type");
        }
    }
}

void verify_funexdecl(ast_funexdecl_t funex, compiler_state_t* state) {

    ABORT_ON_ERROR(state);

    ffi_type_t* ffi_type = ffi_host_get_type(state->ffi,
        srcref_as_sstr(funex.name));

    bty_type_t* bty_type = bty_ctx_lookup(state->tyctx, funex.name);
    ffi_type_t* lang_type = bty_to_ffi_type(bty_type);

    if( ffi_type == NULL ) {

        /* If the ffi type could not be found
           chances are that the function is never
           actually called.
           If the missing function is called then the 
           user will get an error at the call site. */

        trace_msg_t* msg = trace_create_message(state->trace,
            TM_WARNING,
            funex.name);
        trace_msg_append_costr(msg,
            "external (imported) function '");
        trace_msg_append_srcref(msg, funex.name);
        if( lang_type != NULL ) {
            trace_msg_append_ffi_type(msg, lang_type);
        }
        trace_msg_append_costr(msg,
            "' could not be found in the provided FFI bundle.");

    } else {

        if( bty_type == NULL || lang_type == NULL ) {

            trace_msg_t* msg = trace_create_message(state->trace,
                TM_INTERNAL_ERROR,
                funex.name);
            trace_msg_append_costr(msg,
                "external (imported) function validation failed for '");
            trace_msg_append_srcref(msg, funex.name);
            trace_msg_append_ffi_type(msg, ffi_type);
            trace_msg_append_costr(msg, 
                "'\nThis is likely an internal error "
                "in the compiler (or type checker).");

        } else if( ffi_equals(ffi_type, lang_type) == false ) {

            trace_msg_t* msg = trace_create_message(state->trace,
                TM_ERROR,
                funex.name);
            trace_msg_append_costr(msg, "unexpected function import '");
            trace_msg_append_srcref(msg, funex.name);
            trace_msg_append_costr(msg, "' type signature. ");
            trace_msg_append_costr(msg, "\n\tDefined FFI type: ");
            trace_msg_append_srcref(msg, funex.name);
            trace_msg_append_ffi_type(msg, ffi_type);
            trace_msg_append_costr(msg, "\n\tSource file type: ");
            trace_msg_append_srcref(msg, funex.name);
            trace_msg_append_ffi_type(msg, lang_type);
        }
    }

    ffi_recfree(lang_type);
}

void codegen(ast_node_t* node, compiler_state_t* state) {

    ABORT_ON_ERROR(state);

    switch(node->type) {
        case AST_BINOP: {
            codegen_binop(node->u.n_binop, state);
        } break;
        case AST_UNOP: {
            codegen_unop(node->u.n_unop, state);
        } break;
        case AST_ASSIGN: {
            codegen_assignment(node->u.n_assign, state);
        } break;
        case AST_RETURN: {
            codegen_return_stmt(node->u.n_return, state);
        } break;
        case AST_ARRAY: {
            size_t count = node->u.n_array.count;
            for(size_t i = 0; i < count; i++) {
                codegen(node->u.n_array.content[i], state);
            }
            vb_result_t app_res = valbuffer_insert_int(&state->consts, (int)count);
            if( app_res.out_of_memory ) {
                trace_out_of_memory_error(state->trace);
                return;
            }
            uint32_t const_index = app_res.index;
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_PUSH_VALUE,
                .args = { (uint32_t) const_index, 0 }
            });
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_MAKE_ARRAY,
                .args = { 0 }
            });
        } break;
        case AST_BLOCK: {
            size_t count = node->u.n_block.count;
            for(size_t i = 0; i < count; i++) {
                codegen(node->u.n_block.content[i], state);
            }
        } break;
        case AST_ARGLIST: {
            size_t count = node->u.n_args.count;
            for(size_t i = 0; i < count; i++) {
                codegen(node->u.n_args.content[i], state);
            }
        } break;
        case AST_IF_CHAIN: {
            codegen_if_chain(node, state);
        } break;
        case AST_FOREACH: {
            codegen_foreach(node->u.n_foreach, state);
        } break;
        case AST_FUN_DECL: {
            codegen_fundecl(node->u.n_fundecl, state);
        } break;
        case AST_FUN_CALL: {
            codegen_funcall(node->u.n_funcall, state);
        } break;
        case AST_FUN_EXDECL: {
            verify_funexdecl(node->u.n_funexdecl, state);
        } break;
        case AST_VAR_REF: {
            ir_index_t var_index = state_get_localvar(state, node->u.n_varref.name);
            assert(var_index.tag == IRID_VAR && "variable not found");
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_LOAD_LOCAL,
                .args = { var_index.idx, 0 }
            });
        } break;
        case AST_VALUE: {
            codegen_value(node->u.n_value, state);
        } break;
        case AST_TYANNOT: {
            if( node->u.n_tyannot.expr->type == AST_VAR_REF ) {
                ast_node_t* var = node->u.n_tyannot.expr;
                // just add valiable name to frame local var set.
                state_add_localvar(state, var->u.n_varref.name);
            } else {
                // this is a function annotated with its return type
                ast_node_t* expr = node->u.n_tyannot.expr;
                (void)(expr); // unused in release builds
                assert( expr->type == AST_FUN_DECL || expr->type == AST_FUN_EXDECL );
                codegen(node->u.n_tyannot.expr, state);
            }
        } break;
        case AST_BREAK: {
            assert(false && "break op is not implemented yet");
        } break;

    }
}

void recalc_index_to_bytecode_adress(ir_list_t* instrs, uint32_t* idx2addr) {
    uint32_t max_addr = idx2addr[instrs->count];
    for (uint32_t i = 0; i < instrs->count; i++) {
        switch(instrs->irs[i].opcode) {
            case OP_CALL:
            case OP_ITER_NEXT:
            case OP_JUMP:
            case OP_JUMP_IF_FALSE: {
                uint32_t index = instrs->irs[i].args[0];
                assert(idx2addr[index] <= max_addr);
                instrs->irs[i].args[0] = idx2addr[index];
            } break;
            default: break;
        }
    }
}

void create_index_to_addr_map(ir_list_t* instrs, uint32_t* idx2addr, uint32_t size) {
    uint32_t addr = 0;
    const uint32_t argbytes = 4; // 32-bit args
    assert(instrs->count + 1 <= size);
    for (uint32_t i = 0; i < instrs->count; i++) {
        idx2addr[i] = addr;
        uint32_t argcount = get_op_arg_count(instrs->irs[i].opcode);
        addr = addr + (argcount * argbytes) + 1;
    }
    // needed for jumps landing
    // after last instruction
    // happens for if ... else ...
    idx2addr[instrs->count] = addr; 
}

vm_program_t write_program(ir_list_t* instrs, valbuffer_t* consts, ffi_host_t* ffi, uint32_t* idx2addr) {

    recalc_index_to_bytecode_adress(instrs, idx2addr);

    u8buffer_t bytecode;
    u8buffer_create(&bytecode, instrs->count);

    for (uint32_t i = 0; i < instrs->count; i++) {
        u8buffer_write(&bytecode, (uint8_t) instrs->irs[i].opcode);
        uint32_t argcount = get_op_arg_count(instrs->irs[i].opcode);
        for (uint32_t j = 0; j < argcount; j++) {
            uint32_t value = instrs->irs[i].args[j];
            u8buffer_write(&bytecode, (uint8_t) ((value >> (8*0)) & 0xFF));
            u8buffer_write(&bytecode, (uint8_t) ((value >> (8*1)) & 0xFF));
            u8buffer_write(&bytecode, (uint8_t) ((value >> (8*2)) & 0xFF));
            u8buffer_write(&bytecode, (uint8_t) ((value >> (8*3)) & 0xFF));
        }
    }

    val_t* const_buf = (val_t*) malloc( sizeof(val_t) * consts->size );
    memcpy(const_buf, consts->values, sizeof(val_t) * consts->size);

    uint8_t* code_buf = (uint8_t*) malloc( sizeof(uint8_t) * bytecode.size );
    memcpy(code_buf, bytecode.data, sizeof(uint8_t) * bytecode.size );

    vm_program_t result = (vm_program_t) {
        .cons.buffer = const_buf,
        .cons.count = consts->size,
        .inst.buffer = code_buf,
        .inst.size = bytecode.size,
        .ffi = ffi
    };

    u8buffer_destroy(&bytecode);
    return result;
}

bool check_entry_points(compiler_state_t* state) {

    int missing_count = 0;

    ir_index_t index = state_get_funcaddr(state, srcref_const("main"));
    if(index.tag != IRID_INS) {
        missing_count ++;
        trace_msg_t* msg = trace_create_message(state->trace, TM_ERROR, trace_no_ref());
        trace_msg_append_costr(msg, "no main() function found in program.\n");
    }

    if( state->ffi == NULL ) {
        // no ffi
        return missing_count == 0;
    }

    ffi_host_t* ffi = state->ffi;
    for(int i = 0; i < ffi->count; i++) {

        if( ffi->handle->tag != FFI_PROGRAM_REQUIREMENT )
            continue;

        srcref_t name = srcref(
            sstr_ptr(&ffi->name[i]), 0, 
            sstr_len(&ffi->name[i]));

        ir_index_t index = state_get_funcaddr(state, name);
        bty_type_t* script_type_bty = bty_ctx_lookup(state->tyctx, name);
        ffi_type_t* script_type_ffi = bty_to_ffi_type(script_type_bty);

        bool missing = false;

        if( index.tag != IRID_INS || script_type_bty == NULL || script_type_ffi == NULL ) {
            missing = true;
        }
        
        if( script_type_bty->tag == BTY_FUNC ) {
            missing = script_type_bty->u.fun.exported;
        } else {
            missing = true;
        }

        if( missing ) {
            missing_count ++;
            trace_msg_t* msg = trace_create_message(state->trace, TM_ERROR, trace_no_ref());
            trace_msg_append_costr(msg, "FFI required function ");
            trace_msg_append_srcref(msg, name);
            trace_msg_append_costr(msg, " could not be found.\n");
            trace_msg_append_costr(msg, "Did you forget to export the function?");
            continue;
        }

        if( ffi_equals(ffi->type[i], script_type_ffi) == false ) {
            missing_count ++;
            trace_msg_t* msg = trace_create_message(state->trace,
                TM_ERROR,
                trace_no_ref());
            trace_msg_append_costr(msg, "unexpected export function '");
            trace_msg_append_srcref(msg, name);
            trace_msg_append_costr(msg, "' type signature. ");
            trace_msg_append_costr(msg, "\n\tDefined FFI type: ");
            trace_msg_append_srcref(msg, name);
            trace_msg_append_ffi_type(msg, ffi->type[i]);
            trace_msg_append_costr(msg, "\n\tSource file type: ");
            trace_msg_append_srcref(msg, name);
            trace_msg_append_ffi_type(msg, script_type_ffi);
            continue;
        }
    }

    return missing_count == 0;
}

vm_program_t gvm_compile(arena_t* arena, ast_node_t* node, trace_t* trace, ffi_host_t* ffi) {

    vm_program_t program = { 0 };

    trace_clear(trace);

    compiler_state_t state = (compiler_state_t) {
        .trace = trace,
        .ffi = ffi,
        .tyctx = bty_ctx_create(arena, trace, 16)
    };

    if( bty_typecheck(state.tyctx, node) == false ) {
        return program;
    }

    if( srcmap_init(&state.functions, 16) == false ) {
        trace_out_of_memory_error(state.trace);
        return program;
    }
    
    if( srcmap_init(&state.localvars, 16) == false ) {
        trace_out_of_memory_error(state.trace);
        srcmap_destroy(&state.functions);
        return program;
    }

    if( irl_init(&state.instrs, 16) == false ) {
        trace_out_of_memory_error(state.trace);
        srcmap_destroy(&state.functions);
        srcmap_destroy(&state.localvars);
        return program;
    }

    if( valbuffer_create(&state.consts, 16) == false ) {
        trace_out_of_memory_error(state.trace);
        srcmap_destroy(&state.functions);
        srcmap_destroy(&state.localvars);
        irl_destroy(&state.instrs);
        return program;
    }

    // generate the code
    codegen(node, &state);

    // add final halt instruction
    irl_add(&state.instrs, (ir_inst_t) {
        .opcode = OP_HALT,
        .args = { 0 }
    });

    check_entry_points(&state);

    if( trace_get_error_count(state.trace) == 0 ) {
        
        uint32_t idx2addr_count = state.instrs.count + 1;
        uint32_t idx2addr[idx2addr_count];

        create_index_to_addr_map(&state.instrs, idx2addr, idx2addr_count);

        program = write_program(&state.instrs, &state.consts, ffi, idx2addr);

        // allocate req_count + 1 in order to include main
        program.eps.count = 1 + ffi_host_get_count(ffi, FFI_PROGRAM_REQUIREMENT);
        program.eps.addrs = (uint32_t*) malloc(program.eps.count * sizeof(uint32_t));
        assert(program.eps.addrs != NULL && "out of memory");

        // write required entrypoints to ffi
        int ep_index = 0;
        if( ffi != NULL ) {
            for( int i = 0; i < ffi->count; i++ ) {
                if( ffi->handle[i].tag != FFI_PROGRAM_REQUIREMENT )
                    continue;
                srcref_t name = srcref(
                    sstr_ptr(&ffi->name[i]), 0, 
                    sstr_len(&ffi->name[i]));
                ir_index_t index = state_get_funcaddr(&state, name);
                assert( index.tag == IRID_INS );
                assert( index.idx < idx2addr_count );
                program.eps.addrs[ep_index++] = idx2addr[index.idx];
            }
        }
        
        // write the main entry point
        ir_index_t main_index = state_get_funcaddr(&state, srcref_const("main"));
        assert( main_index.tag == IRID_INS );
        assert( main_index.idx < idx2addr_count );
        program.eps.addrs[ep_index] = idx2addr[main_index.idx];
    }
    
    valbuffer_destroy(&state.consts);
    irl_destroy(&state.instrs);
    srcmap_destroy(&state.localvars);
    srcmap_destroy(&state.functions);

    return program;
}
