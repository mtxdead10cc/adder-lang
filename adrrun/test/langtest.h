
#ifndef LANGTEST_H_
#define LANGTEST_H_

typedef struct ltc_t {
    char* name;
    char* code;
    char* expect;
    char* filepath;
} ltc_t;

ltc_t langtest_testcases[] = {
    {
        .name = "1+1",
        .code = 
        "num main() {\n"
        "    return 1 + 1;\n"
        "}\n",
        .expect = "2",
        .filepath = "basics.txt"
    },
    {
        .name = "hellostr",
        .code = 
        "str main() {\n"
        "    num a = 1;\n"
        "    if( a <= 2 ) {\n"
        "        return \"hello123\";\n"
        "    }\n"
        "    return \"%HELLO\";\n"
        "}\n",
        .expect = "hello123",
        .filepath = "basics.txt"
    },
    {
        .name = "and-or",
        .code = 
        "bol main() {\n"
        "    num a = 1;\n"
        "    return (a >= 6) and (6 <= a) or true;\n"
        "}\n",
        .expect = "false",
        .filepath = "basics.txt"
    },
    {
        .name = "funcall",
        .code = 
        "num add(num a, num b) {\n"
        "    return a + b;\n"
        "}\n"
        "num main() {\n"
        "    num a = 3;\n"
        "    return add(a, 4);\n"
        "}\n",
        .expect = "7",
        .filepath = "basics.txt"
    },
    {
        .name = "foreach",
        .code = 
        "num loop(num a) {\n"
        "    var array = [0, 1, 10]; \n"
        "    for(num i in array) {\n"
        "        a = a + i;\n"
        "    }\n"
        "    return a;\n"
        "}\n"
        "num main() {\n"
        "    num a = 0;\n"
        "    return loop(a);\n"
        "}\n",
        .expect = "11",
        .filepath = "basics.txt"
    },
    {
        .name = "ifelse",
        .code = 
        "num loop() {\n"
        "    num a = 0;\n"
        "    for(num i in [1, 2, 3, 4]) {\n"
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
        "num main() {\n"
        "    return loop();\n"
        "}\n",
        .expect = "4",
        .filepath = "basics.txt"
    },
    {
        .name = "negate",
        .code = 
        "bol main() {\n"
        "    num a = 100.4;\n"
        "    return -a;\n"
        "}\n",
        .expect = "-100.4",
        .filepath = "basics.txt"
    },
    {
        .name = "divide",
        .code = 
        "bol main() {\n"
        "    num a = 1;\n"
        "    return a / 2;\n"
        "}\n",
        .expect = "0.5",
        .filepath = "basics.txt"
    },
    {
        .name = "modulus",
        .code = 
        "bol main() {\n"
        "    num a = 1;\n"
        "    return a % 2;\n"
        "}\n",
        .expect = "1",
        .filepath = "basics.txt"
    },
    {
        .name = "not",
        .code = 
        "bol main() {\n"
        "    num a = 1;\n"
        "    return not (a == 2);\n"
        "}\n",
        .expect = "true",
        .filepath = "basics.txt"
    },
    {
        .name = "native-extern",
        .code = 
        "#extern none print(str arg);\n"
        "num main() {\n"
        "    print(\"hello\n\");\n"
        "    return 0;\n"
        "}\n",
        .expect = "0",
        .filepath = "basics.txt"
    },
    {
        .name = "operator-preceedence",
        .code = 
        "num main() {\n"
        "    if( (1 + 2 * 3) != 7 ) {\n"
        "        return false;\n"
        "    }\n"
        "    if( (1 - 2 / 2) != 0 ) {\n"
        "        return false;\n"
        "    }\n"
        "    return true;\n"
        "}\n",
        .expect = "true",
        .filepath = "basics.txt"
    }
};

#endif // LANGTEST_H_
