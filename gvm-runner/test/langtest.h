
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
        .name = "bool ops",
        .code = 
        "bol main() {\n"
        "    num a = 1;\n"
        "    return (a <= 6) and (6 >= a) or true;\n"
        "}\n",
        .expect = "true",
        .filepath = "basics.txt"
    }
};

#endif // LANGTEST_H_
