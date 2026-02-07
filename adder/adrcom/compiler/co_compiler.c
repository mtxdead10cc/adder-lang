#include "adrcom/compiler/co_compiler.h"
#include "adrcom/parser/co_srcmap.h"

#include <adrcom/shared/co_utils.h>
#include <adrcom/shared/co_trace.h>

#include <adrcom/typechecker/co_bty.h>

#include <shared/sh_asminfo.h>
#include <shared/sh_log.h>

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
        sh_log_error("out of memory, can't realloc.");
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

void irl_dump(cstr_t str, ir_list_t* list) {
    for(uint32_t i = 0; i < list->count; i++) {
        int argcount = get_op_arg_count(list->irs[i].opcode);
        cstr_append_fmt(&str, "%03d #  ('%s'", i, get_op_name(list->irs[i].opcode));
        for (int j = 0; j < argcount; j++) {
            cstr_append_fmt(&str, " %d", list->irs[i].args[j]);
        }
        cstr_append_fmt(&str, ")\n");
    }
}

typedef struct compiler_state_t {
    srcmap_t                localvars;
    srcmap_t                functions;
    ffi_definition_set_t    host_supplied;
    ffi_definition_set_t    program_supplied;
    ir_list_t               instrs;
    valbuffer_t             consts;
    trace_t*                trace;
    bty_ctx_t*              tyctx;
} compiler_state_t;

#define ABORT_ON_ERROR(STATE) do { if(trace_get_error_count((STATE)->trace) > 0) return; } while(false)

bool state_add_localvar(compiler_state_t* state, sstr_t name) {
    srcmap_value_t value = (srcmap_value_t) {
        .data = (uint32_t) state->localvars.count
    };
    return srcmap_insert(&state->localvars, name, value);
}

ir_index_t state_get_localvar(compiler_state_t* state, sstr_t name) {
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

bool state_add_funcaddr(compiler_state_t* state, sstr_t name, ir_index_t index) {
    assert(index.tag == IRID_INS && "received incorrect index type");
    srcmap_value_t value = (srcmap_value_t) {
        .data = (uint32_t) index.idx
    };
    return srcmap_insert(&state->functions, name, value);
}

ir_index_t state_get_funcaddr(compiler_state_t* state, sstr_t name) {
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

void codegen(ast_t* node, compiler_state_t* state);

void codegen_binop(ast_t* node, compiler_state_t* state) {
    
    ABORT_ON_ERROR(state);

    codegen(node->as.items[1], state); // right
    codegen(node->as.items[0], state); // left
    switch(node->tag) {
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
            const char* m = ast_tag_to_string(node->tag);
            trace_msg_append(msg, (char*) m, strlen(m));
        } break;
    }
}

void codegen_unop(ast_t* node, compiler_state_t* state) {
    
    ABORT_ON_ERROR(state);

    codegen(node->as.items[0], state);
    
    switch(node->tag) {
        case AST_UNA_NEG: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_NEG,
                .args = { 0 }
            });
        } break;
        case AST_UNA_NOT: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_NOT,
                .args = { 0 }
            });
        } break;
        default: {
            trace_msg_t* msg = trace_create_message(state->trace, TM_ERROR, trace_no_ref());            
            trace_msg_append_costr(msg, "unhandled unary operation: ");
            const char* m = ast_tag_to_string(node->tag);
            trace_msg_append(msg, (char*) m, strlen(m));
        } break;
    }
}

