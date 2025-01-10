import itertools, sys

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

def gen_native_to_val_call(argtype:str, argname:str):
    if argtype == "char*":
        return f"xu_string_to_val(vm, {argname})"
    if argtype == "int" or argtype == "float":
        return f"val_number({argname})"
    return f"val_{argtype}({argname})"

def gen_val_to_native_call(rettype:str, argname:str):
    if rettype == "char*":
        return f"xu_val_to_string(vm, {argname})"
    if rettype == "int" or rettype == "float":
        return f"val_number({argname})"
    return f"val_{rettype}({argname})"

def type_get_id(name:str):
    if name.startswith("array<"):
        return name[6]
    if name == 'char*':
        return 's'
    return name[0]

def gen_func_args(types:list[str], count:int):
    if count == 0:
        return
    for prod in itertools.product(types, repeat=count):
        args = []
        for i in range(count):
            s = prod[i]
            args.append({
                "name": f"arg{i}",
                "type": f"{s}",
                "id": f"{type_get_id(s)}"
            })
        yield args
        
def gen_func_decldata(rettypes:str, argtypes:list[str], maxargs:int):
    for rettype in rettypes:
        suffix_0 = type_get_id(rettype)
        for acnt in range(0, maxargs+1):
            argdatalist = list(gen_func_args(argtypes, acnt))
            if len(argdatalist) > 0:
                for argdata in gen_func_args(argtypes, acnt):
                    suffix_1 = "".join([a['id'] for a in argdata])
                    if len(suffix_1) > 0:
                        suffix_1 = f"_{suffix_1}"
                    arglist = ", ".join([ f"{a['type']} {a['name']}" for a in argdata])
                    yield {
                        "decl": f"{rettype} xu_call{suffix_0}{suffix_1}({arglist})",
                        "argdata": argdata,
                        "rettype": rettype
                    }
            else:
                yield {
                    "decl": f"{rettype} xu_call{suffix_0}()",
                    "argdata": [],
                    "rettype": rettype
                }
                
def gen_func_body(decldata:dict):
    nargs = len(decldata['argdata'])
    to_native_name = gen_val_to_native_call(decldata['rettype'], "result")
    s = decldata['decl']
    s += " {\n"
    s += f"    assert(c->entrypoint.argcount == {nargs});\n"
    s +=  "\n"
    s +=  "    xu_classlist_t* classes = c->class.classlist;\n"
    s +=  "    int ref = c->class.classref;\n"
    s +=  "    vm_env_t* env = &classes->envs[ref];\n"
    s +=  "    program_t* program = &classes->programs[ref];\n"
    s +=  "\n"
    for i in range(0, nargs):
        arg = decldata['argdata'][i]
        to_val_name = gen_native_to_val_call(arg['type'], arg['name'])
        s +=  f"    program_entry_point_set_arg(&c->entrypoint, {i}, {to_val_name});\n"
    s +=  "\n"
    if decldata['rettype'] == "void":
        s +=  "    vm_execute(vm, env, &c->entrypoint, program);\n"
    else:
        s +=  "    val_t result = vm_execute(vm, env, &c->entrypoint, program);\n"
        s += f"    return {to_native_name};\n"
    s +=  "}\n"
    s +=  "\n"
    return s

def gen_header_file(file_name:str, decldatagen):
    name = file_name.replace(".", "_").upper()
    defsym = f"_{name}_"
    s =   "// GENERATED FILE\n"
    s +=  "\n"
    s += f"#ifndef {defsym}\n"
    s += f"#define {defsym}\n"
    s +=  "\n"
    
    for decldata in decldatagen:
        s += f"{decldata['decl']};\n"
    
    s +=  "\n"
    s += f"#endif // {defsym}\n"
    return s

def gen_source_file(include_name:str, decldatagen):
    s =   "// GENERATED FILE\n"
    s += f"#include \"{include_name}\"\n"
    s +=  "#inclide \"xu_lib.h\"\n"
    s +=  "\n"
    for decldata in decldatagen:
        s += gen_func_body(decldata)
    return s

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("no target dir provided")
        exit(1)
    
    name = "xu_call"
    print(f"Generating xutils helper ({name})")
    
    decldatagen = gen_func_decldata(return_type_list, arg_type_list, 2)
    header_string = gen_header_file(name + ".h", decldatagen)
    source_string = gen_source_file(name + ".h", decldatagen)
    
    
    
    
