all: basename cat cd cp env false free ln mkdir mv printenv pwd rm sleep stat sync true uname uptime wc whoami yes
CC = gcc
CFLAGS = -g -Wall -Wno-unused-variable -O0 -std=gnu99
SRCDIR = ./src
BINDIR = ./bin

basename:
	$(CC) $(CFLAGS) -o $(BINDIR)/basename $(SRCDIR)/basename.c
cat:
	$(CC) $(CFLAGS) -o $(BINDIR)/cat $(SRCDIR)/cat.c
cd:
	$(CC) $(CFLAGS) -o $(BINDIR)/cd $(SRCDIR)/cd.c
cp:
	$(CC) $(CFLAGS) -o $(BINDIR)/cp $(SRCDIR)/cp.c
env:
	$(CC) $(CFLAGS) -o $(BINDIR)/env $(SRCDIR)/env.c
false:
	$(CC) $(CFLAGS) -o $(BINDIR)/false $(SRCDIR)/false.c
free:
	$(CC) $(CFLAGS) -o $(BINDIR)/free $(SRCDIR)/free.c
ln:
	$(CC) $(CFLAGS) -o $(BINDIR)/ln $(SRCDIR)/ln.c
mkdir:
	$(CC) $(CFLAGS) -o $(BINDIR)/mkdir $(SRCDIR)/mkdir.c
mv:
	$(CC) $(CFLAGS) -o $(BINDIR)/mv $(SRCDIR)/mv.c
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
true:
	$(CC) $(CFLAGS) -o $(BINDIR)/true $(SRCDIR)/true.c
uname:
	$(CC) $(CFLAGS) -o $(BINDIR)/uname $(SRCDIR)/uname.c
uptime:
	$(CC) $(CFLAGS) -o $(BINDIR)/uptime $(SRCDIR)/uptime.c
wc:
	$(CC) $(CFLAGS) -o $(BINDIR)/wc $(SRCDIR)/wc.c
whoami:
	$(CC) $(CFLAGS) -o $(BINDIR)/whoami $(SRCDIR)/whoami.c
yes:
	$(CC) $(CFLAGS) -o $(BINDIR)/yes $(SRCDIR)/yes.c

clean:
	- rm $(BINDIR)/*

strip:
	strip $(BINDIR)/*

