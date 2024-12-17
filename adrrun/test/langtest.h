
#ifndef LANGTEST_H_
#define LANGTEST_H_

typedef struct ltc_t {
    char* category;
    char* name;
    char* code;
    char* expect;
    char* filepath;
} ltc_t;

ltc_t langtest_testcases[] = {
    {
        .category = "verify",
        .name = "1+1",
        .code = 
        "int main() {\n"
        "    return 1 + 1;\n"
        "}\n",
        .expect = "2",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "hellostr",
        .code = 
        "string main() {\n"
        "    int a = 1;\n"
        "    if( a <= 2 ) {\n"
        "        return \"hello123\";\n"
        "    }\n"
        "    return \"%HELLO\";\n"
        "}\n",
        .expect = "hello123",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "and-or",
        .code = 
        "bool main() {\n"
        "    int a = 1;\n"
        "    return (a >= 6) and (6 <= a) or true;\n"
        "}\n",
        .expect = "false",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "funcall",
        .code = 
        "int add(int a, int b) {\n"
        "    return a + b;\n"
        "}\n"
        "int main() {\n"
        "    int g = 3;\n"
        "    return add(g, 4);\n"
        "}\n",
        .expect = "7",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "foreach",
        .code = 
        "int valfn(int v) {\n"
        "    return v;\n"
        "}\n"
        "int loop(int a) {\n"
        "    array<int> array = [valfn(0), 1, 10]; \n"
        "    for(int i in array) {\n"
        "        a = a + i;\n"
        "    }\n"
        "    return a;\n"
        "}\n"
        "int main() {\n"
        "    int a = 0;\n"
        "    return loop(a);\n"
        "}\n",
        .expect = "11",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "ifelse",
        .code = 
        "int loop() {\n"
        "    int a = 0;\n"
        "    for(int i in [1, 2, 3, 4]) {\n"
        "        if( i >= 4 ) {\n"
        "            return i + a;\n"
        "        } else if ( i == 1 ) {\n"
        "            a = a + 2;\n"
        "        } else if ( i == 2 ) {\n"
        "            a = a - 1;\n"
        "        } else {\n"
        "            a = a - 1;\n"
        "        }\n"
        "    }\n"
        "    return 0;\n"
        "}\n"
        "int main() {\n"
        "    return loop();\n"
        "}\n",
        .expect = "4",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "negate",
        .code = 
        "float main() {\n"
        "    float a = 100.4;\n"
        "    return -a;\n"
        "}\n",
        .expect = "-100.4",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "divide",
        .code = 
        "int main() {\n"
        "    int a = 1;\n"
        "    return a / 2;\n"
        "}\n",
        .expect = "0.5",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "modulus",
        .code = 
        "int main() {\n"
        "    int a = 1;\n"
        "    return a % 2;\n"
        "}\n",
        .expect = "1",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "not",
        .code = 
        "bool main() {\n"
        "    int a = 1;\n"
        "    return not (a == 2);\n"
        "}\n",
        .expect = "true",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "native-import",
        .code = 
        "import void print(string arg);\n"
        "void main() {\n"
        "    print(\"hello\n\");\n"
        "    return;\n"
        "}\n",
        .expect = "<none>",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "operator-preceedence",
        .code = 
        "bool main() {\n"
        "    if( (2 * 3 + 1) != 7 ) {\n"
        "        return false;\n"
        "    }\n"
        "    if( (2 / 2 - 1) != 0 ) {\n"
        "        return false;\n"
        "    }\n"
        "    if( (-2 * -2 - 1) != 3 ) {\n"
        "        return false;\n"
        "    }\n"
        "    return true;\n"
        "}\n",
        .expect = "true",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "double-negative-parens",
        .code = 
        "int main() {\n"
        "    return 100 - -100 - 200;\n"
        "}\n",
        .expect = "0",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "implicit return",
        .code = 
        "import void print(string arg);\n"
        "void test() {\n"
        "    print(\"T\n\");\n"
        "}\n"
        "int main() {\n"
        "    test();\n"
        "    test();\n"
        "    test();\n"
        "    return 0;\n"
        "}\n",
        .expect = "0",
        .filepath = "basics.txt",
    },
    {
        .category = "verify",
        .name = "export-func-test",
        .code = 
        "export int test_int_1_int(int v) {\n"
        "    return v + 1;\n"
        "}\n"
        "int main() {\n"
        "    return 10000;\n"
        "}\n",
        .expect = "EXPORT(test_int_1_int;2",
        .filepath = "basics.txt",
    }
};

#endif // LANGTEST_H_
