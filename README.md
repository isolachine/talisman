# TALISMAN: Tamper Analysis for Reference Monitors

This repository contains the source code for the paper _TALISMAN: Tamper Analysis for Reference Monitors_ accepted to NDSS 2024.

**This README file is still under construction. If you need immediate help on how to run the analysis, please feel free to drop the authors a message.**

---

## Requirements and preparation


### System Requirements

We've been building the system under Ubuntu 18.04, which provides the best compatibility of softwares used and convenience in compiling the version of Linux we are analyzing. The instructions in this documentation assumes the user is running this version of Ubuntu Linux.

### Compiling the analysis toolchain

1. Before building, make sure to check:

- The system's default 'python' is linked to a python2 executable. Check by `python --version`.
- gcc-5 is installed for compiling the LLVM 3.7.1 version. Use `gcc -v` to check version. 
- Use `update-alternatives` to change default `python` and `gcc` of the system if versions does not match.

2. To build the toolchain, run the commands below.

```sh
# First direct to project's root dir
cd ./information-flow-analysis

# Configure the project under root and run 'make' to build LLVM
./configure
make

# Direct to projects folders, configure and make for each package.
cd projects/poolalloc/
./configure
make

cd ../llvm-deps/
./configure
make
```

### Compiling the Linux kernel

#### Instructions for generating LLVM bitcode files for specific modules

 (Optional): Follow this reference if you are attempting to compile an older version of linux and require an earlier gcc version
    - Install gcc-5: <https://askubuntu.com/questions/1229774/how-to-use-an-older-version-of-gcc>
 (Optional): Patches for kernel 4.14.1 below. These were required for compiling on Ubuntu 20.04 with gcc version 9.4
 (Required): Install the following libraries: libssl-dev libelf-dev

- cd linux-X.X.X
- Run 'make allyesconfig'
- Edit the Makefile and add the following commands to add additional compiler flags. The location to insert these commands will vary slightly between linux versions. In linux version 4.14.1 all variables have been created by line 630.

```sh
    KBUILD_CFLAGS += $(call cc-option, -fno-pie)
    KBUILD_CFLAGS += $(call cc-option, -no-pie)
    KBUILD_AFLAGS += $(call cc-option, -fno-pie)
    KBUILD_CPPFLAGS += $(call cc-option, -fno-pie) 
```

- Run `make prepare`
- Run `make modules_prepare`
- Run `make modules``  // Needed to generate modules.symvers file
- Run the clang compilation command for each LSM module (You may need to adjust the path to clang depending on how you compiled LLVM):

```sh
make V=0 \
    CLANG_FLAGS="-emit-llvm-bc" \
    HOSTCC="/usr/local/bin/clang --save-temps=obj -no-integrated-as -g" \
    CC="/usr/local/bin/clang --save-temps=obj -no-integrated-as -g" \
    -j8 M=security/tomoyo \
    2>/dev/null
```

- Link .bc files
    At this point there should be a .bc file for each .c file in the source tree. We can link .bc files together using the LLVM command llvm-link.

```sh
    llvm-link *.bc -o binary_name.bc 
    # Will link all .bc files in current directory and output to binary_name.bc.
    # Be mindful if source tree as subdirectories, depending on what you are 
    # compiling you may need to recursively find all .bc files.
```

#### Libraries needed: `libssl-dev libelf-dev`

---

## Running the tamper analysis

### Running the source analysis _[Under Construction]_

### Finding the information flow violations

```sh
# Direct to the benchmark folder
cd ./information-flow-analysis/projects/llvm-deps/linux_security_tests

# Running the analysis
# This script runs the information flow analysis on all LSMs and generates the results.
./linux_tests_automation.sh
```

---

## Running the benchmark on endorsed kernel

Please refer to this [README](https://github.com/mitthu/lsm-trusty-kernel/wiki) for more details on how to run the endorsement evaluation with LMBench.
