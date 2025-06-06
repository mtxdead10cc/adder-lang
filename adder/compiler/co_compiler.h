#ifndef GVM_COMPILER_H_
#define GVM_COMPILER_H_

#include "co_ast.h"
#include "co_types.h"
#include "sh_types.h"
#include "sh_arena.h"

program_t gvm_compile(arena_t* arena, ast_node_t* node, trace_t* trace);

#endif // GVM_COMPILER_H_
