# Bongo Language

A simple, and arguably useless, programming language written for the
SFU DNS Club's first event: the Create a Programming Language event.

This programming language allows you to define, set, add, subtract,
and multiply variables, but does not provide any control structures.
Thus, this language is absolutely static, and can only be used for
simple mathematical calculations, like a calculator.

A bongo instruction structure is as follows:

```
instruction: argument-1 argument-2 argument-3 ... argument-n
```

Where each instruction takes a list of arguments terminated by a new-line.
A bongo program is a sequence of such instructions.

A comment is defined by two colons at the beginning of a line;
whatever text follows these two colons is ignored by the compiler.

Available instructions are:

```
:: Define a variable
define: <var-name> <initial-value>

:: Set a variable's value to another variable's value
set: <a-name> <b-name>

:: Add two variables (result = a + b)
add: <res-name> <a-name> <b-name>

:: Subtract two variables (res = a - b)
subtract: <res-name> <a-name> <b-name>

:: Multiply two variables (res = a * b)
multiply: <res-name> <a-name> <b-name>

:: Return the integer value stored at the given variable
return: <var-name>
```
