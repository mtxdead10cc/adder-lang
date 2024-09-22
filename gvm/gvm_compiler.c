#include "gvm_compiler.h"
#include <assert.h>
#include "gvm_ir.h"

#define GVM_COMPILER_MAP_SIZE 5

typedef struct src2idxmap_t {
    size_t          size;
    ir_index_tag_t  tag;
    srcref_t        key[GVM_COMPILER_MAP_SIZE];
    bool            is_in_use[GVM_COMPILER_MAP_SIZE];
    ir_index_t      value[GVM_COMPILER_MAP_SIZE];
} src2idxmap_t;

void s2sim_init(src2idxmap_t* map, ir_index_tag_t tag) {
    *map = (src2idxmap_t) { 0 };
    map->tag = tag;
}

size_t s2im_hash(srcref_t ref) {
    size_t len = srcref_len(ref);
    size_t hash_code = len + 5;
    char* keystr = srcref_to_str(ref); 
    for(size_t i = 0; i < len; i++) {
        hash_code += (hash_code + keystr[i]) * 7919U;
    }
    return hash_code;
}

bool s2im_insert(src2idxmap_t* map, srcref_t key, ir_index_t val) {
    assert(val.tag == map->tag && "table tag and value mismatch");
    size_t hk = s2im_hash(key);
    size_t start_index = hk % GVM_COMPILER_MAP_SIZE;
    for(size_t i = 0; i < GVM_COMPILER_MAP_SIZE; i++) {
        size_t tab_index = (i + start_index) % GVM_COMPILER_MAP_SIZE;
        if( map->is_in_use[tab_index] == false ) {
            map->value[tab_index] = val;
            map->is_in_use[tab_index] = true;
            map->key[tab_index] = key;
            map->size ++;
            return true;
        } else if ( srcref_equals(map->key[tab_index], key) ) {
            return false;
        }
    }
    return false;
}

void s2im_clear(src2idxmap_t* map) {
    ir_index_tag_t tag = map->tag;
    *map = (src2idxmap_t) { 0 }; 
    map->tag = tag;
}

void s2im_print(src2idxmap_t* map) {
    printf("[Source -> Index Map (size=%d)]\n", (uint32_t) map->size);
    for(int i = 0; i < GVM_COMPILER_MAP_SIZE; i++) {
        printf("%i > ", i);
        if( map->is_in_use[i] == false ) {
            printf("<empty>");
        } else {
            srcref_print(map->key[i]);
        }
        printf("\n");
    }
}

ir_index_t* s2im_lookup(src2idxmap_t* map, srcref_t key) {
    size_t hk = s2im_hash(key);
    size_t start_index = hk % GVM_COMPILER_MAP_SIZE;
    size_t hit_count = 0;
    for(size_t i = 0; i < GVM_COMPILER_MAP_SIZE; i++) {
        size_t tab_index = (i + start_index) % GVM_COMPILER_MAP_SIZE;
        if( map->is_in_use[tab_index] == false ) {
            continue;
        }
        if( srcref_equals(map->key[tab_index], key) ) {
            return &map->value[tab_index];
        }
        if( (hit_count ++) == map->size ) {
            return NULL;
        } 
    }
    return NULL;
}


typedef struct compiler_state_t {
    src2idxmap_t     localvars;
    src2idxmap_t     functions;
    ir_list_t        instrs;
    valbuffer_t      consts;
} compiler_state_t;

void codegen(ast_node_t* node, compiler_state_t* state);

void codegen_binop(ast_binop_t node, compiler_state_t* state) {
    codegen(node.left, state);
    codegen(node.right, state);
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
        default: {
            printf("Unhandled binary operation: %s\n",
                ast_binop_type_as_string(node.type));
        } break;
    }
}

void codegen_unop(ast_unop_t node, compiler_state_t* state) {
    codegen(node.inner, state);
    switch(node.type) {
        case AST_UN_NEG: {
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_NEG,
                .args = { 0 }
            });
        } break;
        default: {
            printf("Unhandled unary operation: %s\n",
                ast_unop_type_as_string(node.type));
        } break;
    }
}

void codegen_value(ast_value_t node, compiler_state_t* state) {
    int const_index = -1;
    switch(node.type) {
        case AST_VALUE_TYPE_BOOL: {
            const_index = au_consts_add_bool(&state->consts,
                node.u._bool);
        } break;
        case AST_VALUE_TYPE_NUMBER: {
            const_index = au_consts_add_number(&state->consts,
                node.u._number);
        } break;
        case AST_VALUE_TYPE_STRING: {
            // Note: needs to start with a '"'
            srcref_t ref = node.u._string;
            char* ptr = srcref_to_str(ref);
            assert(ptr[0] == '\"' && "current impl requres quoted strings.");
            const_index = au_consts_add_string(&state->consts, ptr);
        } break;
        default: {
            /* ignore */
            printf("error: cant push constant reference, unsupported value type %d\n",
                node.type);
        } break;
    }
    assert((const_index >= 0) && "unsupported constant value");
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_PUSH_VALUE,
        .args = { const_index, 0 }
    });
}

