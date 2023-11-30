../../../../Debug+Asserts/bin/clang -S -emit-llvm ./main.c -o /dev/stdout | ../../../../Debug+Asserts/bin/opt -S -mem2reg -o example-opt.ll
