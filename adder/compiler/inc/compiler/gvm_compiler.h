#ifndef GVM_COMPILER_H_
#define GVM_COMPILER_H_

#include "compiler/gvm_ast.h"
#include "shared/gvm_types.h"
#include "compiler/gvm_types.h"

gvm_program_t gvm_compile(ast_node_t* node, cres_t* status);

#endif // GVM_COMPILER_H_