SRCS=memcpy.S memset.S memsetw.S memchr.S strnlen.c strkcat.c strkcpy.c strbswp.c strtrim.c
OBJS=memcpy.o memset.o memsetw.o memchr.o strnlen.o strkcat.o strkcpy.o strbswp.o strtrim.o

#TARGET=i586-elf-
TARGET=

CC=$(TARGET)gcc
LD=$(TARGET)ld
AS=$(TARGET)as
AR=$(TARGET)ar

CFLAGS?=-g

CFLAGS:=$(CFLAGS) -m32
OFLAGS=-Os

WFLAGS=-Wall -Wextra -Werror
XFLAGS=-nostdlib -nostartfiles -nodefaultlibs
IFLAGS=-I../include

all: build

build: string.ro

clean:
	rm -f *.o *.ro *.a
	rm -f *.core core

.S.o:
	@echo "COMP: $<"
	@$(CC) $(CFLAGS) $(IFLAGS) -o $@ -c $< $(WFLAGS) $(XFLAGS)

.s.o:
	@echo "COMP: $<"
	@$(AS) --32 -o $@ $<

.c.o:
	@echo "COMP: $<"
	@$(CC) $(OFLAGS) $(CFLAGS) $(IFLAGS) -x c -o $@ -c $< $(WFLAGS) $(XFLAGS)
	
string.ro: $(OBJS)
	@echo "LINK: $@"
	@$(LD) -m elf_i386 -r -o $@ $(OBJS)
