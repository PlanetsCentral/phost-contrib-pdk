#
#  Makefile for tests, Unix version
#

CFLAGS = -I.. -O -W -Wall
LDFLAGS = -L.. -lpdk -lm

CDEP = ../*.h
LDEP = ../libpdk.a

TESTS = t_stand

all: $(TESTS) t_sys

clean:
	$(RM) *.o
	$(RM) $(TESTS)

test: all
	for i in $(TESTS); do ./$$i || exit 1; done
	@echo "*** ok ***"




t_stand: t_stand.o $(LDEP)
	$(CC) -o $@ t_stand.o $(LDFLAGS)

t_stand.o: t_stand.c $(CDEP)


t_sys: t_sys.o $(LDEP)
	$(CC) -o $@ t_sys.o $(LDFLAGS)

t_sys.o: t_sys.c $(CDEP)

