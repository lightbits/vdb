## CMake example

This example shows how to integrate vdb into your CMake project. You may run the example by following the usual CMake steps:

```
mkdir build
cd build
cmake ..
make && ./main
```

To integrate VDB into your project I suggest to do the following.

* Make sure users of your project acquire [SDL2](https://www.libsdl.org/).

For example, add a notice in your readme telling Debian-based users (including Ubuntu) to acquire SDL2 by running the following line in their terminal:

```
sudo apt-get install libsdl2-dev
```

This will install everything necessary to build programs that use SDL2.

* Update your CMakeLists.txt to include SDL2.

You may look at the CMakeLists.txt file in this folder for inspiration.

* Bundle VDB with your project.

One way to include VDB with your project is to copy the [src](../src) directory into your project and distribute it alongside. This way, any program that uses VDB can include the library by writing ```#include "path/to/vdb.h"``` at the top of the header file, and it will work for all users.
