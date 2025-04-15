#include <stdio.h>
#include <vm.h>
#include <sh_value.h>
#include <sh_ffi.h>
#include <vm_value_tools.h>
#include <sh_log.h>
#include <vm_heap.h>
#include <vm_env.h>
#include <co_program.h>
#include <co_ast.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <assert.h>
#include <sh_program.h>
#include <sh_arena.h>
#include <sh_ift.h>
#include <xu_lib.h>
#include <xu_invoke.h>

#define UNUSED_PARAM(X) (void)(X)

void logfn(sh_log_tag_t _tag, const char* fmt, va_list args) {
    UNUSED_PARAM(_tag);
    vprintf(fmt, args);
}

val_t get_name(ffi_hndl_meta_t m, int argcount, val_t* args) {
    UNUSED_PARAM(argcount);
    UNUSED_PARAM(args);
    return xu_string_to_val(m.vm, "nameless one");
}

int main(int argv, char** argc) {
    UNUSED_PARAM(argv);
    UNUSED_PARAM(argc);

    // register the log handler
    sh_log_init(&logfn);


    // LOADING ADDER SOURCE

    xu_classlist_t classlib = { 0 };

    char* adder_code =  "import void print(string msg);                       \n"
                        "import string stradd(string fst, string lst);        \n"
                        "import string get_name();                            \n"
                        "                                                     \n"
                        "export int plus_one(int number) {                    \n"
                        "   return number + 1;                                \n"
                        "}                                                    \n"
                        "                                                     \n"
                        "export void say_hello() {                            \n"
                        "    print(stradd(\"hello \", get_name()));           \n"
                        "}                                                    \n";

    // alternatively; use xu_class_read_and_create
    source_code_t code = program_source_from_memory( 
        adder_code, strlen(adder_code));

    xu_class_t class = xu_class_create(&classlib, &code, 0);

    program_source_free(&code);


    // IMPORTS / EXPORTS

    // inject 'get_name' (enable import)
    xu_class_inject(class,
        "get_name", ift_func(ift_list(ift_char())),
        (ffi_handle_t) {
            .local = NULL,
            .tag = FFI_HNDL_HOST_FUNCTION,
            .u.host_function = get_name
        });

    // get a handle for 'say_hello'
    xu_caller_t say_hello = xu_class_extract(class,
        "say_hello", ift_func(ift_list(ift_char())));

    // get a handle for 'plus_one'
    xu_caller_t plus_one = xu_class_extract(class,
        "plus_one", ift_func_1(ift_void(), ift_int()));

    xu_finalize_all(&classlib); // or use xu_class_finalize


    // RUNNING THE VM

    vm_t vm = { 0 };
    vm_create(&vm, 128);
    
    // call 'plus_one'
    int number = 1;
    printf("calling: plus_one(%d)\n", number);
    int result = icalli(&vm, &plus_one, number);
    printf("  got %d back\n", result);

    // call 'say_hello'
    printf("calling: say_hello()\n");
    vcall(&vm, &say_hello);


    // CLEANUP

    vm_destroy(&vm);
    xu_cleanup_all(&classlib);

    return 0;
}