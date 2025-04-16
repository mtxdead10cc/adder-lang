# adder-lang

Adder is a small language that compiles to byte code. The language is built to be easily embedded within a native-c host application.

## About the language

* Strongly typed (bidirectional typechecking and a few other compiletime checks)

* C-like syntax

* Import and export functions (between c and adder)

* Garbage collected (arrays)

* In development and potentially unstable (USE AT YOUR OWN RISK)

* Currently only targeting Linux and Android (adding support for more platforms should not be too hard)

* No external dependencies (except for libc)

* Limited by design (endless looping is not allowed)

## Example

```c
// import some host-provided functions
import string itos(int v);                      // integer-to-string
import void   print(string msg);                // printing
import string stradd(string fst, string snd);   // string concatenation

string get_description(int number) {
    if( number % 2 == 0 ) {
        return stradd(itos(number), ": an even number");
    } else {
        return stradd(itos(number), ": an odd number");
    }
}

void main() {
    array<int> numbers = [ 8, 9, 42 ];
    for(int n in numbers) {
        string description = get_description(n);
        print(stradd("  ", description));
    }
}
```
### Running the example 
```c
$ ./adrrun example.adr 
example.adr [OK]
>   8.000000: an even number
>   9.000000: an odd number
>   42.000000: an even number
```

## Documentation

- [Language features](docs/language-features.md)
- [Host program integration](docs/host-integration.md)
- [The adrrun command-line tool](docs/adrrun.md)
- [Where are we going?](docs/whats-next.md)
