# llvm-nullptr

An LLVM pass that aims to detect bad pointer usage in C code.
Assignment for a compiler course.

Heavy WIP:
initial commit only supports bad pointer usage without taking control flow into account.

Build:

    $ cd llvm-nullptr
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ cd ..

Run:

    $ clang -Xclang -load -Xclang build/nullp/libNullpPass.* something.c
