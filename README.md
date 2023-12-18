# Uncoolc

This repo is just my implementation of a compiler following the Stanford course [Compilers](https://www.edx.org/learn/computer-science/stanford-university-compilers). The implementation is a work in progress.

The code is very different from the implementations presented in the course. This implementation tries to go one step closer to what a production compiler would look like.

Here are some of the bigger differences (marked as planned if not yet done).

- More modern C++.
- Hand-written lexer instead of using flex.
- Hand-written parser instead of using bison.
- Target an intermediate representation (IR) instead of assembly directly [planned]
- Implement an optimization pipeline over the IR [planned]
- Implement an LLVM backend [planned]
- Implement a hand-written backend [planned]

Here are some of the things that are **out of scope** for uncoolc, which would need to be much better in a production compiler.

- Performance of the compiler is not a concern. E.g. the code uses `std::` containers whenever it simplifies the code.
- Performance of the produced code is a non-goal. Showing good examples of optimization passes _is_ a goal, though.
- Quality of error reporting is pretty poor. The course's assignment compiler is a better example of reasonable error reporting, and actual production compilers like `clang` are on a different level entirely.

## Build 

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

## Test

Tests are built as part of the normal build. After you build with the instructions above, just run:

```sh
./tests
```

## License & Copyright

The code in this repo is shared under the MIT License, which you can find under [LICENSE.txt](/LICENSE.txt).

Copyright © 2023 Ian Tayler.

The only exception is [Doctest](https://github.com/doctest/doctest/tree/master) which is bundled and shared under the license in [test/include/LICENSE.txt](/test/include/LICENSE.txt). 
