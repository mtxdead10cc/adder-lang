#ifndef GVM_COMPILER_H_
#define GVM_COMPILER_H_

#include "gvm_ast.h"
#include "gvm_types.h"
#include "gvm_ctypes.h"

gvm_program_t gvm_compile(ast_node_t* node, cres_t* status);

#endif // GVM_COMPILER_H_