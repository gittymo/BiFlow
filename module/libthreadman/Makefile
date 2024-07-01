GCC=clang
CFLAGS=-g -Wall
SUFFIX=d

LIBNAME=libthreadman

debug: $(LIBNAME)$(SUFFIX)

release: CFLAGS=-O2

release: SUFFIX=

release: $(LIBNAME)$(SUFFIX)

$(LIBNAME)$(SUFFIX).o: $(LIBNAME).c $(LIBNAME).h
	$(GCC) $(CFLAGS) -c -o $(LIBNAME)$(SUFFIX).o $(LIBNAME).c

$(LIBNAME)$(SUFFIX).a: $(LIBNAME)$(SUFFIX).o
	ar rcs $(LIBNAME)$(SUFFIX).a $(LIBNAME)$(SUFFIX).o

$(LIBNAME)$(SUFFIX): $(LIBNAME)$(SUFFIX).a
	
.PHONY: clean

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf test