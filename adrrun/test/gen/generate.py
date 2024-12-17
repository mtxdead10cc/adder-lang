import sys
from os import listdir
from os.path import isfile, join, dirname, abspath

START_TOKEN = "$START"
VERIFY_TOKEN = "$VERIFY"
TODO_TOKEN = "$TODO"
OUTFILE = "langtest.h"

def is_test_file(source_dir, name:str):
    return isfile(join(source_dir, name)) and name.endswith("txt")

def parse_testcase_end(line:str) -> str:
    if line.startswith(VERIFY_TOKEN):
        return "verify"
    elif line.startswith(TODO_TOKEN):
        return "todo"
    else:
        return None

def parse_testcase(lines:list[str]):
    while len(lines) > 0:
        if lines[0].strip().startswith(START_TOKEN):
            start_line = lines[0]
            lines = lines[1:]
            
            name = start_line \
                .replace(START_TOKEN, "")\
                .replace("(", "")\
                .replace(")", "")\
                .replace("\"","")\
                .replace("\n","")
            
            code_lines = []
            category = None
            while len(lines) > 0:
                category = parse_testcase_end(lines[0])
                if category == None:
                    code_lines.append(lines[0])
                    lines = lines[1:]
                else:
                    break

            stop_line = lines[0]
            expected = stop_line                    \
                .replace(VERIFY_TOKEN, "")          \
                .replace(TODO_TOKEN, "")            \
                .strip()
            expected = expected[1:]
            expected = expected[:-1]
            lines = lines[1:]
                
            yield (name, code_lines, expected, category)
        lines = lines[1:]

def write_file(tests:list[str], outpath:str):
    test_string = ',\n    '.join(tests)
    cont = f"""
#ifndef LANGTEST_H_
#define LANGTEST_H_

typedef struct ltc_t {{
    char* category;
    char* name;
    char* code;
    char* expect;
    char* filepath;
}} ltc_t;

ltc_t langtest_testcases[] = {{
    {test_string}
}};

#endif // LANGTEST_H_
"""
    f = open(outpath, 'w')
    f.write(cont)
    f.close()

def gen_testcase(name, code_lines, expected, category, filepath):
    code = ""
    for code_line in code_lines:
        code_line = code_line       \
            .replace("\n", "\\n")   \
            .replace("\t", "\\t")   \
            .replace("\"", "\\\"")
        code += f"        \"{code_line}\"\n"
    
    if len(expected) > 0:
        if expected[0] != "\"":
            expected = "\"" + expected
        if expected[-1] != "\"" or len(expected) == 1:
            expected = expected + "\""
    else:
        expected = "\"\""

    out =   "{\n"
    out += f"        .category = \"{category}\",\n"
    out += f"        .name = \"{name}\",\n"
    out += f"        .code = \n"
    out += f"{code[:-1]},\n"
    out += f"        .expect = {expected},\n"
    out += f"        .filepath = \"{filepath}\",\n"
    out +=  "    }"

    return out

if __name__ == "__main__":

    dir_path    = dirname(abspath(__file__))
    source_dir  = dir_path + "/testfiles"
    test_source = [join(source_dir, f) for f in listdir(source_dir) if is_test_file(source_dir, f)]
    outpath = join(dir_path, "../")
    outpath = join(outpath, "langtest.h")
    
    test_strings = []
    for tsrc in test_source:
        filename = tsrc.split("/")[-1]
        f = open(tsrc, 'r')
        for name, code, expected, category in parse_testcase(f.readlines()):
            test_strings.append(gen_testcase(name, code, expected, category, filename))
        f.close()
    
    write_file(test_strings, outpath)

    
