GCC=clang
CFLAGS=-g -Wall
SUFFIX=d

release: SUFFIX=

libmem$(SUFFIX).o : libmem.c libmem.h
	$(GCC) $(CFLAGS) -c libmem.c -o libmem$(SUFFIX).o -I./

libmem$(SUFFIX).a : libmem$(SUFFIX).o
	ar rcs libmem$(SUFFIX).a libmem$(SUFFIX).o

debug: libmem$(SUFFIX).a

release: libmem$(SUFFIX).a

test: debug testlibmem.c
	$(GCC) $(CFLAGS) -o testlibmem testlibmem.c -lmem$(SUFFIX) -I./ -L./

.PHONY: clean

clean:
	rm -rf *.o
	rm -rf *.a
	rm testlibmem