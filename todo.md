# TODO

## New Language features

 - [-] Tuples
 - [-] Structs
 - [-] Enums
 - [-] Typedefs  
 - [-] Array indexing (safe)
 - [-] Tail recursion (safe)

## New tooling support

 - [-] Enable implementing an LSP on top of the parser and typechecker.

## Structural changes

### Split up in stages

| Step | Input    | Descr         | Output    |
| ---  | -------- | ------------- | --------- |
| 1.   | SOURCE   | Parser        | AST       |
| 2.   | AST      | Typechecker   | AST       |
| 2.5. | AST      | Exporter      | JSON      |
| 3.   | AST      | Compiler      | BYTECODE  |

## Parser

 1. Add support for ??? nodes (failed to parse).
 2. Rework to support a flexible error trace.
 3. Parse all, on failure add ??? nodes, and log errors but continue parsing.

