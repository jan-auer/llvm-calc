
# LLVM Calculator

`llvm-calc` is a small utility which compiles simple arithmetic expressions to LLVM and executes them using JIT.

## Prerequisites

To build this project, please ensure that *clang* and *llvm* in version *3.4* or newer are available:

 - On Linux, install `clang` and the `llvm-dev` package. 
 - On Mac OS, use homebrew to install the `llvm` package and link it with `brew link llvm --force`.
 - On Windows, get a VM.

For more information, please follow the instructions on [Get Started](http://clang.llvm.org/get_started.html).

## Install

Simply clone this repository and run `make`.
This should create an executable called `calc` in the project directory.

## Usage

Call the program `calc` with an expression and any number of parameters. 

```
$ ./calc [expression] [params...]
```

Valid expressions comprise:

 - **Constants**: 64 bit integer values, like `42`, `4711` or `-1`. 
                  Constant **must not** start with *0*.
 - **Variables**: Parameters of the compiled function. 
                  Variables either start with with an underscore, dollar or hash sign followed by the zero-based index of the parameter, e.g. `#0`, `_1`, `$2`.
 - **Operations**: Operations are defined in infix notation with two expressions connected by an operator. 
                   Possible operators are `+`, `-`, `*` and `/`.
 - **Parentheses**: Use parantheses to create sub-expressions, e.g. to change operator precedence: `2 * (1 + $0)`.
 
## Example

The following example evaluates the expression *(a + b) * (c - d)* for the values *a=1, b=2, c=5* and *d=2*:

```
$ ./calc '($0 + $1) * ($2 - $3)' 1 2 5 2
LLVM code compilation completed successfully.

---------
; ModuleID = 'test'

define i64 @calc(i64, i64, i64, i64) {
EntryBlock:
  %ADD = add i64 %0, %1
  %Sub = sub i64 %2, %3
  %Mul = mul i64 %ADD, %Sub
  ret i64 %Mul
}
---------

Starting calc with JIT ...
RESULT: 9
```