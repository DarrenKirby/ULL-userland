CC ?= gcc-16

CFLAGS       = -Wall -Wextra -Werror -Wdeprecated-declarations -O2 -std=gnu2x
CFLAGS_DEBUG = -Wall -Wextra -Werror -g -O0 -std=gnu2x -fsanitize=address -fno-omit-frame-pointer
LDFLAGS ?=

PKG_CONFIG ?= pkg-config

SRC_DIR := src
BIN_DIR := bin

# Automatically discover all .c files in src/, but exclude template.c
SRCS     := $(filter-out $(SRC_DIR)/template.c, $(wildcard $(SRC_DIR)/*.c))
PROGRAMS := $(patsubst $(SRC_DIR)/%.c,%,$(SRCS))
BINARIES := $(patsubst %,$(BIN_DIR)/%,$(PROGRAMS))

# Remove proc from non-Linux builds
UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S), Linux)
	SRCS := $(filter-out ps.c, $(SRCS))
	PROGRAMS := $(filter-out ps, $(PROGRAMS))
endif

package := ull-userland
version := 0.4.3
tarname := $(package)
distdir := $(tarname)-$(version)

# Default target
all: prep $(BINARIES)

prep:
	mkdir -p $(BIN_DIR)

# Shared header files
$(BIN_DIR)/sha256sum $(BIN_DIR)/sha512sum $(BIN_DIR)/sha384sum $(BIN_DIR)/sha224sum: $(SRC_DIR)/sha2.h
$(BIN_DIR)/df: $(SRC_DIR)/mount.h
$(BIN_DIR)/ls $(BIN_DIR)/dir $(BIN_DIR)/vdir: $(SRC_DIR)/ls.h

# Provide aliases for running `make df` or `make base32` etc...
.PHONY: $(PROGRAMS)
$(PROGRAMS): %: $(BIN_DIR)/%

# Pattern rule to build binaries from their corresponding .c file.
# The $< variable is the prerequisite (.c file), and $@ is the target (binary).
$(BIN_DIR)/%: $(SRC_DIR)/%.c $(SRC_DIR)/common.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_$*) $(LDFLAGS)

# Special link flags per program
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
	cp AUTHORS COPYING INSTALL README.md TODO.md $(distdir) || true
	cp src/*.h $(distdir)/src
	cp src/*.c $(distdir)/src

# Housekeeping
clean:
	-rm -rf $(BIN_DIR)/*

strip:
	strip $(BIN_DIR)/*

FORCE:

.PHONY: all clean dist strip prep
