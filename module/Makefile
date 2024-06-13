GCC=clang
CFLAGS=-g -Wall

threadman.o: threadman.c threadman.h
	$(GCC) $(CFLAGS) -pthread -c -o threadman.o threadman.c -I./
	
libthreadman: threadman.o
	ar rcs libthreadman.a threadman.o

json.o: json.c json.h
	$(GCC) $(CFLAGS) -c -o json.o json.c

libjson: json.o
	ar rcs libjson.a json.o

test: libthreadman net.c
	$(GCC) $(CFLAGS) -pthread -o test net.c -lthreadman -L./ -I./

.PHONY: clean

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf test