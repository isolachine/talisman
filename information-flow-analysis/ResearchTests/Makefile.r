CFLAGS = -g -O0

printLine.o:
	g++ -g -o $@ -c /home/troy/llvm-dsa/ResearchTests/instrumentation/BBInfo/printLine.cpp

.c.o:
	$(RM) $@
	clang $(CCFLAGS) -emit-llvm -c $< -o $<.bc
	opt -load /home/troy/llvm-dsa/Debug+Asserts/lib/Research.so -prtLnTrace $<.bc -o $<.g.bc >> /tmp/nodes 2>&1
	llc -filetype=obj $<.g.bc -o $@


OBJECTS	 = shell.o eval.o y.tab.o general.o make_cmd.o print_cmd.o $(GLOBO) \
	   dispose_cmd.o execute_cmd.o variables.o copy_cmd.o error.o \
	   expr.o flags.o $(JOBS_O) subst.o hashcmd.o hashlib.o mailcheck.o \
	   trap.o input.o unwind_prot.o pathexp.o sig.o test.o version.o \
	   alias.o array.o braces.o bracecomp.o bashhist.o bashline.o \
	   getcwd.o siglist.o vprint.o oslib.o list.o stringlib.o \
	   locale.o xmalloc.o \
		 printLine.o
