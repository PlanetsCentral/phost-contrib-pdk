#
#  Main Makefile for PDK
#
#  This is the main makefile to build the PDK with a GNU toolchain
#  (Linux, other unix, DJGPP). It requires
#  - GNU make
#  - gcc
#  - Perl for the small library
#
#  You can build the normal library as well as a "small" one. See
#  below for docs about the small one.
#
#  Public targets:
#  - all (default, build the PDK and the utilities)
#  - pdk (build just the pdk library)
#  - clean (clean up)
#  - docs (build pdk-api.txt and pdk.inf from pdk.texi)
#  - libpdks.a (build small library)
#
#  This makefile usually is the most up-to-date one. If you port to an
#  operating system not yet supported by the PDK, you should probably
#  start with this one. Please include a date in your new file, so
#  that it can easily be seen when your file is possibly obsolete. 
#  Thanks.
#
#     --Stefan, 07/Aug/2004
#

RANLIB = ranlib

SRCS = auxdata.c pconfig.c crc.c enum.c hostio.c main.c portable.c \
	vgapdata.c vgapmsg.c vgaputil.c worm.c doscvt.c vgapdat1.c \
	vgapdat2.c vgapdat3.c vgapdat4.c vgapdat5.c msgtmp.c \
	rstmsg.c pgetopt.c points.c alldata.c vgapdat6.c vgapdat7.c vgapdat8.c vgapdat9.c \
	vgapdatb.c vgapdatc.c battle.c hullfunc.c listmat.c utildata.c fcode.c commands.c \
	exp.c tons.c
HDRS = battle.h doscvt.h listmat.h phostpdk.h ptscore.h squishio.h version.h \
	conf.h includes.h pgetopt.h private.h shrouds.h stdtyp.h
OBJS = $(SRCS:.c=.o)

SPLITSRC = $(patsubst %.c, split/%.mc, $(SRCS))
SPLITOBJ = $(patsubst %.c, split/%.mo, $(SRCS))

CC = gcc -Wall -O3 -I. -g

all: libpdk.a sendmess crack ptscore killrace pmaster pally

pdk: libpdk.a

sendmess: sendmess.o phostpdk.h libpdk.a
	$(CC) -o sendmess sendmess.o -L. -lpdk -lm

killrace: killrace.o phostpdk.h libpdk.a
	$(CC) -o killrace killrace.o -L. -lpdk -lm

crack: crack.o phostpdk.h libpdk.a
	$(CC) -o crack crack.o -L. -lpdk -lm

ptscore: ptscore.o phostpdk.h libpdk.a
	$(CC) -o ptscore ptscore.o -L. -lpdk -lm

pmaster: pmaster.o phostpdk.h libpdk.a
	$(CC) -o pmaster pmaster.o -L. -lpdk -lm

pally: pally.o phostpdk.h libpdk.a
	$(CC) -o pally pally.o -L. -lpdk -lm

libpdk.a: $(OBJS)
	-rm -f libpdk.a
	ar r libpdk.a $(OBJS)
	$(RANLIB) libpdk.a

clean:
	-rm -f *.o libpdk.a crack sendmess ptscore killrace pmaster *.log

%.o: %.c *.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

############################## Documentation ##############################

docs: pdk.inf pdk-api.txt

pdk.inf: pdk.texi
	makeinfo --no-split -o pdk.inf pdk.texi

pdk-api.txt: pdk.texi
	makeinfo --no-split --no-headers -o pdk-api.txt pdk.texi

############################ The Small Library ############################

# When you use a function from one .c file, usual linkers pull in the
# whole file, not just that function. For example, a program which
# just reads Host data files will also pull in the writing functions,
# because they happen to be in the same .c file. Because manually
# splitting the files apart is not an option (too much work - and do
# you want to deal with 300+ files?), we automate the split. For more
# details, see splitmod.pl. The resulting library (.a) will be much
# larger than the normal one, but the executables will generally be
# much smaller.

# Idea: when the split version of a file is made, we also make a "marker
# file" split/foo.mc. When the snippets are all successfully compiled,
# we make a marker split/foo.mo. This way we don't need to hardwire the
# names of the snippets in the makefile.
# This is supposed to be `echo foo >...', not `touch'; echo also works
# on DOS.

libpdks.a: $(SPLITOBJ)
	-rm -f libpdks.a
	ar r libpdks.a split/*.o
	$(RANLIB) libpdks.a

split/%.mc: split/00 %.c
	-rm -f split/$*_* split/$*.*
	perl splitmod.pl $*.c split/$*_%.c
	echo foo >split/$*.mc

# This rule is split, because we want the wildcard to be expanded
# after generating the .mc file, not when Make reads the makefile.
# Someone please explain to me why we have to call make explicitly
# again instead of relying on dependencies.
split/%.mo: split/%.mc
	$(MAKE) split/$*.mo-helper
	echo foo >split/$*.mo

split/%.mo-helper:
	@$(MAKE) $(patsubst %.c,%.o, $(wildcard split/$*_*.c))

split/00:
	-mkdir split
	echo foo >split/00
