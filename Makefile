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
#  You can build the PDK in a different directory. All object files
#  will be dropped in that directory, and the sources will not be
#  touched. To do that, 'cd' into the target directory and do
#  'make -f <DIR>/Makefile SRCDIR=<DIR>'. If you do not specify SRCDIR,
#  this Makefile automatically looks for sources in the current
#  directory, and one or two levels up. This makes it easy to build
#  versions for multiple platforms from a single source tree.
#
#  This makefile usually is the most up-to-date one. If you port to an
#  operating system not yet supported by the PDK, you should probably
#  start with this one. Please include a date in your new file, so
#  that it can easily be seen when your file is possibly obsolete. 
#  Thanks.
#
#     --Stefan, 19/May/2005
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

# Take a wild guess where our sources are.
SRCDIR = $(patsubst %/,%,$(dir $(firstword $(wildcard phostpdk.h ../phostpdk.h ../../phostpdk.h))))

OBJS = $(SRCS:.c=.o)
SPLITSRC = $(patsubst %.c, split/%.mc, $(SRCS))
SPLITOBJ = $(patsubst %.c, split/%.mo, $(SRCS))
EXAMPLES = $(patsubst $(SRCDIR)/%.c, %, $(wildcard $(SRCDIR)/ex[0-9]*.c))

CC = gcc -Wall -O3 -I$(SRCDIR) -g
MAKE += -f $(SRCDIR)/Makefile

# Figure out platform
ifdef DJDIR
PLATFORM=DOS
else
PLATFORM=$(shell uname)
endif
ifeq ($(PLATFORM),BeOS)
LM= 
else
LM=-lm
endif


all: libpdk.a sendmess crack ptscore killrace pmaster pally

pdk: libpdk.a

sendmess: sendmess.o $(SRCDIR)/phostpdk.h libpdk.a
	$(CC) -o sendmess sendmess.o -L. -lpdk $(LM)

killrace: killrace.o $(SRCDIR)/phostpdk.h libpdk.a
	$(CC) -o killrace killrace.o -L. -lpdk $(LM)

crack: crack.o $(SRCDIR)/phostpdk.h libpdk.a
	$(CC) -o crack crack.o -L. -lpdk $(LM)

ptscore: ptscore.o $(SRCDIR)/phostpdk.h libpdk.a
	$(CC) -o ptscore ptscore.o -L. -lpdk $(LM)

pmaster: pmaster.o $(SRCDIR)/phostpdk.h libpdk.a
	$(CC) -o pmaster pmaster.o -L. -lpdk $(LM)

pally: pally.o $(SRCDIR)/phostpdk.h libpdk.a
	$(CC) -o pally pally.o -L. -lpdk $(LM)

libpdk.a: $(OBJS)
	-rm -f libpdk.a
	ar r libpdk.a $(OBJS)
	$(RANLIB) libpdk.a

clean:
	-rm -f *.o libpdk.a libpdks.a crack sendmess ptscore killrace pmaster pally *.log

%.o: $(SRCDIR)/%.c $(SRCDIR)/*.h
	$(CC) -c $(CPPFLAGS) -I$(SRCDIR) $(CFLAGS) -o $*.o $(SRCDIR)/$*.c

################################# Examples ################################

examples: $(EXAMPLES)

$(EXAMPLES): %: %.o libpdk.a
	$(CC) -o $* $*.o -L. -lpdk $(LM)

############################## Documentation ##############################

docs: pdk.inf pdk-api.txt

pdk.inf: $(SRCDIR)/pdk.texi
	makeinfo --no-split -o pdk.inf $(SRCDIR)/pdk.texi

pdk-api.txt: $(SRCDIR)/pdk.texi
	makeinfo --no-split --no-headers -o pdk-api.txt $(SRCDIR)/pdk.texi

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

split/%.mc: split/00 $(SRCDIR)/%.c
	-rm -f split/$*_* split/$*.*
	perl $(SRCDIR)/splitmod.pl $(SRCDIR)/$*.c split/$*_%.c $(SRCDIR)/splitmod.def
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

split/%.o: split/%.c $(SRCDIR)/*.h
	$(CC) -c $(CPPFLAGS) -I$(SRCDIR) $(CFLAGS) -o split/$*.o split/$*.c
