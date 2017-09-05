## Example - Compiling with make

This example shows how to integrate vdb into a traditional Makefile-based project, where vdb is compiled seperately, before it is linked together with the rest of the project.

To do this, do the following:

* Create a file [vdb.cpp](vdb.cpp) that includes the vdb header

* In all other files that use vdb, write ```#define VDB_HEADER_ONLY``` before including vdb.h. This prevents the implementation part from being included. See [main.cpp](main.cpp).

* Update your Makefile to compile vdb and link against it. See [Makefile](Makefile).


