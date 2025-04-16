## Host integration

The c code referenced in this section can be found in the [examples](examples/host/main.c) folder.

### Register a log function

```c
// define a log hadler
void logfn(sh_log_tag_t _tag, const char* fmt, va_list args) {
    vprintf(fmt, args);
}
```

```c
// register the log handler
sh_log_init(&logfn);
```

### Load adder source

xu_classlist_t (classlib) is a container for managing (loading, compiling and unloading) adder code.

xu_class_t (class) represents the compiled adder binary and bindings we may create.

```c
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

// alternatively; use xu_class_read_and_create to 
// read the adder source from a file 
source_code_t code = program_source_from_memory( 
    adder_code, strlen(adder_code));

xu_class_t class = xu_class_create(&classlib, &code, 0);

program_source_free(&code);

// now we have a class (essentially a reference to
// the compiled adder code)

```

### Register a host function

To import a c function in adder we need to have it registered with the class.

```c
// define a function adder can import
val_t get_name(ffi_hndl_meta_t m, int argcount, val_t* args) {
    return xu_string_to_val(m.vm, "nameless one");
}
```

```c
// inject 'get_name' (enable import)
xu_class_inject(class,
    "get_name", ift_func(ift_list(ift_char())),
    (ffi_handle_t) {
        .local = NULL,
        .tag = FFI_HNDL_HOST_FUNCTION,
        .u.host_function = get_name
    });
```

The odd looking expression ift_func(ift_list(ift_char())) is essentially a way for us to tell adder about the function signature. In this case we register a function that returns an array of characters (string) and takes no arguments.

### Get exported function handle

```c
// get a handle for 'say_hello'
xu_caller_t say_hello = xu_class_extract(class,
    "say_hello", ift_func(ift_list(ift_char())));
// get a handle for 'plus_one'
xu_caller_t plus_one = xu_class_extract(class,
    "plus_one", ift_func_1(ift_void(), ift_int()));
```

### Finalize the xulib setup

We need to finalize the class list (classlib) before running the VM. Finalize does the following

- Sets up the execution environment

- Verifies that all classes compiled ok

```c
xu_finalize_all(&classlib); // or use xu_class_finalize
```

### Create a Virtual Machine

We call vm_create to create a virtual machine. The second parameter (128) represents how many values (val_t) the VM has to work with in total (stack + heap).

```c
vm_t vm = { 0 };
bool create_ok = vm_create(&vm, 128); 
```

### Invoke adder functions

xu_invoke.h is a massive file that mostly contains (generated) c macros. The idea is to simplify the binding code by providing helper macros that all take a pointer to a virtual machine, an extracted adder function handle (xu_caller_t) and the value arguments to the function.

The naming convention is basically \<return-type\>call\<arg-type\><arg-type\>... Meaning, for example if **foo** is a function that returns a float and takes an integer, a float and a bool as arguments, the macro to use would be **fcallifb(&vm, &foo, 1, 2.0, true)**.

| type               | signature letter |
| ------------------ | ---------------- |
| bool               | b                |
| int                | i                |
| float              | f                |
| string             | s                |
| void (return only) | v                |

```c
// call 'plus_one'
int n = icalli(&vm, &plus_one, 1);
// call 'say_hello'
vcall(&vm, &say_hello);
```

### Cleanup

When we are done with the VM and the class list (classlib) we call the cleanup functions.

```c
vm_destroy(&vm);
xu_cleanup_all(&classlib);
```