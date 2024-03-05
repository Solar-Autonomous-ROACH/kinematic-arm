# Armada

# Repositry structure: 

## src

All the library's code including .c and .h files.

## include

This header file has the header files that will be exposed as part of the library.

## Makefile

Compiles the library. The compiled library is stored in `lib` directory (in gitignore). The object files are stored in `dist` directory. Check this [section](#modifying-rover-library-path-and-arm-library-path) to see how you can modify (librover) library paths. We currently assume librover is located in `/home/ubuntu/SIMBA_C_Driver/rover`

## examples

Contains examples of how to use the library. This also includes a Makefile which does the neccessary linking for the libraries needed (libarm and librover). The makefile run `make` in the  root of the repo to make sure the library gets compiled if needed.
The examples assume the directory hiearchy of the kria board. If the libraries are located elsewhere, you can 


# Info

## Modifying rover library path and arm library path
You can set environment variables ROVERLIBPATH and ARMLIBPATH to update the location of the library.
By default, they are set to:
```
ROVERLIBPATH ?= /home/ubuntu/SIMBA_C_Driver/rover
ARMLIBPATH ?= /home/ubuntu/kinematic-arm
```
You can simply modify them by setting ROVERLIBPATH and ARMLIBPATH in your shell. For example,
```bash
export ROVERLIBPATH=~/Documents/projects/capstone/SIMBA_C_Driver/rover
export ARMLIBPATH=~/Documents/projects/capstone/kinematic-arm
```

## Running vision dummy instead of regular vision program
When you compile the library, the regular vision program will be used by default. You can use the dummy vision program instead by making this way
```bash
VISION_DUMMY=1 make
```
You should get a warning that the following warning:
`warning: Using dummy vision program`


