import itertools
import sys
from os import listdir
from os.path import isfile, join, dirname, abspath

arg_type_list = [
    "bool",
    "int",
    "float",
    "char",
    "char*"
]

return_type_list = [
    "bool",
    "int",
    "float",
    "char",
    "char*",
    "void"
]

def gen_native_to_val_call(ctype:str, argname:str):
    if ctype == "char*":
        return f"xu_string_to_val(vm, ({argname}))"
    if ctype == "int" or ctype == "float":
        return f"val_number(({argname}))"
    return f"val_{ctype}(({argname}))"

def gen_val_to_native_call(return_ctype:str, argname:str):
    if return_ctype == "char*":
        return f"xu_val_to_string(vm, {argname})"
    if return_ctype == "int" or return_ctype == "float":
        return f"val_into_number({argname})"
    return f"val_into_{return_ctype}({argname})"

def type_get_id(ctype:str):
    if ctype == 'char*':
        return 's'
    return ctype[0]

class Arg:
    
    def __init__(self, ctype:str, name:str):
        self.ctype = ctype
        self.name = name
        
    def get_ctype(self) -> str:
        return self.ctype
    
    def get_name(self) -> str:
        return self.name
    
    def get_type_id(self) -> str:
        return type_get_id(self.ctype)
    
    def get_ccode_to_native(self, return_type:str) -> str:
        return gen_val_to_native_call(return_type, self.name)
    
    def get_ccode_to_val(self):
        return gen_native_to_val_call(self.ctype, self.name)
    
class InvokeFun:
    
    def __init__(self, return_type:str, arg_count:int):
        self.return_type = return_type
        self.arg_count = arg_count
        
    def get_ctype(self):
        return self.return_type
    
    def get_arg_count(self):
        return self.arg_count
    
    def get_args(self):
        for i in range(0, self.arg_count):
            yield Arg("val_t", f"arg{i}")
            
    def get_name(self) -> str:
        return f"{type_get_id(self.return_type)}call{self.arg_count}"
    

def gen_func_args(types:list[str], count:int):
    if count == 0:
        return
    for prod in itertools.product(types, repeat=count):
        args = []
        for i in range(count):
            args.append(Arg(prod[i], f"arg{i}"))
        yield args
        
def gen_interface_define(return_type:str, args:list[Arg]):
    return_type_id = type_get_id(return_type)
    arg_count = len(args)
    lhs_name = f"{return_type_id}call"
    rhs_name = f"{return_type_id}call{arg_count}"
    argstr_lhs = "vm, caller"
    argstr_rhs = "(vm), (caller)"
    if arg_count > 0:
        argstr_lhs += ", " + ", ".join([ f"{a.get_name()}" for a in args ])
        argstr_rhs += ", " + ", ".join([ f"{a.get_ccode_to_val()}" for a in args ])
        lhs_name += "".join([ a.get_type_id() for a in args ])        
    return f"#define {lhs_name}({argstr_lhs}) {rhs_name}({argstr_rhs})"
        
def gen_interface_defines(return_types:list[str], argtypes:list[str], maxargs:int):
    for acnt in range(0, maxargs+1):
        for return_type in return_types:
            if acnt == 0:
                yield gen_interface_define(return_type, [])
            else:
                for args in gen_func_args(argtypes, acnt):
                    yield gen_interface_define(return_type, args)
                
def gen_c_declaraction(fun:InvokeFun):
    if fun.get_arg_count() == 0:
        return f"{fun.get_ctype()} {fun.get_name()}(vm_t* vm, xu_caller_t* caller)"
    else:
        argstr = ", ".join([ f"{a.get_ctype()} {a.get_name()}" for a in fun.get_args() ])
        return f"{fun.get_ctype()} {fun.get_name()}(vm_t* vm, xu_caller_t* caller, {argstr})"
                
def gen_c_declarations(return_types:list[str], maxargs:int):
    for acnt in range(0, maxargs+1):
        for return_type in return_types:
            yield gen_c_declaraction(InvokeFun(return_type, acnt))
            
def gen_c_definition(fun:InvokeFun):
    code =   gen_c_declaraction(fun)
    code +=  " {\n"
    code += f"    assert(caller->entrypoint.argcount == {fun.get_arg_count()});\n"
    
    code +=  "\n"
    
    code += f"    xu_classlist_t* classes = caller->class.classlist;\n"
    code += f"    int ref = caller->class.classref;\n"
    code += f"    vm_env_t* env = &classes->envs[ref];\n"
    code += f"    program_t* program = &classes->programs[ref];\n"
    
    code +=  "\n"
    
    index = 0
    for a in fun.get_args():
        code += f"    program_entry_point_set_arg(&caller->entrypoint, {index}, {a.get_name()});\n"
        index += 1
    
    if index > 0:
        code +=  "\n"
    
    if fun.get_ctype() == "void":
        code += f"    vm_execute(vm, env, &caller->entrypoint, program);\n"
    else:
        conv_code = gen_val_to_native_call(fun.get_ctype(), "result")
        code += f"    val_t result = vm_execute(vm, env, &caller->entrypoint, program);\n"
        code += f"    return {conv_code};\n"
        
    code +=  "}\n"
    return code

def gen_c_definitions(return_types:list[str], maxargs:int):
    for acnt in range(0, maxargs+1):
        for return_type in return_types:
            yield gen_c_definition(InvokeFun(return_type, acnt))

def gen_header(file_name:str, return_types:list[str], arg_types:list[str], max_arg_count:int):
    symbol = "_" + file_name.replace(".", "_").upper() + "_"
    header_str = f"""
    // GENERATED FILE

    #ifndef {symbol}
    #define {symbol}

    #include <stdbool.h>
    #include <sh_value.h>

    typedef struct vm_t vm_t;
    typedef struct xu_caller_t xu_caller_t;
    
    """.replace("    ", "")
    for declstr in gen_c_declarations(return_types, max_arg_count):
        header_str += declstr + ";\n"
    header_str += "\n\n"
    for defstr in gen_interface_defines(return_types, arg_types, max_arg_count):
        header_str += defstr + "\n"
    header_str += "\n\n"
    header_str += f"#endif // {symbol}\n"
    return header_str

def gen_source(include_name:str, return_types:list[str], arg_types:list[str], max_arg_count:int):
    source_str = f"""
    // GENERATED FILE
    
    #include "{include_name}"
    #include "xu_lib.h"
    #include <stdbool.h>
    #include <sh_value.h>

    """.replace("    ", "")
    for defstr in gen_c_definitions(return_types, max_arg_count):
        source_str += defstr + "\n"
    source_str += "\n\n"
    return source_str

if __name__ == "__main__":
    
    name = "xu_invoke"
    
    print(f"Generating xutils helper ({name})")
    
    # gen_header with 8 args generates a ~1 Gb header, you have been warned
    header_string = gen_header(name + ".h", return_type_list, arg_type_list, 6) 
    source_string = gen_source(name + ".h", return_type_list, arg_type_list, 16)
    
    out_dir = dirname(abspath(__file__))
    
    # write header
    f = open(join(out_dir, name + ".h"), 'w')
    f.write(header_string)
    f.close()
    
    # write source
    f = open(join(out_dir, name + ".c"), 'w')
    f.write(source_string)
    f.close()
    
