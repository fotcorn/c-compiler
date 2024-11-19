# Self-hosting C Compiler written by AI
This is an exercise on how easy it is to build a self-hosting (compiler that can compile itself) compiler with the help of AI models, mostly Claude Sonnet 3.5, but also o1-preview for a few things.

## Datatypes
* int
* char
* struct
* arrays
* pointers, one level deep, maybe two.

No floats or doubles. No booleans, use int with 0 and 1.
No modifiers like const, volatile, register etc.

## Operations
* arithmetic: +, -, *, /
* assignment: =
* comparison: ==, !=, >, <, >=, <=
* logical: &&, ||

We don't support increment/decrement operators.
Only simple assigment, no compound assignment.

## Control structures
* if/else
* while
* function definition, call, return

We might need break and continue and some point, but trying without for now.

## Code generation

We'll generate x86-64 assembly code.
