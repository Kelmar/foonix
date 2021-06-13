SRCS=memtest.c
OBJS=memtest.o
LIBS=string.cro
#BINS=memtest kptest
BINS=memtest

STUBS=stubs.o ../string/strnlen.o ../string/strkcat.o

# We override these for our test programs.
TARGET=

CC=gcc
LD=ld
AS=as
OBJCOPY=objcopy
NM=nm

CFLAGS=-g -m32 -D_TEST_

WFLAGS=-Wall -Wextra -Werror -O3 -mmmx
XFLAGS=
IFLAGS=

all: $(BINS)

clean:
	rm -f $(BINS)
	rm -f *.o *.cro *.cro.syms
	rm -f *.core core

.S.o:
	@echo "COMP: $<"
	@$(CC) $(CFLAGS) $(IFLAGS) -o $@ -c $< $(WFLAGS) $(XFLAGS)

.s.o:
	@echo "COMP: $<"
	@$(AS) --32 -o $@ $<

.c.o:
	@echo "COMP: $<"
	@$(CC) $(CFLAGS) $(IFLAGS) -x c -o $@ -c $< $(WFLAGS) $(XFLAGS)

#
# Rename symbols inside of the library so that functions will have a 
# "_foo_" prefix to avoid name colisions with the stdc functions we are.
# testing against.
#
# This idea is taken (more or less) from the BSD crunchgen utility.
#
# TODO: Make this more dynamic (like .c rules above)
#
string.cro: ../string/string.ro
	@echo "LINK: $@"
	@$(NM) -ng ../string/string.ro | awk '/^ *U / { next; } /^[0-9a-fA-F]+ C/ { next; } { print $$3 " _foo_" $$3; next; }' > string.cro.syms
	@$(OBJCOPY) --redefine-syms string.cro.syms ../string/string.ro string.cro

memtest: memtest.o $(LIBS)
	@echo "LINK: $@"
	@$(CC) -m elf_i386 -g -o $@ $@.o $(LIBS)

#kptest: kptest.o ../kern/kprintf.o $(STUBS)
#	@echo "LINK: $@"
#	@$(CC) -g -o $@ $@.o ../kern/kprintf.o $(STUBS)
