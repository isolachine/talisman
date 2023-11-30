#!/bin/bash
# Run Constant Analyzer Tool

#Store the number of arguments

len=$#

#check the total number of arguments

if [ $len -ne 1 ]; then

echo "Specify the path to source target as the only argument"

fi


# /home/user/Setup-Documentation/linux-reference/linux-4.14.1/security/selinux

/home/user/LLVM/llvm-project/build/bin/pp-trace /home/user/Setup-Documentation/linux-reference/linux-4.14.1/security/selinux/hooks.c -- -Wp,-MD, /home/user/Setup-Documentation/linux-reference/linux-4.14.1/security/selinux/.hooks.o.d  -nostdinc -isystem /usr/lib/clang/9/include -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/arch/x86/include -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/arch/x86/include/generated/uapi -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/arch/x86/include/generated  -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/include -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/arch/x86/include/uapi -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/arch/x86/include/generated/uapi -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/include/uapi -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/include/generated/uapi -I/usr/lib/gcc/x86_64-linux-gnu/9/ -include /home/user/Setup-Documentation/linux-reference/linux-4.14.1/include/linux/kconfig.h -D__KERNEL__ -fno-pie -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -m64 -mno-80387 -mtune=generic -mno-red-zone -mcmodel=kernel -DCONFIG_X86_X32_ABI -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_AS_CFI_SECTIONS=1 -DCONFIG_AS_FXSAVEQ=1 -DCONFIG_AS_CRC32=1 -DCONFIG_AS_AVX=1 -DCONFIG_AS_AVX2=1 -pipe -Wno-sign-compare -fno-asynchronous-unwind-tables -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -fno-delete-null-pointer-checks -fno-pie -no-pie -O0 -Wframe-larger-than=2048 -fno-stack-protector -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -pg -mfentry -DCC_USING_FENTRY -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -DCC_HAVE_ASM_GOTO  -fprofile-arcs -ftest-coverage -D"KBUILD_STR(s)=#s" -D"KBUILD_BASENAME=KBUILD_STR(tomoyo)"  -D"KBUILD_MODNAME=KBUILD_STR(tomoyo)" -c &> selinux.pptrace

grep -R "#define" $1 &> selinux.defines

python organize-pp-trace.py selinux.defines selinux.pptrace

/home/user/LLVM/llvm-project/build/bin/constant-analyzer /home/user/Setup-Documentation/linux-reference/linux-4.14.1/security/selinux/hooks.c -- -Wp,-MD, /home/user/Setup-Documentation/linux-reference/linux-4.14.1/security/selinux/.hooks.o.d  -nostdinc -isystem /usr/lib/clang/9/include -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/arch/x86/include -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/arch/x86/include/generated/uapi -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/arch/x86/include/generated  -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/include -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/arch/x86/include/uapi -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/arch/x86/include/generated/uapi -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/include/uapi -I/home/user/Setup-Documentation/linux-reference/linux-4.14.1/include/generated/uapi -I/usr/lib/gcc/x86_64-linux-gnu/9/ -include /home/user/Setup-Documentation/linux-reference/linux-4.14.1/include/linux/kconfig.h -D__KERNEL__ -fno-pie -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -m64 -mno-80387 -mtune=generic -mno-red-zone -mcmodel=kernel -DCONFIG_X86_X32_ABI -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_AS_CFI_SECTIONS=1 -DCONFIG_AS_FXSAVEQ=1 -DCONFIG_AS_CRC32=1 -DCONFIG_AS_AVX=1 -DCONFIG_AS_AVX2=1 -pipe -Wno-sign-compare -fno-asynchronous-unwind-tables -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -fno-delete-null-pointer-checks -fno-pie -no-pie -O0 -Wframe-larger-than=2048 -fno-stack-protector -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -pg -mfentry -DCC_USING_FENTRY -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -DCC_HAVE_ASM_GOTO  -fprofile-arcs -ftest-coverage -D"KBUILD_STR(s)=#s" -D"KBUILD_BASENAME=KBUILD_STR(tomoyo)"  -D"KBUILD_MODNAME=KBUILD_STR(tomoyo)" -c &> selinux.constants



