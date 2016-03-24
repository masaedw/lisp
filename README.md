# lisp

An implementation of a subset of scheme (r7rs) written in C based on 3imp.

* compiler (supports call/cc)
* vm
* syntaxes (defaine, lambda, if, set!, begin, and, of, let, let\*, letrec, letrec\*, ...)
* data types (integer(fixnum), string, port, vector, bytevector, symbol, boolean, ...)
* library functions (+, -, *, /, <, <=, >, >=, eq?, eqv?, equal?, cons, car, cdr, list, ...)
* macros (define-macro)

## sample programs

See samples.

* bf.scm -- brainfuck interperter
* fizzbuzz.scm -- fizzbuzz

## How to build

```
cmake .
make
```

## requirements

* cmake
* Boehm GC
