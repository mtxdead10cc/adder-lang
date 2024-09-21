# GVM SCRIPT

## Operations

### push [number] (or [string] or [ivec2])

Push constant value to the stack.

### globstore [symbol]

Stores value on top of stack to a global register.

### globload [symbol]

Loads a value from a global register onto the stack.

### store [index]

Stores the value on the top of the stack to a local register.

### load [index]

Loads a value from a local variable to the top of the stack.

### print 

Pops the top off the stack and then prints the value.

### call [label]

Pushes the current instruction pointer to the stack and then jumps to label.

### frame-old [num-args] [num-locals]

Sets up a function frame on the stack.
1. pops the return address from stack
2. pops num-args from the stack
3. pushes frame-value to the stack
4. pushes num-locals to the stack
5. pushes arguments (poped at 2) to the stack 

### frame [num-args] [num-locals]

Sets up a function frame on the stack.
1. pops the return address from stack
2. pops num-args from the stack
3. pushes frame-value to the stack
4. pushes arguments (poped at 2) to the stack 
5. pushes num-locals to the stack

### return

1. Saves the top value of the stack (return value).
2. Pops all values until the frame-value is reached.
3. Moved the instruction pointer to the return address.
4. Pops the frame value.
5. Pushes the return value (if any).

### pop1

Pops a value off the stack.

### pop2

Pops 2 values off the stack.

### dup1  

Duplicates the top of the stack and pushes the duplicated value.

### dup2

Duplicates the two topmost values of the stack and pushes the duplicated values.

### rot2

Swaps the order of the two topmost values on the stack.

### is-less

1. pops a value from the stack (A)
2. pops a value from the stack (B)
3. pushes a boolean value (A < B)

### is-more

1. pops a value from the stack (A)
2. pops a value from the stack (B)
3. pushes a boolean value (A > B)

### is-equal

1. pops a value from the stack (A)
2. pops a value from the stack (B)
3. pushes a boolean value (A == B)

### if-false [label]

Pop the top off the stack. If the value is True continue with next instruction otherwise jump to the label.

### jump [label]

Jump to [label].

### exit [number]

Exit with an exit code ([number]).

### and 

1. pops a value from the stack (A)
2. pops a value from the stack (B)
3. pushes a boolean value (A and B)

### or

1. pops a value from the stack (A)
2. pops a value from the stack (B)
3. pushes a boolean value (A or B)

### mul

1. pops a value from the stack (A)
2. pops a value from the stack (B)
3. pushes a number value (A * B)

### add

1. pops a value from the stack (A)
2. pops a value from the stack (B)
3. pushes a number value (A + B)

### sub

1. pops a value from the stack (A)
2. pops a value from the stack (B)
3. pushes a number value (A - B)

### neg

1. pops a value from the stack (A)
3. pushes a number value (-A)

### array

1. Pops the array-length off the stack (number).
2. Allocates array-length amount of heap space. 
3. Copies array-length number of values from the stack and into the heap location.
4. Pops array-length number of values off the stack.
5. Pushes a reference to the array onto the stack.

### len

1. Pops an array-reference off the stack.
2. Pushes the length of the array onto the stack.

### iter

1. Pops an array-reference off the stack.
2. Pushes an iterator onto the stack.

### iter-next [exit-label]

1. Leaves iterator on the stack (not popping it).
2. Then either
 - Advance the iterator and push the corresponding value.
 - Otherwise, jump to exit-label.