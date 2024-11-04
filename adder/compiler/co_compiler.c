#include "sh_asminfo.h"
#include "co_utils.h"
#include "co_compiler.h"
#include "co_srcmap.h"
#include "co_trace.h"
#include <assert.h>

typedef struct ir_inst_t {
    gvm_op_t opcode;
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
        case AST_ARRAY:     return args->u.n_array.count;
        case AST_BLOCK:     return args->u.n_block.count;
        case AST_FUN_CALL:
        case AST_UNOP:
        case AST_BINOP:
        case AST_VALUE:     return 1;
        default:            return 0;
    }
}

typedef struct funsign_set_t {
    size_t              capacity;
    size_t              count;
    native_funsign_t*   entries;
} funsign_set_t;

bool funsign_set_init(funsign_set_t* decls, size_t capacity) {
    decls->entries = (native_funsign_t*) malloc( capacity * sizeof(native_funsign_t) );
    if( decls->entries == NULL )
        return false;
    decls->count = 0;
    decls->capacity = capacity;
    return true;
}

native_funsign_t mk_native_funsign(srcref_t name, ast_node_t* args, srcref_t type) {
    return (native_funsign_t) {
        .arg_count = get_node_content_length(args),
        .name = srcref_as_sstr(name),
        .return_type = srcref_as_sstr(type)
    };
}

bool funsign_equal(native_funsign_t* a, native_funsign_t* b) {
    if( a->arg_count != b->arg_count )
        return false;
    if( sstr_equal(&a->name, &b->name) == false )
        return false;
    //return sstr_equal(&a->returntype, &b->returntype); // todo: when types are fixed
    return true;
}

int funsign_set_find_index(funsign_set_t* decls, native_funsign_t* extdecl) {
    for(size_t i = 0; i < decls->count; i++) {
        if( funsign_equal(&decls->entries[i], extdecl) )
            return (int) i;
    }
    return -1;
}

bool funsign_set_add(funsign_set_t* decls, native_funsign_t extdecl) {
    if( funsign_set_find_index(decls, &extdecl) >= 0 )
        return false;
    if( decls->count == decls->capacity ) {
        size_t new_capacity = decls->capacity * 2;
        native_funsign_t* entries = (native_funsign_t*) realloc(decls->entries, new_capacity * sizeof(native_funsign_t));
        if( entries == NULL )
            return false;
        decls->entries = entries;
        decls->capacity = new_capacity;
    }
    decls->entries[decls->count++] = extdecl;
    return true;
}

void funsign_set_destroy(funsign_set_t* decls) {
    if( decls->entries != NULL )
        free(decls->entries);
    decls->entries = NULL;
    decls->count = 0;
    decls->capacity = 0;
}

