import sys
from os import listdir
from os.path import isfile, join, dirname, abspath

START_TOKEN = "$START"
STOP_TOKEN = "$STOP"
OUTFILE = "langtest.h"

def is_test_file(source_dir, name:str):
    return isfile(join(source_dir, name)) and name.endswith("txt")

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
            while len(lines) > 0 and lines[0].startswith(STOP_TOKEN) == False:
                code_lines.append(lines[0])
                lines = lines[1:]

            stop_line = lines[0]
            expected = stop_line.replace(STOP_TOKEN, "").strip()
            expected = expected[1:]
            expected = expected[:-1]
            lines = lines[1:]

            yield (name, code_lines, expected)
        lines = lines[1:]

def write_file(tests:list[str], outpath:str):
    test_string = ',\n    '.join(tests)
    cont = f"""
#ifndef LANGTEST_H_
#define LANGTEST_H_

typedef struct ltc_t {{
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

def gen_testcase(name, code_lines, expected, filepath):
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
    out += f"        .name = \"{name}\",\n"
    out += f"        .code = \n"
    out += f"{code[:-1]},\n"
    out += f"        .expect = {expected},\n"
    out += f"        .filepath = \"{filepath}\"\n"
    out +=  "    }"

    return out

if __name__ == "__main__":

    dir_path    = dirname(abspath(__file__))
    source_dir  = dir_path + "/source"
    test_source = [join(source_dir, f) for f in listdir(source_dir) if is_test_file(source_dir, f)]
    outpath = join(dir_path, "../")
    outpath = join(outpath, "langtest.h")
    
    test_strings = []
    for tsrc in test_source:
        filename = tsrc.split("/")[-1]
        f = open(tsrc, 'r')
        for n,c,e in parse_testcase(f.readlines()):
            test_strings.append(gen_testcase(n, c, e, filename))
        f.close()
    
    write_file(test_strings, outpath)

    
