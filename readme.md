# adder-lang

Adder is a small language that compiles to byte code. The language is built to be easily embedded within a native-C host application.

## About the language

* Strongly typed (bidirectional typechecking and a few other compiletime checks)

* C-like syntax

* Import and export functions (between C and adder)

* Garbage collected (arrays)

* In development, potentially unstable, no official release version, and no promises regarding backward compatibility in future updates (USE AT YOUR OWN RISK)

* Currently only targeting Linux and Android (adding support for more platforms should not be too hard)

* No external dependencies (except for libc)

* Limited by design (endless looping is not allowed)

### Language Features

### Import from host

Native C functions can be registered and made available in the adder code.

There are currently a (small) number of built-in functions automatically available when using adrrun (or adder/xutils).

```c
import void print(string msg);                 // prints a message
import string itos(int v);                     // create a string from an integer value 
import string ftos(float v);                   // create a string from a float value  
import string btos(bool v);                    // create a string from a boolean value 
import string stradd(string fst, string snd);  // concatenate two strings 
```

### Export to host

Adder functions can be exported, this enables them to be called from the native C code.

```c
// export a function called 'hello'
export string hello() {
    return "adder says: hello";
}

// any function called 'main' is 
// automatically exported
string main() {
   return "adder says: main"; 
}
```

### Variables and Data types

adder has support for the following basic types

* int (integer)

* float (floating point)

* bool (boolean)

* array<type> (arrays, where type is  any other type)

* string (is actually just an alias for array<char>) 

The char (character) type is supported internally but can't be defined as a left hand side value in assignments for the moment.

```c
void main() {
    bool b = true;
    int i = 10;
    float f = 0.1;
    string s = "string";
    array<int> a = [1,2,3,4,5];
}
```

### Control flow

Adder supports controle flow by way of if ... else if .. else. 

Most operators are similar to C with the notable exception that '&&'' is replaced with the **and** keyword and '||'' is replaced with **or** keyword.

```c
string main(int n) {
    if ( n < 0 ) {
        return "the number is negative";
    } else if ( n < 100 and (n % 2) == 0 ) {
        return "the number is less than 100 and even";
    } else {
        return "we don't deal with numbers like that";
    }
}
```

### Loops

We can iterate over arrays one item at the time, as can be see in the example below.

There is currently no way to do arbitrary iteration (eg. 'while(true)' etc.), however, using adder in conjunction with our own host-provided functions we can account for this limitation.

```c
int calculate_sum(array<int> numbers) {
    int sum = 0;
    for(int n in numbers) {
        sum = sum + n;
    }
    return sum;
}
```

## Host integration

The c code for this example can be found in the [examples](examples/host/main.c) folder.

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

We call vm_create to create a virtual machine. The memory parameter essentially means how many values (val_t) the VM has to work with in total (stack + heap).

```c
vm_t vm = { 0 };
bool create_ok = vm_create(&vm, 128); 
```

### Invoke adder functions

xu_invoke.h is a massive file that mostly contains (generated) c macros. The idea is to simplify the binding code by providing helper macros that all take a pointer to a virtual machine, an extracted adder function handle (xu_caller_t) and the value arguments to the function.

The naming convention is basically <return-type>call<arg-type><arg-type> etc. Meaning, for example if **foo** is a function that returns a float and takes 3 integers as arguments, the macro to use would be **float val = fcalliii(&vm, &foo, 1, 2, 3)**.

| type               | signature letter |
| ------------------ | ---------------- |
| bool               | b                |
| int                | i                |
| float              | f                |
| string             | s                |
| void (return type) | v                |

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

## Adrrun

Adrrun is a simple command-line tool for executing .adr scripts and run the test suite.

To demonstrate how this works, let's say we have a simple example program.

**test.adr**

```c
// import the built-in print function
import void print(string msg);

void main() {
    print("Hello World!");
}
```

To compile and execute test.adr we simply call adrrun with the path to the .adr file as the only argument. Unless we provide an entrypoint explicitly adrrun will look for a function called main (with no arguments).

```bash
$ ./adrrun test.adr
test.adr [OK]
> Hello World!
```

We can export any function we'd like. Let's have a look at how this works.

**test-export.adr**

```c
// import the built-in print function
import void print(string msg);

export int add(int a, int b) {
    print("adder is adding numbers");
    return a + b;
}

export int subtract(int a, int b) {
    print("adder is subtracting numbers");
    return a - b;
}
```

To tell adrrun which entrypoint and arguments to use we provide an extra argument.

```bash
$ ./adrrun test-export.adr "add(3, 5)"
test-export.adr [OK]
> adder is adding numbers
 => 3.000000
```

adrrun has support for showing dissasembly and showing a representation of the ast, in case you'd like to investigate the byte code or the behavior of the compiler.

```bash
$ ./adrrun test.adr -d
test.adr [OK]
INFO: [DISASSEMBLY]
#    0| make-frame       0         0        
#    9| push-const       0         (H)
#   14| push-const       1         (e)
#   19| push-const       2         (l)
#   24| push-const       2         (l)
#   29| push-const       3         (o)
#   34| push-const       4         ( )
#   39| push-const       5         (W)
#   44| push-const       3         (o)
#   49| push-const       6         (r)
#   54| push-const       2         (l)
#   59| push-const       7         (d)
#   64| push-const       8         (!)
#   69| push-const       9         (12.000000)
#   74| make-array      
#   75| call-native      0        
#   80| return-nothing  
#   81| halt            
> Hello World!
```
