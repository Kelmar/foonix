###########################################################################
#
# $Id: Makefile 114 2011-04-10 21:06:43Z kfiresun $ 
#
###########################################################################

SRCS = loader.s cpu.S isr.S display.c keyboard.c kprintf.c desctab.c 
SRCS+= main.c timer.c paging.c kheap.c machine.c hexdump.c klist.c
SRCS+= process.c

# NOTE!  LOADER MUST COME FIRST!
OBJS = loader.o cpu.o isr.o display.o keyboard.o kprintf.o desctab.o
OBJS+= main.o timer.o paging.o kheap.o machine.o hexdump.o klist.o
OBJS+= process.o

LIBS=../string/string.ro

#TARGET=i586-elf-
TARGET=

CPP=$(TARGET)g++
CC=$(TARGET)gcc
LD=$(TARGET)ld
AS=$(TARGET)as
AR=$(TARGET)ar

CFLAGS=-g
CXXFLAGS=-fno-rtti -fno-exceptions

WFLAGS=-Wall -Wextra -Werror
XFLAGS=-nostdlib -nostartfiles -nodefaultlibs
IFLAGS=-I../include

all: build

build: kernel.bin

clean:
	rm -f kernel.bin pad *.img *.o
	rm -f *.core core

.S.o:
	@echo "COMP: $<"
	@$(CC) $(CFLAGS) $(IFLAGS) -o $@ -c $< $(WFLAGS) $(XFLAGS)

.s.o:
	@echo "COMP: $<"
	@$(AS) -o $@ $<

.c.o:
	@echo "COMP: $<"
	@$(CC) $(CFLAGS) $(IFLAGS) -x c -o $@ -c $< $(WFLAGS) $(XFLAGS)

.cpp.o:
	@echo "COMP: $<"
	@$(CPP) $(CFLAGS) $(IFLAGS) -x c++ -o $@ -c $< $(WFLAGS) $(XFLAGS)

kernel.bin: $(OBJS) linker.ld
	@echo "LINK: $@"
	@$(LD) -T linker.ld -o $@ $(OBJS) $(LIBS)
