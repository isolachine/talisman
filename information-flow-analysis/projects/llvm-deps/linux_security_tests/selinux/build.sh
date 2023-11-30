#!/usr/bin/env bash
# linking example

BASE_DIR=/home/quan/Documents
LINUX_DIR=$BASE_DIR/linux
CUR_DIR=$BASE_DIR/LLVM-Deps
BIN_DIR=$CUR_DIR/Debug+Asserts/bin
ANALYSIS_DIR=$CUR_DIR/projects/llvm-deps/linux_security_tests/selinux

sudo update-alternatives --set gcc /usr/bin/gcc-5
sudo update-alternatives --set g++ /usr/bin/g++-5

cd $LINUX_DIR
pwd
make V=0 \
    CLANG_FLAGS="-emit-llvm-bc" \
    HOSTCC="/home/quan/Documents/LLVM-Deps/Debug+Asserts/bin/clang --save-temps=obj -no-integrated-as -g" \
    CC="/home/quan/Documents/LLVM-Deps/Debug+Asserts/bin/clang --save-temps=obj -no-integrated-as -g" \
    -j8 M=security/selinux \
    2>/dev/null

cd security/selinux
pwd
$BIN_DIR/llvm-link \
    hooks.bc avc.bc xfrm.bc \
    -o test.bc

mv test.bc $ANALYSIS_DIR/
cd $ANALYSIS_DIR
pwd
ls -la test.bc
$BIN_DIR/llvm-dis test.bc

sudo update-alternatives --set gcc /usr/bin/gcc-11
sudo update-alternatives --set g++ /usr/bin/g++-11