void codegen_fundecl(ast_fundecl_t node, compiler_state_t* state) {
    assert(node.args->type == AST_BLOCK);
    assert(state->localvars.size == 0 && "Function declared inside function?");
    ir_index_t frame_index = irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_MAKE_FRAME,
        .args = { 0 }
    });
    bool ok = s2im_insert(&state->functions, node.name, frame_index);
    assert(ok == true);
    s2im_clear(&state->localvars);
    codegen(node.args, state); // in order to "add" arg names
    uint32_t arg_count = (uint32_t) state->localvars.size;
    codegen(node.body, state); // adds locals to frame
    uint32_t locals_count = ((uint32_t) state->localvars.size) - arg_count;
    irl_get(&state->instrs, frame_index)->args[0] = arg_count;
    irl_get(&state->instrs, frame_index)->args[1] = locals_count;
    s2im_clear(&state->localvars);
}

void codegen_funcall(ast_funcall_t node, compiler_state_t* state) {
    codegen(node.args, state);
    ir_index_t* ir_index = s2im_lookup(&state->functions, node.name);
    assert(ir_index == NULL && "Function not found (not declared)");
    // if tag invalid: could not find index of function name (not defined)
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_CALL,
        .args = { ir_index->idx, 0 }
    });
}

void codegen_assignment(ast_assign_t node, compiler_state_t* state) {
    codegen(node.right_value, state);
    assert(state->localvars.size > 0 && "local vars was empty");
    srcref_t varname = { 0 };
    // depending on declared variable or just
    // a reference to already existing
    // we do slightly differen things.
    if( node.left_var->type == AST_VAR_DECL ) {
        codegen(node.left_var, state); // add var to known locals
        varname = node.left_var->u.n_vardecl.name;
    } else if ( node.left_var->type == AST_VAR_REF ) {
        varname = node.left_var->u.n_varref.name;
    } else {
        assert(false && "expected variable node as LSH in assignment");
    }
    ir_index_t* index = s2im_lookup(&state->localvars, varname);
    assert(index != NULL && "varname not found");
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_STORE_LOCAL,
        .args = { (uint32_t) index->idx, 0 }
    });
}

void codegen_foreach(ast_foreach_t node, compiler_state_t* state) {
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
    ir_index_t* varindex = s2im_lookup(&state->localvars, varname);
    assert(varindex != NULL && "variable not found");
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_STORE_LOCAL,
        .args = { varindex->idx, 0 }
    });
    codegen(node.during, state);
    irl_add(&state->instrs, (ir_inst_t){
        .opcode = OP_JUMP,
        .args = { loop_start_index.idx, 0 }
    });
    irl_get(&state->instrs, loop_start_index)->args[0] = state->instrs.count;
}

void codegen(ast_node_t* node, compiler_state_t* state) {
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
        case AST_ARRAY: {
            size_t count = node->u.n_array.count;
            for(size_t i = 0; i < count; i++) {
                codegen(node->u.n_array.content[i], state);
            }
            int const_index = au_consts_add_number(&state->consts, (float)count);
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_PUSH_VALUE,
                .args = { (uint32_t) const_index, 0 }
            });
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_MAKE_ARRAY,
                .args = { 0 }
            });
        } break;
        case AST_RETURN: {
            codegen(node->u.n_return.result, state);
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_RETURN,
                .args = { 0 }
            });
        } break;
        case AST_BLOCK: {
            size_t count = node->u.n_block.count;
            for(size_t i = 0; i < count; i++) {
                codegen(node->u.n_block.content[i], state);
            }
        } break;
        case AST_IF: {
            codegen(node->u.n_if.cond, state);
            ir_index_t if_index = irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_JUMP_IF_FALSE,
                .args = { 0 }
            });
            codegen(node->u.n_if.iftrue, state);
            irl_get(&state->instrs, if_index)->args[0] = state->instrs.count;
        } break;
        case AST_IF_ELSE: {
            codegen(node->u.n_ifelse.cond, state);
            ir_index_t if_false_index = irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_JUMP_IF_FALSE,
                .args = { 0 }
            });
            codegen(node->u.n_ifelse.iftrue, state);
            ir_index_t jump_end_index = irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_JUMP,
                .args = { 0 }
            });
            // set jump loc for if false
            irl_get(&state->instrs, if_false_index)->args[0] = state->instrs.count;
            codegen(node->u.n_ifelse.iffalse, state);
            irl_get(&state->instrs, jump_end_index)->args[0] = state->instrs.count;
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
            ir_index_t* var_index = s2im_lookup(&state->localvars, node->u.n_varref.name);
            assert(var_index != NULL && "variable not found");
            irl_add(&state->instrs, (ir_inst_t){
                .opcode = OP_LOAD_LOCAL,
                .args = { var_index->idx, 0 }
            });
        } break;
        case AST_VALUE: {
            codegen_value(node->u.n_value, state);
        } break;
        case AST_VAR_DECL: {
            // just add valiable name to frame local var set.
            s2im_insert(&state->localvars, node->u.n_vardecl.name, (ir_index_t){
                .tag = IRID_VAR,
                .idx = (uint32_t) state->localvars.size
            });
        } break;
        case AST_BREAK: {
            assert(false && "break op is not implemented yet");
        } break;
    }
}


gvm_program_t gvm_compile(ast_node_t* node) {

    compiler_state_t state = (compiler_state_t) { 0 };
    s2sim_init(&state.functions, IRID_INS);
    s2sim_init(&state.localvars, IRID_VAR);
    irl_init(&state.instrs, 16);
    valbuffer_create(&state.consts, 16);
    codegen(node, &state);

    irl_dump(&state.instrs);
    
    // destroy valbuffer

    return (gvm_program_t) {0};
}