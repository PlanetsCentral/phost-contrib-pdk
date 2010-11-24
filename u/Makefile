#
#  Makefile for tests, Unix version
#

CFLAGS = -I.. -O -W -Wall
LDFLAGS = -L.. -lpdk -lm

CDEP = ../*.h
LDEP = ../libpdk.a

ALL = t_stand t_vcr t_sys

all: $(ALL)

clean:
	$(RM) *.o
	$(RM) $(ALL)

test: all
	./t_stand
	./t_vcr
	@echo "*** ok ***"




t_stand: t_stand.o $(LDEP)
	$(CC) -o $@ t_stand.o $(LDFLAGS)

t_stand.o: t_stand.c $(CDEP)


t_sys: t_sys.o $(LDEP)
	$(CC) -o $@ t_sys.o $(LDFLAGS)

t_sys.o: t_sys.c $(CDEP)


t_vcr: t_vcr.o $(LDEP)
	$(CC) -o $@ t_vcr.o $(LDFLAGS)

t_vcr.o: t_vcr.c $(CDEP)

