# bxzstr documentation
This file details how to build and run automatic tests for bxzstr.

## Building tests
### Requirements
- C++11 compliant compiler.
- git.
- Internet access.

### Compiling from source
Enter the bxzstr directory and run
```
cmake -DCMAKE_BUILD_TESTS=1 -DCMAKE_BUILD_TYPE=Debug .
make
```

This will build the `runTests` executable.

## Running the tests
After building the tests, enter the bxzstr directory and run
```
./runTests
```

This will print the results of running the tests.
