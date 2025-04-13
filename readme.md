# adder-lang

Adder is a small language that compiles to byte code. The language is built to be easily embedded within a native-C host application.

## About the language

* Strongly typed (bidirectional typechecking and a few other compiletime checks)

* C-like syntax

* Import and export functions (between C and adder)

* Garbage collected (arrays)

* In development (use at your own risk)

* Currently only tested on Linux and Android

* No external dependencies (except for libc, for now)

* Limited by design (should never get stuck in endless loops)

### Language Features

### Import from host

Native C functions can be registered and made available in the adder code.

There are currently a (small) number of built-in functions automatically available when using adrrun (or adder/xutils).

```adder
import void print(string msg);                 // prints the message
import string itos(int v);                     // create a string from an integer value 
import string ftos(float v);                   // create a string from a float value  
import string btos(bool v);                    // create a string from a boolean value 
import string stradd(string fst, string snd);  // concatenate two strings 
```

### Export to host

Adder functions can be exported, this enables them to be called from the native C code.

```adder
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

```adder
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

```adder
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

```adder
int calculate_sum(array<int> numbers) {
    int sum = 0;
    for(int n in numbers) {
        sum = sum + n;
    }
    return sum;
}
```

### adrrun

Adrrun is a simple command-line tool for executing .adr scripts and run the test suite.

To demonstrate how this works, let's say we have a simple example program.

**test.adr**

```adder
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

```adder
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