void codegen_value(ast_t* node, compiler_state_t* state) {

    ABORT_ON_ERROR(state);

    vb_result_t append_result = (vb_result_t) { 0 };

    switch(node->tag) {
        case AST_BOOL: {
            append_result = valbuffer_insert_bool(&state->consts, node->as._bool);
        } break;
        case AST_FLOAT: {
            append_result = valbuffer_insert_float(&state->consts, node->as._float);
        } break;
        case AST_INT: {
            append_result = valbuffer_insert_int(&state->consts, node->as._int);
        } break;
        case AST_CHAR: {
            append_result = valbuffer_insert_char(&state->consts, node->as._char);
        } break;
        default: {
            trace_msg_t* msg = trace_create_message(state->trace, TM_ERROR, trace_no_ref());
            trace_msg_append_costr(msg, "unsupported value type: ");
            const char* typename = ast_tag_to_string(node->tag);
            trace_msg_append(msg,
                (char*) typename, strlen(typename));
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

ift_t bty_to_ffi_type(bty_type_t* t) {
    if( t == NULL )
        return ift_unknown();
    switch(t->tag) {
        case BTY_VOID:  return ift_void();
        case BTY_BOOL:  return ift_bool();
        case BTY_INT:   return ift_int();
        case BTY_FLOAT: return ift_float();
        case BTY_CHAR:  return ift_char();
        case BTY_LIST:  return ift_list(bty_to_ffi_type(t->u.con));
        case BTY_FUNC:  {
            ift_t ot = ift_func(bty_to_ffi_type(t->u.fun.ret));
            for(int i = 0; i < t->u.fun.argc; i++) {
                ot = ift_func_add_arg(ot, bty_to_ffi_type(t->u.fun.args[i]));
            }
            return ot;
        }
        default: {
            sh_log_error("bty_to_ffi_type unhandled bty_type %d\n", t->tag);
            return ift_unknown();
        }
    }
}


void add_host_provided_function_definition(srcref_t name, compiler_state_t* state) {
    ABORT_ON_ERROR(state);

    bty_type_t* bty_type = bty_ctx_lookup(state->tyctx, srcref_as_sstr(name));
    ift_t lang_type = bty_to_ffi_type(bty_type);

    if( bty_type == NULL || ift_is_unknown(lang_type) ) {
        trace_msg_t* msg = trace_create_message(state->trace,
            TM_INTERNAL_ERROR,
            name);
        trace_msg_append_costr(msg,
            "Imported (host supplied) function registration failed for '");
        trace_msg_append_srcref(msg, name);
        trace_msg_append_costr(msg, 
            "'\nThis is likely an internal error "
            "in the compiler (or type checker).");
        return;
    }

    bool ok = ffi_definition_set_add(&state->host_supplied,
        srcref_as_sstr(name), 
        lang_type);

    if( ok == false ) {
        trace_msg_t* msg = trace_create_message(state->trace,
            TM_INTERNAL_ERROR,
            name);
        trace_msg_append_costr(msg,
            "Failed to register imported (host supplied) function '");
        trace_msg_append_srcref(msg, name);
        trace_msg_append_costr(msg, 
            "'.\nThe host system might be out of memory.");
    }
}

void add_program_provided_function_definition(srcref_t name, compiler_state_t* state) {
    ABORT_ON_ERROR(state);

    bty_type_t* bty_type = bty_ctx_lookup(state->tyctx, srcref_as_sstr(name));
    ift_t lang_type = bty_to_ffi_type(bty_type);

    if( bty_type == NULL || ift_is_unknown(lang_type) ) {
        trace_msg_t* msg = trace_create_message(state->trace,
            TM_INTERNAL_ERROR,
            name);
        trace_msg_append_costr(msg,
            "Exported (program supplied) function "
            "registration failed for '");
        trace_msg_append_srcref(msg, name);
        trace_msg_append_costr(msg, 
            "'\nThis is likely an internal error "
            "in the compiler (or type checker).");
        return;
    }

    bool ok = ffi_definition_set_add(&state->program_supplied,
        srcref_as_sstr(name), 
        lang_type);

    if( ok == false ) {
        trace_msg_t* msg = trace_create_message(state->trace,
            TM_INTERNAL_ERROR,
            name);
        trace_msg_append_costr(msg,
            "Failed to register exported (program supplied) function '");
        trace_msg_append_srcref(msg, name);
        trace_msg_append_costr(msg, 
            "'.\nThe host system might be out of memory.");
    }
}

void codegen_fundef(ast_t* node, compiler_state_t* state) {

    ABORT_ON_ERROR(state);

    srcref_t funcname = ast_try_get_name(node);

    bool is_main = srcref_equals_string(funcname, "main");

    if( ast_is_exported(node) == false && is_main) {
        trace_msg_t* msg = trace_create_message(state->trace,
            TM_INTERNAL_ERROR, funcname);
        trace_msg_append_costr(msg,
            "the main function was not "
            "marked as exported.");
        return;
    }

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

    bool ok = state_add_funcaddr(state, srcref_as_sstr(funcname), frame_index);

    (void)(ok); // unused in release builds

    assert(ok && "the function already exists");
    
    srcmap_clear(&state->localvars);

    ast_t* funsign = node->as.items[AST_FUNDEFN_FUNSIGN];
    codegen(funsign->as.items[AST_FUNSIGN_ARGLIST], state); // in order to "add" arg names

    uint32_t arg_count = (uint32_t) state->localvars.count;
    codegen(node->as.items[AST_FUNDEFN_BODY], state); // adds locals to frame

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

void codegen_funcall(ast_t* node, compiler_state_t* state) {

    ABORT_ON_ERROR(state);

    codegen(ast_try_get(node, AST_ARGLIST), state);

    srcref_t name = ast_try_get_name(node);
    ir_index_t ir_index = state_get_funcaddr(state, srcref_as_sstr(name));

    if( ir_index.tag == IRID_INS ) {
        // if tag invalid: could not find index
        // of function name (not defined)
        irl_add(&state->instrs, (ir_inst_t){
            .opcode = OP_CALL,
            .args = { ir_index.idx, 0 }
        });
        return;
    }

    int ext_index = ffi_definition_set_index_of(&state->host_supplied,
        srcref_as_sstr(name));

    if ( ext_index >= 0 ) {
        irl_add(&state->instrs, (ir_inst_t){
            .opcode = OP_CALL_NATIVE,
            .args = { ext_index, 0 }
        });
        return;
    }

    trace_msg_t* msg = trace_create_message(state->trace, TM_ERROR, name);
    trace_msg_append_costr(msg, "the function '");
    trace_msg_append_srcref(msg, name);
    trace_msg_append_costr(msg, "' could not be found.");
}

void codegen_assignment(ast_t* node, compiler_state_t* state) {

    ABORT_ON_ERROR(state);

    codegen(node->as.items[AST_ASSIGN_RIGHT], state); // right

    srcref_t varname = ast_try_get_name(node->as.items[AST_ASSIGN_LEFT]); // left
    
    ast_tag_t left_node_type = node->as.items[AST_ASSIGN_LEFT]->tag;

    assert(left_node_type == AST_VARDECL || left_node_type == AST_VARREF);
    assert(srcref_is_valid(varname));

    // if variable declaration:
    //    it is a new var, so add it to known locals
    // if just a variable; already known (do not add)

    if( left_node_type == AST_VARDECL ) {
        codegen(node->as.items[AST_ASSIGN_LEFT], state); // add var to known locals
    }

    assert(state->localvars.count > 0 && "local vars was empty");

    ir_index_t index = state_get_localvar(state, srcref_as_sstr(varname));
    assert(index.tag == IRID_VAR && "varname not found");
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_STORE_LOCAL,
        .args = { (uint32_t) index.idx, 0 }
    });
}

void codegen_foreach(ast_t* node, compiler_state_t* state) {
    
    ABORT_ON_ERROR(state);

    codegen(node->as.items[AST_FOREACH_COLL], state); // loop collection
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_MAKE_ITER,
        .args = { 0 }
    });
    ir_index_t loop_start_index = irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_ITER_NEXT,
        .args = { 0 }
    });
    codegen(node->as.items[AST_FOREACH_VAR], state); // add vardecl
    srcref_t varname = ast_try_get_name(node->as.items[0]);
    assert(srcref_is_valid(varname));
    ir_index_t varindex = state_get_localvar(state, srcref_as_sstr(varname));
    assert(varindex.tag == IRID_VAR && "variable not found");
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_STORE_LOCAL,
        .args = { varindex.idx, 0 }
    });
    codegen(node->as.items[AST_FOREACH_BODY], state); // loop body
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_JUMP,
        .args = { loop_start_index.idx, 0 }
    });
    irl_get(&state->instrs, loop_start_index)->args[0] = state->instrs.count;
}

