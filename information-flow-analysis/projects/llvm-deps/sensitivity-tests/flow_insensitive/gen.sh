rm opt.ll
rm optO0.ll
rm opt-mem2reg.ll
rm optO0-mem2reg.ll

rm opt.txt
rm optO0.txt
rm opt-mem2reg.txt
rm optO0-mem2reg.txt

../../../../Debug+Asserts/bin/clang -S -emit-llvm ./main.c -o /dev/stdout | ../../../../Debug+Asserts/bin/opt -S -o opt.ll
../../../../Debug+Asserts/bin/clang -S -emit-llvm -O0 ./main.c -o /dev/stdout | ../../../../Debug+Asserts/bin/opt -S -o optO0.ll
../../../../Debug+Asserts/bin/clang -S -emit-llvm -O0 ./main.c -o /dev/stdout | ../../../../Debug+Asserts/bin/opt -mem2reg -S -o optO0-mem2reg.ll
../../../../Debug+Asserts/bin/clang -S -emit-llvm ./main.c -o /dev/stdout | ../../../../Debug+Asserts/bin/opt -mem2reg -S -o opt-mem2reg.ll

mv opt.ll opt.txt
mv optO0.ll optO0.txt
mv opt-mem2reg.ll opt-mem2reg.txt
mv optO0-mem2reg.ll optO0-mem2reg.txt
