allfile: allfile.o
	gcc -o allfile allfile.o
	ln allfile gzip.exe

allfile.o: allfile.c crypt.h getopt.h gzip.h lzw.h revision.h tailor.h
	gcc -c -DSTDC_HEADERS=1 -DHAVE_UNISTD_H=1 -DDIRENT=1 -DHAVE_ALLOCA_H=1 ${COMPILE_PARAMETERS} allfile.c

clean:
	rm -f allfile.o allfile allfile.c.inst.exe allfile.int.c gzip.exe

all: clean allfile
build: all
