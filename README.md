# Uncoolc

This repo is just my implementation of a compiler following the Stanford course [Compilers](https://www.edx.org/learn/computer-science/stanford-university-compilers). The implementation is a work in progress.

The code might be very different from the implementations presented in the course as I'm trying to write each bit _ahead_ of the relevant class, which I think maximizes the educational purpose of the project.

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
