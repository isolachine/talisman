## Mediator Project ##

# Installation #

The following steps will build and install LLVM and prepare it for out of source pass compilation.
Currently we have only tested the tool to be used with LLVM 5.0 & 9.0, but it should work on all versions in between.

1. Download LLVM [source](http://llvm.org/releases/)
and unpack it in a directory of your choice which will refer to as `[LLVM_SRC]`

2. Create a build directory in [LLVM_SRC]

    ```bash
    $ mkdir llvm-build
    $ cd llvm-build
    ```
3. Instruct CMake to detect and configure your build environment:

   ```bash
   $ cmake -DCMAKE_BUILD_TYPE=Debug -DLLVM_TARGETS_TO_BUILD=X86 [LLVM_SRC]
   ```

    Note that is we do not specify `LLVM_TARGETS_TO_BUILD`, LLVM will by default build for all
	supported backends. This will save us a fair bit of time building LLVM.

4. Now start the actual compilation within your build directory

    ```bash
    $ cmake --build .
    ```
    The `--build` option is a portable way to tell cmake to invoke the underlying
    build tool (make, ninja, xcodebuild, msbuild, etc.)

5. Building takes some time to finish. After that we will specifiy a local installation.
	
    ```bash
    $ export LLVM_HOME=/home/location/to/install
    ```
    Alternatively, you can add this environment variable to /etc/environment/ or .bashrc.
    We will need `[LLVM_HOME]` in the next stage, and we assume that you have defined
    it as an environment variable `$LLVM_HOME`. Now you can issue the following command
    ```bash
    $ cmake -DCMAKE_INSTALL_PREFIX=$LLVM_HOME -P cmake_install.cmake
    ```

6. Once LLVM is installed properly, navigate to this project and 

   ```
   $ mkdir build/
   $ cd build/
   $ cmake ../
   $ make
   ```




