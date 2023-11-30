allfile: allfile.o
	gcc -o allfile allfile.o
#	rm -f FaultSeeds.h
	ln allfile gzip.exe

allfile.o: allfile.c crypt.h getopt.h gzip.h lzw.h revision.h tailor.h config.h
	touch FaultSeeds.h
	gcc -DHAVE_CONFIG_H ${COMPILE_PARAMETERS} -I. -c allfile.c

clean:
	rm -f allfile.o allfile allfile.c.inst.exe allfile.int.c gzip.exe

all: clean allfile
build: all
