#ifndef GVM_COMPILER_H_
#define GVM_COMPILER_H_

#include "gvm_ast.h"
#include "gvm_types.h"

gvm_program_t gvm_compile(ast_node_t* node);

#endif // GVM_COMPILER_H_