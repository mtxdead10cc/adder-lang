## Language Features

This section provides a breif overview of the adder language.

### Import from host

Native c functions can be registered and made available to the adder code.

There are currently a (small) number of built-in functions automatically available when using adrrun (or adder/xutils).

```c
import void print(string msg);                 // prints a message
import string itos(int v);                     // create a string from an integer value 
import string ftos(float v);                   // create a string from a float value  
import string btos(bool v);                    // create a string from a boolean value 
import string stradd(string fst, string snd);  // concatenate two strings 
```

### Export to host

Adder functions can be exported, this enables them to be called from the native c code.

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

Adder has support for the following basic types

| adder type name    | type                     |
| ------------------ | ------------------------ |
| bool               | boolean (true/false)     |
| int                | integer (32 bit)         |
| float              | float (32 bit)           |
| array\<*type*>     | *type* is any type       |
| string             | alias for array\<char>   |
| void (return only) | nothing                  |

The char (character) type is supported internally but can't be defined as a left side value in assignments for the moment.

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

Adder supports control flow by way of if ... else if .. else. 

Most operators are similar to C with the notable exception that **&&** is replaced with the **and** keyword and **||** is replaced with the **or** keyword.

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

We can iterate over arrays one item at the time, as can be seen in the example below.
There is no way to do arbitrary iteration (eg. 'while(true)' etc.), however, this might change in the future.

```c
int calculate_sum(array<int> numbers) {
    int sum = 0;
    for(int n in numbers) {
        sum = sum + n;
    }
    return sum;
}
```