int get_if_chain_length(ast_t* current) {
    int count = 0;
    while(current != NULL) {
        if( current->tag == AST_IFCHAIN ) {
            count ++;
            current = current->as.items[2]; // next
        } else {
            break;
        }
    }
    return count;
}

void codegen_if_chain(ast_t* node, compiler_state_t* state) {
    
    ABORT_ON_ERROR(state);

    int chain_len = get_if_chain_length(node);
    ir_index_t exit_indices[chain_len];

    ast_t* current = node;
    int count = 0;
    ir_index_t if_next_index;

    while( current->tag == AST_IFCHAIN ) {

        // 1. if not cond <jump: next>      (or jump to end if no else or else if)
        // 2. body                          (always generate)
        // 3. <jump: end>                   (skip if no trailing else or else if)
        // 4. next ...                      (go to next if or exit to else or nothing)
        // N. [else] ... 
        // N+1. end

        // 1)
        codegen(current->as.items[AST_IFCHAIN_COND], state);

        if_next_index = irl_add(
            &state->instrs,
            (ir_inst_t){
                .opcode = OP_JUMP_IF_FALSE,
                .args = { 0 }
            });

        // 2)
        codegen(current->as.items[AST_IFCHAIN_IFTRUE], state);

        current = current->as.items[AST_IFCHAIN_IFNEXT];

        // 3)
        if( current->tag == AST_IFCHAIN || ast_is_valid_else_block(current) ) {
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

void codegen_return_stmt(ast_t* stmt, compiler_state_t* state) {
    uint32_t ret_size = 0;
    ast_t* inner = stmt->as.items[AST_RETURN_EXPR];
    switch (inner->tag) {
        case AST_IFCHAIN:
        case AST_FOREACH:
        case AST_ASSIGN:
        case AST_TYDESCR:
        case AST_FUNDEFN:
        case AST_VARDECL:
        case AST_RETURN:
        case AST_ARGLIST: {
            ret_size = 0;
        } break;
        case AST_VARREF:
        case AST_FUNCALL: {
            ret_size = 1;
        } break;
        case AST_BLOCK: {
            ret_size = inner->size;
        } break;
        default: {
            assert(inner->tag != AST_SYMBOL && "can a srcref be returned?");
            if(ast_is_value(inner) || inner->tag == AST_ARRAY)
                ret_size = 1;
            else if(ast_is_binop(inner))
                ret_size = 1;
            else if(ast_is_unop(inner))
                ret_size = 1;
            else
                ret_size = 0;
        } break;
    }
    if( ret_size == 0 ) {
        irl_add(&state->instrs, (ir_inst_t){
            .opcode = OP_RETURN_NOTHING,
            .args = { 0 }
        });
    } else { 
        codegen(inner, state);
        irl_add(&state->instrs, (ir_inst_t){
            .opcode = OP_RETURN_VALUE,
            .args = { 0 }
        });
    }
}

void codegen(ast_t* node, compiler_state_t* state) {

    ABORT_ON_ERROR(state);

    switch(node->tag) {
        case AST_ASSIGN: {
            codegen_assignment(node, state);
        } break;
        case AST_RETURN: {
            codegen_return_stmt(node, state);
        } break;
        case AST_STRING: {
            srcref_t str = node->as._srcref;
            int count = ((int) srcref_len(str)) - 2;
            char* ptr = srcref_ptr(str) + 1;
            for(int i = 0; i < count; i++) {
                vb_result_t cr = valbuffer_insert_char(&state->consts, ptr[i]);
                irl_add(&state->instrs, (ir_inst_t){
                    .opcode = OP_PUSH_VALUE,
                    .args = { cr.index, 0 }
                });
            }
            vb_result_t ins_res = valbuffer_insert_int(&state->consts, count);
            if( ins_res.out_of_memory ) {
                trace_out_of_memory_error(state->trace);
                return;
            }
            uint32_t const_index = ins_res.index;
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_PUSH_VALUE,
                .args = { (uint32_t) const_index, 0 }
            });
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_MAKE_ARRAY,
                .args = { 0 }
            });
        } break;
        case AST_ARRAY: {
            int count = node->size;
            for(int i = 0; i < count; i++) {
                codegen(node->as.items[i], state);
            }
            vb_result_t app_res = valbuffer_insert_int(&state->consts, count);
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
            int count = node->size;
            for(int i = 0; i < count; i++) {
                codegen(node->as.items[i], state);
            }
        } break;
        case AST_ARGLIST: {
            int count = node->size;
            for(int i = 0; i < count; i++) {
                codegen(node->as.items[i], state);
            }
        } break;
        case AST_IFCHAIN: {
            codegen_if_chain(node, state);
        } break;
        case AST_FOREACH: {
            codegen_foreach(node, state);
        } break;
        case AST_FUNDEFN: {
            codegen_fundef(node, state);
            if(ast_is_exported(node))
                add_program_provided_function_definition(ast_try_get_name(node), state);
        } break;
        case AST_FUNSIGN: {
            if(ast_is_imported(node))
                add_host_provided_function_definition(ast_try_get_name(node), state);
        } break;
        case AST_FUNCALL: {
            codegen_funcall(node, state);
        } break;
        case AST_VARREF: {
            srcref_t name = ast_try_get_name(node);
            ir_index_t var_index = state_get_localvar(state, srcref_as_sstr(name));
            assert(var_index.tag == IRID_VAR && "variable not found");
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_LOAD_LOCAL,
                .args = { var_index.idx, 0 }
            });
        } break;
        case AST_VARDECL: {
            // just add valiable name to frame local var set.
            srcref_t name = ast_try_get_name(node);
            state_add_localvar(state, srcref_as_sstr(name));
        } break;
        case AST_TYDESCR: {
            /* nothing to do here */
        } break;
        case AST_SYMBOL: {
            /* nothing to do here */
        } break;
        default: {
            if(ast_is_value(node))
                codegen_value(node, state);
            else if(ast_is_binop(node))
                codegen_binop(node, state);
            else if(ast_is_unop(node))
                codegen_unop(node, state);
        } break;
    }
}

