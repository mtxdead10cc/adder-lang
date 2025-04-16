# What's next?

## Reworking the virtual machine

In a sense, the VM is lagging behind the rest of the code base. val_t (uint64_t) contains information about the type of the value and the value itself. Since the compiler does typechecking and has complete type information, the type encoding is not needed in order to execute a compiled program.

Having type information encoded in the val_t decreases the number of bits available to represet the value itself. 

The VM currently does runtime typechecking in a few places, removing these checks would increase performance. 

Value printing need to be handled in some other way, since there would be no way of inferring the the type based on the value itself. 

Currently there is only one stack in the VM, to simplify the VM code and potentially gain some performance the idea is to have a separate stack for function frames.

Another part of this work would be to figure out how to handle stack and heap representations of tuples and user defined structures.

## Other improvements

Add support for function overloading, perhaps via name mangling (functions with the same name but different type signatures).

Implement some basic optimization techniques in the compiler (constant folding, peephole optimization).
