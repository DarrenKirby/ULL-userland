all: basename cat env false free mv printenv pwd rm sleep true uname uptime wc whoami yes
CC = gcc
CFLAGS =
SRCDIR = ./src
BINDIR = ./bin

basename:
	$(CC) $(CFLAGS) -o $(BINDIR)/basename $(SRCDIR)/basename.c
	strip $(BINDIR)/basename
cat:
	$(CC) $(CFLAGS) -o $(BINDIR)/cat $(SRCDIR)/cat.c
	strip $(BINDIR)/cat
cd:
	$(CC) $(CFLAGS) -o $(BINDIR)/cd $(SRCDIR)/cd.c
	strip $(BINDIR)/cd
env:
	$(CC) $(CFLAGS) -o $(BINDIR)/env $(SRCDIR)/env.c
	strip $(BINDIR)/env
false:
	$(CC) $(CFLAGS) -o $(BINDIR)/false $(SRCDIR)/false.c
	strip $(BINDIR)/false
free:
	$(CC) $(CFLAGS) -o $(BINDIR)/free $(SRCDIR)/free.c
	strip $(BINDIR)/free
mv:
	$(CC) $(CFLAGS) -o $(BINDIR)/mv $(SRCDIR)/mv.c
	strip $(BINDIR)/mv
printenv:
	$(CC) $(CFLAGS) -o $(BINDIR)/printenv $(SRCDIR)/printenv.c
	strip $(BINDIR)/printenv
pwd:
	$(CC) $(CFLAGS) -o $(BINDIR)/pwd $(SRCDIR)/pwd.c
	strip $(BINDIR)/pwd
rm:
	$(CC) $(CFLAGS) -o $(BINDIR)/rm $(SRCDIR)/rm.c
	strip $(BINDIR)/rm
rmdir:
	$(CC) $(CFLAGS) -o $(BINDIR)/rmdir $(SRCDIR)/rmdir.c
	strip $(BINDIR)/rmdir
sleep:
	$(CC) $(CFLAGS) -o $(BINDIR)/sleep $(SRCDIR)/sleep.c
	strip $(BINDIR)/sleep
true:
	$(CC) $(CFLAGS) -o $(BINDIR)/true $(SRCDIR)/true.c
	strip $(BINDIR)/true
uname:
	$(CC) $(CFLAGS) -o $(BINDIR)/uname $(SRCDIR)/uname.c
	strip $(BINDIR)/uname
uptime:
	$(CC) $(CFLAGS) -o $(BINDIR)/uptime $(SRCDIR)/uptime.c
	strip $(BINDIR)/uptime
wc:
	$(CC) $(CFLAGS) -o $(BINDIR)/wc $(SRCDIR)/wc.c
	strip $(BINDIR)/wc
whoami:
	$(CC) $(CFLAGS) -o $(BINDIR)/whoami $(SRCDIR)/whoami.c
	strip $(BINDIR)/whoami
yes:
	$(CC) $(CFLAGS) -o $(BINDIR)/yes $(SRCDIR)/yes.c
	strip $(BINDIR)/yes

clean:
	rm $(BINDIR)/*