void recalc_index_to_bytecode_adress(ir_list_t* instrs, uint32_t* idx2addr) {
    for (uint32_t i = 0; i < instrs->count; i++) {
        switch(instrs->irs[i].opcode) {
            case OP_CALL:
            case OP_ITER_NEXT:
            case OP_JUMP:
            case OP_JUMP_IF_FALSE: {
                uint32_t index = instrs->irs[i].args[0];
                // assert <= max_address
                assert(idx2addr[index] <= idx2addr[instrs->count]);
                instrs->irs[i].args[0] = idx2addr[index];
            } break;
            default: break;
        }
    }
}

void create_index_to_addr_map(ir_list_t* instrs, uint32_t* idx2addr, uint32_t size) {
    uint32_t addr = 0;
    const uint32_t argbytes = 4; // 32-bit args
    (void)(size);
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

void set_entrypoints(compiler_state_t* state, uint32_t* idx2addr, uint32_t* dest) {
    for( int i = 0; i < state->program_supplied.count; i++ ) {
        ir_index_t index = state_get_funcaddr(state, state->program_supplied.def[i].name);
        assert( index.tag == IRID_INS );
        dest[i] = idx2addr[index.idx];
    }
}

program_t write_program(compiler_state_t* state, uint32_t* idx2addr) {

    recalc_index_to_bytecode_adress(&state->instrs, idx2addr);

    u8buffer_t bytecode;
    u8buffer_create(&bytecode, state->instrs.count);

    for (uint32_t i = 0; i < state->instrs.count; i++) {
        u8buffer_write(&bytecode, (uint8_t) state->instrs.irs[i].opcode);
        uint32_t argcount = get_op_arg_count(state->instrs.irs[i].opcode);
        for (uint32_t j = 0; j < argcount; j++) {
            uint32_t value = state->instrs.irs[i].args[j];
            u8buffer_write(&bytecode, (uint8_t) ((value >> (8*0)) & 0xFF));
            u8buffer_write(&bytecode, (uint8_t) ((value >> (8*1)) & 0xFF));
            u8buffer_write(&bytecode, (uint8_t) ((value >> (8*2)) & 0xFF));
            u8buffer_write(&bytecode, (uint8_t) ((value >> (8*3)) & 0xFF));
        }
    }

    val_t* const_buf = (val_t*) malloc( sizeof(val_t) * state->consts.size );
    memcpy(const_buf, state->consts.values, sizeof(val_t) * state->consts.size);

    uint8_t* code_buf = (uint8_t*) malloc( sizeof(uint8_t) * bytecode.size );
    memcpy(code_buf, bytecode.data, sizeof(uint8_t) * bytecode.size );

    uint32_t* expaddrs = (uint32_t*) malloc( sizeof(uint32_t) * state->program_supplied.count );
    set_entrypoints(state, idx2addr, expaddrs);

    program_t result = (program_t) {
        .cons.buffer = const_buf,
        .cons.count = state->consts.size,
        .inst.buffer = code_buf,
        .inst.size = bytecode.size,
        .exports = state->program_supplied,
        .expaddr = expaddrs,
        .imports = state->host_supplied
    };

    u8buffer_destroy(&bytecode);
    return result;
}


program_t gvm_compile(arena_t* arena, ast_t* node, trace_t* trace) {

    program_t program = { 0 };

    trace_clear(trace);

    compiler_state_t state = (compiler_state_t) {
        .trace = trace,
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

    if(ffi_definition_set_init(&state.host_supplied, 8) == false) {
        trace_out_of_memory_error(state.trace);
        srcmap_destroy(&state.functions);
        srcmap_destroy(&state.localvars);
        irl_destroy(&state.instrs);
        ffi_definition_set_destroy(&state.host_supplied);
        return program;
    }

    if(ffi_definition_set_init(&state.program_supplied, 8) == false) {
        trace_out_of_memory_error(state.trace);
        srcmap_destroy(&state.functions);
        srcmap_destroy(&state.localvars);
        irl_destroy(&state.instrs);
        ffi_definition_set_destroy(&state.host_supplied);
        ffi_definition_set_destroy(&state.program_supplied);
        return program;
    }

    // generate the code
    codegen(node, &state);

    // add final halt instruction
    irl_add(&state.instrs, (ir_inst_t) {
        .opcode = OP_HALT,
        .args = { 0 }
    });

    if( trace_get_error_count(state.trace) == 0 ) {
        uint32_t idx2addr_count = state.instrs.count + 1;
        uint32_t idx2addr[idx2addr_count];

        // ownership of state.host_supplied and state.program_supplied
        // is transfered to program
        create_index_to_addr_map(&state.instrs, idx2addr, idx2addr_count);
        program = write_program(&state, idx2addr);
    }
    
    valbuffer_destroy(&state.consts);
    irl_destroy(&state.instrs);
    srcmap_destroy(&state.localvars);
    srcmap_destroy(&state.functions);

    return program;
}
