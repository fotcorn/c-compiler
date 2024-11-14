# Datatypes
* int
* char
* struct
* arrays
* pointers, one level deep, maybe two.

No floats or doubles. No booleans, use int with 0 and 1.
No modifiers like const, volatile, register etc.

# Operations
* arithmetic: +, -, *, /
* assignment: =
* comparison: ==, !=, >, <, >=, <=
* logical: &&, ||

We don't support increment/decrement operators.
Only simple assigment, no compound assignment.

# Control structures
* if/else
* while
* function definition, call, return

We might need break and continue and some point, but trying without for now.

# Code generation

We'll generate x86-64 assembly code.