typedef struct compiler_state_t {
    srcmap_t                localvars;
    srcmap_t                functions;
    funsign_set_t           extdecls;
    ir_list_t               instrs;
    valbuffer_t             consts;
    trace_t*                trace;
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

ast_decl_type_t funsign_get_decltype(ast_node_t* node) {
    return node->u.n_funsign.decltype;
}

srcref_t funsign_get_name(ast_node_t* node) {
    return node->u.n_funsign.name; 
}

ast_node_t* funsign_get_argspec(ast_node_t* node) {
    return node->u.n_funsign.argspec; 
}

void codegen_fundecl(ast_fundecl_t node, compiler_state_t* state) {

    ABORT_ON_ERROR(state);

    srcref_t funcname = funsign_get_name(node.funsign);

    assert( funsign_get_decltype(node.funsign) == AST_FUNSIGN_INTERN );

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

    assert(ok == true);
    
    srcmap_clear(&state->localvars);

    codegen(funsign_get_argspec(node.funsign), state); // in order to "add" arg names

    uint32_t arg_count = (uint32_t) state->localvars.count;
    codegen(node.body, state); // adds locals to frame
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

    native_funsign_t edecl = mk_native_funsign(node.name, node.args, srcref_const("type"));
    int ext_index = funsign_set_find_index(&state->extdecls, &edecl);
    if ( ext_index >= 0 ) {
        irl_add(&state->instrs, (ir_inst_t){
            .opcode = OP_CALL_NATIVE,
            .args = { ext_index, 0 }
        });
        return;
    }

    trace_msg_t* msg = trace_create_message(state->trace, TM_ERROR, node.name);
    trace_msg_append_costr(msg, "function '");
    trace_msg_append_srcref(msg, node.name);
    trace_msg_append_costr(msg, "' is not declared.");
    
}

void codegen_assignment(ast_assign_t node, compiler_state_t* state) {

    ABORT_ON_ERROR(state);

    codegen(node.right_value, state);
    srcref_t varname = { 0 };
    // depending on declared variable or just
    // a reference to already existing
    // we do slightly differen things.
    if( node.left_var->type == AST_VAR_DECL ) {
        codegen(node.left_var, state); // add var to known locals
        assert(state->localvars.count > 0 && "local vars was empty");
        varname = node.left_var->u.n_vardecl.name;
    } else if ( node.left_var->type == AST_VAR_REF ) {
        assert(state->localvars.count > 0 && "local vars was empty");
        varname = node.left_var->u.n_varref.name;
    } else {
        assert(false && "expected variable node as LSH in assignment");
    }
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
    srcref_t varname = node.vardecl->u.n_vardecl.name;
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

bool is_valid_else_block(ast_node_t* node) {
    if( node == NULL )
        return false;
    return node->type == AST_BLOCK
        && node->u.n_block.count > 0;
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
        if( current->type == AST_IF_CHAIN || is_valid_else_block(current) ) {
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
    if( is_valid_else_block(current) )
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
        case AST_VAR_DECL:
        case AST_FUN_DECL:
        case AST_FUN_SIGN:
        case AST_RETURN:
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
        case AST_VAR_DECL: {
            // just add valiable name to frame local var set.
            state_add_localvar(state, node->u.n_vardecl.name);
        } break;
        case AST_BREAK: {
            assert(false && "break op is not implemented yet");
        } break;
        case AST_FUN_SIGN: {
            assert(node->u.n_funsign.decltype == AST_FUNSIGN_EXTERN && "can't process this function signature");
            funsign_set_add(&state->extdecls,
                mk_native_funsign(node->u.n_funsign.name,
                           node->u.n_funsign.argspec,
                           srcref_const("type")));
        } break;
    }
}

void recalc_index_to_bytecode_adress(ir_list_t* instrs) {
    
    uint32_t idx2addr[instrs->count];
    uint32_t addr = 0;
    const uint32_t argbytes = 4; // 32-bit args
    
    for (uint32_t i = 0; i < instrs->count; i++) {
        idx2addr[i] = addr;
        uint32_t argcount = get_op_arg_count(instrs->irs[i].opcode);
        addr = addr + (argcount * argbytes) + 1;
    }

    for (uint32_t i = 0; i < instrs->count; i++) {
        switch(instrs->irs[i].opcode) {
            case OP_CALL:
            case OP_ENTRY_POINT:
            case OP_ITER_NEXT:
            case OP_JUMP:
            case OP_JUMP_IF_FALSE: {
                uint32_t index = instrs->irs[i].args[0];
                assert(idx2addr[index] <= addr);
                instrs->irs[i].args[0] = idx2addr[index];
            } break;
            default: break;
        }
    }
}

gvm_program_t write_program(ir_list_t* instrs, valbuffer_t* consts, funsign_set_t* imports) {

    recalc_index_to_bytecode_adress(instrs);

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

    native_funsign_t* decls = (native_funsign_t*) malloc( sizeof(native_funsign_t) * imports->count ); 
    memcpy(decls, imports->entries, sizeof(native_funsign_t) * imports->count );

    gvm_program_t result = (gvm_program_t) {
        .cons.buffer = const_buf,
        .cons.count = consts->size,
        .inst.buffer = code_buf,
        .inst.size = bytecode.size,
        .required.signatures = decls,
        .required.count = imports->count
    };

    u8buffer_destroy(&bytecode);
    return result;
}


gvm_program_t gvm_compile(ast_node_t* node, trace_t* trace) {

    gvm_program_t program = { 0 };

    compiler_state_t state = (compiler_state_t) {
        .trace = trace
    };

    if( srcmap_init(&state.functions, 16) == false ) {
        trace_out_of_memory_error(state.trace);
        return program;
    }

    if( funsign_set_init(&state.extdecls, 16) == false ) {
        trace_out_of_memory_error(state.trace);
        return program;
    }
    
    if( srcmap_init(&state.localvars, 16) == false ) {
        trace_out_of_memory_error(state.trace);
        srcmap_destroy(&state.functions);
        funsign_set_destroy(&state.extdecls);
        return program;
    }

    if( irl_init(&state.instrs, 16) == false ) {
        trace_out_of_memory_error(state.trace);
        srcmap_destroy(&state.functions);
        funsign_set_destroy(&state.extdecls);
        srcmap_destroy(&state.localvars);
        return program;
    }

    if( valbuffer_create(&state.consts, 16) == false ) {
        trace_out_of_memory_error(state.trace);
        srcmap_destroy(&state.functions);
        funsign_set_destroy(&state.extdecls);
        srcmap_destroy(&state.localvars);
        irl_destroy(&state.instrs);
        return program;
    }

    ir_index_t entrypoint = irl_add(&state.instrs, (ir_inst_t) {
        .opcode = OP_ENTRY_POINT,
        .args = { 0 }
    });

    codegen(node, &state);

    if( trace_get_error_count(state.trace) == 0 ) {
        ir_index_t index = state_get_funcaddr(&state, srcref_const("main"));
        if(index.tag == IRID_INS) {
            //assert(index.tag == IRID_INS && "main entrypoint not found");
            irl_get(&state.instrs, entrypoint)->args[0] = index.idx;
            // irl_dump(&state.instrs);
            program = write_program(&state.instrs, &state.consts, &state.extdecls);
        } else {
            trace_msg_t* msg = trace_create_message(state.trace, TM_ERROR, trace_no_ref());
            trace_msg_append_costr(msg, "no main() function found in program");
        }
    }
    
    valbuffer_destroy(&state.consts);
    irl_destroy(&state.instrs);
    srcmap_destroy(&state.localvars);
    srcmap_destroy(&state.functions);
    funsign_set_destroy(&state.extdecls);

    return program;
}