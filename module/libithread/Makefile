GCC=clang
CFLAGS=-pthread -g -Wall
SUFFIX=d

SRCDIR=src
LIBDIR=lib
TESTSRC=test
BINDIR=bin
INCDIR=include
OBJDIR=obj

LIBNAME=libithread

SRCFILES=$(wildcard $(SRCDIR)/*.c)

OBJFILES=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o, $(SRCFILES))

debug: dirs $(LIBNAME)$(SUFFIX)

release: CFLAGS=-pthread -O2

release: SUFFIX=

release: dirs $(LIBNAME)$(SUFFIX)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(GCC) $(CFLAGS) -I./$(INCDIR) -c $< -o $@


$(LIBDIR)/$(LIBNAME)$(SUFFIX).a: $(OBJFILES)
	ar rcs $(LIBDIR)/$(LIBNAME)$(SUFFIX).a $(OBJFILES)

$(LIBNAME)$(SUFFIX): $(LIBDIR)/$(LIBNAME)$(SUFFIX).a

.PHONY: clean

clean:
	rm -rf $(BINDIR)/*
	rm -rf $(OBJDIR)/*.o
	rm -rf $(LIBDIR)/*.a

.PHONY: dirs

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
	mkdir -p $(LIBDIR)