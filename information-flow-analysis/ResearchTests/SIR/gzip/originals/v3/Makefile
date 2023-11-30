allfile: allfile.o
	gcc -o allfile allfile.o

allfile.o: allfile.c crypt.h getopt.h gzip.h lzw.h revision.h tailor.h
	gcc -c -DSTDC_HEADERS=1 -DHAVE_UNISTD_H=1 -DDIRENT=1 -DHAVE_ALLOCA_H=1 allfile.c

clean:
	rm -f allfile.o

all: clean allfile
