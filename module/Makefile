GCC=clang
CFLAGS=-g -Wall
SUFFIX=d

LIBJSONDIR=libjson
LIBTHREADMANDIR=libthreadman

APPNAME=test

debug: debugdeps

release: releasedeps

releasedeps:
	cd $(LIBJSONDIR) && make release
	cd $(LIBTHREADMANDIR) && make release

debugdeps:
	cd $(LIBJSONDIR) && make debug
	cd $(LIBTHREADMANDIR) && make debug

.PHONY: clean

clean:
	cd $(LIBJSONDIR) && make clean
	cd $(LIBTHREADMANDIR) && make clean