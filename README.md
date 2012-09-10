brainfuck
=========

[Brainfuck](http://en.wikipedia.org/wiki/Brainfuck) is an esoteric programming language noted for its extreme minimalism. It is a [Turing tarpit](http://en.wikipedia.org/wiki/Turing_tarpit), designed to challenge and amuse programmers, and was not made to be suitable for practical use. It was created in 1993 by Urban Müller.

The project contains a full-functional compiler and a JIT runner for the language.

Installation
------------

Before installing the program, you'll need to install [LLVM](http://llvm.org) 3.1 or higher and a working C/C++ linker, [Clang](http://clang.llvm.org) is recommended as [GCC](http://gcc.gnu.org) failed to link on some platforms. 

Usage
-----

* `bf` is the JIT runner, which can run brainfuck program directly
* `bfc` is the compiler, can be invoked as `bfc [-o output] src`, default output file name is `a.out`

Notes
-----

You can find some Brainfuck programs under `test` directory

Have Fun.

