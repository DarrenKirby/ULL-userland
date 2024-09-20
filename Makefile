# CC ?= gcc

# hard coded path for testing
CC ?= /usr/local/opt/gcc/bin/gcc-14

# defaults
CC ?= clang
CC ?= gcc

#
CFLAGS ?= -O2 -Wall -Wextra -Wundef -Wshadow -Wpedantic -Wno-unused-parameter -std=gnu11

SRCDIR := src
BINDIR := bin

package := ull-userland
version := 0.3
tarname := $(package)
distdir := $(tarname)-$(version)

all: prep basename cat cd chgrp chown cp df dirname env false free link ln ls mkdir mv printenv \
     pwd rm sleep stat sync tee touch true uname unlink uptime vdir wc who whoami yes

prep:
	mkdir -p $(BINDIR)

dist: $(distdir).tar.gz

$(distdir).tar.gz: $(distdir)
	tar chof - $(distdir) | gzip -9 -c > $@
	rm -rf $(distdir)

$(distdir): FORCE
	mkdir -p $(distdir)/src
	mkdir -p $(distdir)/bin
	mkdir -p $(distdir)/doc
	cp Makefile $(distdir)
	cp AUTHORS COPYING INSTALL README TODO $(distdir)
	cp src/*.h $(distdir)/src
	cp src/*.c $(distdir)/src

basename:
	$(CC) $(CFLAGS) -o $(BINDIR)/basename $(SRCDIR)/basename.c
cat:
	$(CC) $(CFLAGS) -o $(BINDIR)/cat $(SRCDIR)/cat.c
cd:
	$(CC) $(CFLAGS) -o $(BINDIR)/cd $(SRCDIR)/cd.c
chgrp:
	$(CC) $(CFLAGS) -o $(BINDIR)/chgrp $(SRCDIR)/chgrp.c
chown:
	$(CC) $(CFLAGS) -o $(BINDIR)/chown $(SRCDIR)/chown.c
cp:
	$(CC) $(CFLAGS) -o $(BINDIR)/cp $(SRCDIR)/cp.c
df:
	$(CC) $(CFLAGS) -o $(BINDIR)/df $(SRCDIR)/df.c
dirname:
	$(CC) $(CFLAGS) -o $(BINDIR)/dirname $(SRCDIR)/dirname.c
env:
	$(CC) $(CFLAGS) -o $(BINDIR)/env $(SRCDIR)/env.c
false:
	$(CC) $(CFLAGS) -o $(BINDIR)/false $(SRCDIR)/false.c
free:
	$(CC) $(CFLAGS) -o $(BINDIR)/free $(SRCDIR)/free.c
ln:
	$(CC) $(CFLAGS) -o $(BINDIR)/ln $(SRCDIR)/ln.c
link:
	$(CC) $(CFLAGS) -o $(BINDIR)/link $(SRCDIR)/link.c
ls:
	$(CC) $(CFLAGS) -o $(BINDIR)/ls $(SRCDIR)/ls.c -lcurses
mkdir:
	$(CC) $(CFLAGS) -o $(BINDIR)/mkdir $(SRCDIR)/mkdir.c
mount:
	$(CC) $(CFLAGS) -o $(BINDIR)/mount $(SRCDIR)/mount.c
mv:
	$(CC) $(CFLAGS) -o $(BINDIR)/mv $(SRCDIR)/mv.c
od:
	$(CC) $(CFLAGS) -o $(BINDIR)/od $(SRCDIR)/od.c
printenv:
	$(CC) $(CFLAGS) -o $(BINDIR)/printenv $(SRCDIR)/printenv.c
pwd:
	$(CC) $(CFLAGS) -o $(BINDIR)/pwd $(SRCDIR)/pwd.c
rm:
	$(CC) $(CFLAGS) -o $(BINDIR)/rm $(SRCDIR)/rm.c
rmdir:
	$(CC) $(CFLAGS) -o $(BINDIR)/rmdir $(SRCDIR)/rmdir.c
sleep:
	$(CC) $(CFLAGS) -o $(BINDIR)/sleep $(SRCDIR)/sleep.c
stat:
	$(CC) $(CFLAGS) -o $(BINDIR)/stat $(SRCDIR)/stat.c
sync:
	$(CC) $(CFLAGS) -o $(BINDIR)/sync $(SRCDIR)/sync.c
tee:
	$(CC) $(CFLAGS) -o $(BINDIR)/tee $(SRCDIR)/tee.c
touch:
	$(CC) $(CFLAGS) -o $(BINDIR)/touch $(SRCDIR)/touch.c
true:
	$(CC) $(CFLAGS) -o $(BINDIR)/true $(SRCDIR)/true.c
uname:
	$(CC) $(CFLAGS) -o $(BINDIR)/uname $(SRCDIR)/uname.c
unlink:
	$(CC) $(CFLAGS) -o $(BINDIR)/unlink $(SRCDIR)/unlink.c
uptime:
	$(CC) $(CFLAGS) -o $(BINDIR)/uptime $(SRCDIR)/uptime.c
vdir:
	$(CC) $(CFLAGS) -o $(BINDIR)/vdir $(SRCDIR)/vdir.c
wc:
	$(CC) $(CFLAGS) -o $(BINDIR)/wc $(SRCDIR)/wc.c
who:
	$(CC) $(CFLAGS) -o $(BINDIR)/who $(SRCDIR)/who.c
whoami:
	$(CC) $(CFLAGS) -o $(BINDIR)/whoami $(SRCDIR)/whoami.c
yes:
	$(CC) $(CFLAGS) -o $(BINDIR)/yes $(SRCDIR)/yes.c

clean:
	-rm -rf $(BINDIR)/*

strip:
	strip $(BINDIR)/*

FORCE:
	-rm $(distdir).tar.gz >/dev/null 2>&1
	-rm -rf $(distdir) >/dev/null 2>&1

.PHONEY: all clean dist strip
