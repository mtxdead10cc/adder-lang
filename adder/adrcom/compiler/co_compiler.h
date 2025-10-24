#ifndef GVM_COMPILER_H_
#define GVM_COMPILER_H_

#include <adrcom/shared/co_ast.h>
#include <adrcom/shared/co_types.h>

#include <shared/sh_types.h>
#include <shared/sh_arena.h>

program_t gvm_compile(arena_t* arena, ast_node_t* node, trace_t* trace);

#endif // GVM_COMPILER_H_
