#ifndef GVM_CODEGEN_H_
#define GVM_CODEGEN_H_

#include "gvm_ast.h"
#include "gvm_asmutils.h"

void gvm_codegen(ast_node_t* node) {
    switch(node->type) {
        case AST_BINOP: {
            gvm_codegen(node->u.n_binop.left);
            gvm_codegen(node->u.n_binop.right);
            switch(node->u.n_binop.type) {
                case AST_BIN_ADD: {
                    printf("add");
                } break;
                case AST_BIN_SUB: {
                    printf("sub");
                } break;
                case AST_BIN_MUL: {
                    printf("mul");
                } break;
                case AST_BIN_AND: {
                    printf("and");
                } break;
                case AST_BIN_OR: {
                    printf("or");
                } break;
                case AST_BIN_EQ: {
                    printf("is-equal");
                } break;
                case AST_BIN_LT: {
                    printf("is-less");
                } break;
                case AST_BIN_GT: {
                    printf("is-more");
                } break;
                default: {
                    printf("<unk>");
                } break;
            }
            printf("\n");
        } break;
        case AST_UNOP: {
            gvm_codegen(node->u.n_unop.inner);
            switch(node->u.n_unop.type) {
                case AST_UN_NEG: {
                    printf("neg");
                } break;
                default: {
                    printf("<unk>");
                } break;
            }
            printf("\n");
        } break;
        case AST_ASSIGN: {
            gvm_codegen(node->u.n_assign.right_value);
            // NOTE: n_varref and n_vardecl works here now
            // because name is placed after type in both cases
            // but it would probably be safer to handle both cases
            // separately.
            printf("store ");
            srcref_print(node->u.n_assign.left_var->u.n_varref.name);
            printf("\n");
        } break;
        case AST_ARRAY: {
            size_t count = node->u.n_array.count;
            for(size_t i = 0; i < count; i++) {
                gvm_codegen(node->u.n_array.content[i]);
            }
            printf("push %d\n", (int) count);
            printf("array\n");
        } break;
        case AST_RETURN: {
            gvm_codegen(node->u.n_return.result);
            printf("return\n");
        } break;
        case AST_BLOCK: {
            size_t count = node->u.n_block.count;
            for(size_t i = 0; i < count; i++) {
                gvm_codegen(node->u.n_block.content[i]);
            }
        } break;
        case AST_IF: {
            gvm_codegen(node->u.n_if.cond);
            printf("if-false [todolabel]\n");
            gvm_codegen(node->u.n_if.iftrue);
            printf("[todolabel]:\n");
        } break;
        case AST_IF_ELSE: {
            printf("# IF ELSE - BEGIN \n");
            gvm_codegen(node->u.n_ifelse.cond);
            printf("if-false [todolabel:else]\n");
            gvm_codegen(node->u.n_ifelse.iftrue);
            printf("jump [todolabel:end]\n");
            printf("[todolabel:else]:\n");
            gvm_codegen(node->u.n_ifelse.iffalse);
            printf("[todolabel:end]:\n");
            printf("# IF ELSE - END \n");
        } break;
        case AST_FOREACH: {
            printf("# FOR EACH START\n");
            gvm_codegen(node->u.n_foreach.collection);
            printf("iter\n");
            printf("[todolabel:loopstart]:\n");
            printf("iter-next [todolabel:exit]\n");
            printf("store ");
            srcref_print(node->u.n_foreach.vardecl->u.n_vardecl.name);
            printf("\n");
            gvm_codegen(node->u.n_foreach.during);
            printf("jump [todolabel:loopstart]\n");
            printf("[todolabel:exit]:\n");
            printf("# FOR EACH END\n");
        } break;
        case AST_FUN_DECL: {
            printf("# FUNCTION - START\n");
            srcref_print(node->u.n_fundecl.name);
            printf(":\n");
            assert(node->u.n_fundecl.args->type == AST_BLOCK);
            uint32_t argcount = node->u.n_fundecl.args->u.n_block.count;
            printf("frame %d\n", argcount);
            printf("# ARGS\n");
            gvm_codegen(node->u.n_fundecl.args);
            printf("# BODY - START\n");
            gvm_codegen(node->u.n_fundecl.body);
            printf("# BODY - END\n");
            printf("# FUNCTION - END\n");
        } break;
        case AST_FUN_CALL: {
            gvm_codegen(node->u.n_funcall.args);
            printf("call ");
            srcref_print(node->u.n_funcall.name);
            printf("\n");
        } break;
        case AST_VAR_REF: {
            printf("load ");
            srcref_print(node->u.n_varref.name);
            printf("\n");
        } break;
        case AST_VALUE: {
            printf("push ");
            ast_dump_value(node->u.n_value);
            printf("\n");
        } break;
        case AST_VAR_DECL: {
            printf("# [");
            srcref_print(node->u.n_vardecl.name);
            printf(":%s ", ast_value_type_as_string(node->u.n_vardecl.type));
            printf("]\n");
        } break;
        case AST_BREAK: {
            printf("break\t#break-loop");
        } break;
    }
}

#endif // GVM_CODEGEN_H_