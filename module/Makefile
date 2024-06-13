GCC=clang
CFLAGS=-g -Wall
SUFFIX=d

APPNAME=test

debug: debugdeps

release: releasedeps

releasedeps:
	cd libjson && make release
	cd libthreadman && make release

debugdeps:
	cd libjson && make debug
	cd libthreadman && make debug

.PHONY: clean

clean:
	cd libjson && make clean
	cd libthreadman && make clean