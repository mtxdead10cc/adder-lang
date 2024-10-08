#ifndef GVM_COMPILER_H_
#define GVM_COMPILER_H_

#include "co_ast.h"
#include "co_types.h"
#include "sh_types.h"

gvm_program_t gvm_compile(ast_node_t* node, cres_t* status);

#endif // GVM_COMPILER_H_