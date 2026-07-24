CC ?= cc

CFLAGS       = -Wall -Wextra -Werror -Wdeprecated-declarations -O2 -std=gnu2x
CFLAGS_DEBUG = -Wall -Wextra -Werror -g -O0 -std=gnu2x -fsanitize=address -fno-omit-frame-pointer
LDFLAGS ?=

PKG_CONFIG ?= pkg-config

SRCDIR := src
BINDIR := bin

PROGRAMS := basename cal cat cd chgrp chown cp df dirname env false free head link ln logname ls mount mkdir mv nl od printenv pwd rm rmdir sleep stat sync tail tee touch true uname unlink uptime vdir wc who whoami yes

package := ull-userland
version := 0.4.1
tarname := $(package)
distdir := $(tarname)-$(version)

# Default target
all: prep $(PROGRAMS)

prep:
	mkdir -p $(BINDIR)

# Build each program
$(PROGRAMS):
	$(CC) $(CFLAGS_DEBUG) -o $(BINDIR)/$@ $(SRCDIR)/$@.c $(LDFLAGS_$@) $(LDFLAGS)

# Special link flags per program
LDFLAGS_nl = -lm

# Tarball distribution
dist: $(distdir).tar.gz

$(distdir).tar.gz: $(distdir)
	tar chof - $(distdir)| gzip -9 -c > $@
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
