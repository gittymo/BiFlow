GCC=clang
CFLAGS=-g -Wall
SUFFIX=d

LIBJSONDIR=libjson
LIBTHREADMANDIR=libthreadman

APPNAME=biflow
BINDIR=test

debug: debugdeps dirs module

release: BINDIR=bin

release: SUFFIX=

release: releasedeps dirs test module

module: $(APPNAME).c
	$(GCC) $(CFLAGS) -pthread -o $(BINDIR)/$(APPNAME)$(SUFFIX) $(APPNAME).c \
		-I./$(LIBJSONDIR) -I./$(LIBTHREADMANDIR) \
		-L./$(LIBJSONDIR) -L./$(LIBTHREADMANDIR) \
		-ljson$(SUFFIX) -lthreadman$(SUFFIX)

releasedeps:
	cd $(LIBJSONDIR) && make release
	cd $(LIBTHREADMANDIR) && make release

debugdeps:
	cd $(LIBJSONDIR) && make debug
	cd $(LIBTHREADMANDIR) && make debug

clean: dirs
	cd $(LIBJSONDIR) && make clean
	cd $(LIBTHREADMANDIR) && make clean
	rm -rf bin/$(APPNAME)*
	rm -rf test/$(APPNAME)*

.PHONY: dirs

dirs:
	mkdir -p bin
	mkdir -p test