#ifndef GVM_COMPILER_H_
#define GVM_COMPILER_H_

#include "co_ast.h"
#include "co_types.h"
#include "sh_types.h"

gvm_program_t gvm_compile(ast_node_t* node, trace_t* trace);

#endif // GVM_COMPILER_H_