CC=../../../../Debug+Asserts/bin/clang
LD=../../../../Debug+Asserts/bin/llvm-link
NM=../../../../Debug+Asserts/bin/llvm-nm

CFLAGS= -isystem include -O0 -emit-llvm -g
LDFLAGS=

TEST1= bn_exp.c
TEST1BC=$(TEST1:.c=.bc)

TEST2=$(TEST1)
TEST2+=bn_lib.c
TEST2BC=$(TEST2:.c=.bc)

TEST3=$(TEST2)
TEST3+=bn_mont.c bn_recp.c
TEST3BC=$(TEST3:.c=.bc)

TEST4=$(TEST3)
TEST4+= bn_div.c bn_shift.c bn_sqr.c bn_mul.c bn_mpi.c bn_add.c bn_mod.c bn_word.c
TEST4BC=$(TEST4:.c=.bc)

all: test2.bc

test1.bc: $(TEST1BC)
	$(LD) $(LDFLAGS)  $^ -o $@

test2.bc: $(TEST2BC)
	$(LD) $(LDFLAGS)  $^ -o $@

test3.bc: $(TEST3BC)
	$(LD) $(LDFLAGS)  $^ -o $@

test4.bc: $(TEST4BC)
	$(LD) $(LDFLAGS)  $^ -o $@

%.fn : %.bc
	$(NM) $< > $@

%.bc: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.ll : %.c
	$(CC) $(CFLAGS) -S $<

clean:
	$(RM) *.bc
