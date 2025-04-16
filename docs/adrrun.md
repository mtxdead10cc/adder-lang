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
