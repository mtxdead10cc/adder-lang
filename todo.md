# TODO

## New Language features

 - [-] Tuples
 - [-] Structs
 - [-] Enums
 - [-] Typedefs  
 - [-] Array indexing (safe)
 - [-] Tail recursion (safe)

## Structural changes

### Split up in stages

| Step | Input    | Descr         | Output    |
| ---  | -------- | ------------- | --------- |
| 1.   | SOURCE   | Parser        | AST       |
| 2.   | AST      | Typechecker   | AST       |
| 3.   | AST      | Compiler      | BYTECODE  |

