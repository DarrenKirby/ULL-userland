CC ?= gcc
CFLAGS ?= -Wall -Wextra -Wundef -Wshadow -Wpedantic -Wno-unused-parameter -std=gnu2x
LDFLAGS ?=

PKG_CONFIG ?= pkg-config

# Ask pkg-config for the correct flags for ncurses
NCURSES_LIBS := $(shell $(PKG_CONFIG) --libs ncurses 2>/dev/null)

# Fallback if pkg-config fails (rare, but possible)
ifeq ($(NCURSES_LIBS),)
    NCURSES_LIBS := -lncurses
endif

SRCDIR := src
BINDIR := bin

PROGRAMS := basename cal cat cd chgrp chown cp df dirname env false free head link ln ls mount mkdir mv nl od printenv pwd rm rmdir sleep stat sync tail tee touch true uname unlink uptime vdir wc who whoami yes

package := ull-userland
version := 0.3
tarname := $(package)
distdir := $(tarname)-$(version)

# Default target
all: prep $(PROGRAMS)

prep:
	mkdir -p $(BINDIR)

# Build each program
$(PROGRAMS):
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $(SRCDIR)/$@.c $(LDFLAGS_$@) $(LDFLAGS)

# Special link flags per program
LDFLAGS_ls = $(NCURSES_LIBS)
LDFLAGS_nl = -lm

# Tarball distribution
dist: $(distdir).tar.gz

$(distdir).tar.gz: $(distdir)
	tar chof - $(distdir) | gzip -9 -c > $@
	rm -rf $(distdir)

$(distdir): FORCE
	mkdir -p $(distdir)/src
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/doc
	cp Makefile $(distdir)
	cp AUTHORS COPYING INSTALL README TODO $(distdir) || true
	cp src/*.h $(distdir)/src
	cp src/*.c $(distdir)/src

# Housekeeping
clean:
	-rm -rf $(BINDIR)/*

strip:
	strip $(BINDIR)/*

FORCE:

.PHONY: all clean dist strip prep $(PROGRAMS)
