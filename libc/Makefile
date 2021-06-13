DEFAULT_HOST!=../default-host.sh
HOST?=$(DEFAULT_HOST)
HOSTARCH!=../target-triplet-to-arch.sh $(HOST)

ARCHDIR=arch/$(HOSTARCH)

DESTDIR?=../sysroot
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
INCLUDEDIR?=$(PREFIX)/include
LIBDIR?=$(EXEC_DIR)/lib

include $(ARCHDIR)/make.config

CFLAGS?=-g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

CFLAGS:=$(CFLAGS) -m32 -ffreestanding -Wall -Wextra -Werror -Iinclude 
CPPFLAGS:=$(CPPFLAGS)
LIBK_CFLAGS:=$(CFLAGS)
LIBK_CPPFLAGS:=$(LIBK_CPPFLAGS) -fno-rtti -fno-exceptions

CFLAGS:=$(CFLAGS) -D__is_libc $(ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(ARCH_CPPFLAGS)
LIBK_CFLAGS:=$(LIBK_CFLAGS) -D__is_libk $(KERNEL_ARCH_CFLAGS)
LIBK_CPPFLAGS:=$(LIBK_CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)

LDFLAGS:=$(LDFLAGS) $(ARCH_LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk -lgcc $(ARCH_LIBS)

IND_FREEOBJS=stdio/vsnprintf.o stdio/snprintf.o stdio/vprintf.o stdio/printf.o \
stdio/putchar.o stdio/puts.o \
stdlib/abort.o stdlib/malloc.o stdlib/free.o \
string/memcmp.o string/memcpy.o string/memmove.o string/memset.o string/memchr.o \
string/_s_strncat.o \
string/strlen.o string/strnlen.o string/strncat.o string/strncpy.o

IND_HOSTEDOBJS=

FREEOBJS!=../filter.sh "$(IND_FREEOBJS)" "$(ARCH_FREEOBJS)" "$(ARCHDIR)"

HOSTEDOBJS!=../filter.sh "$(IND_HOSTEDOBJS)" "$(ARCH_HOSTEDOBJS)" "$(ARCHDIR)"

OBJS=$(FREEOBJS) $(HOSTEDOBJS)

LIBK_OBJS=$(FREEOBJS:.o=.libk.o)

#BINARIES=libc.a libk.a
BINARIES=libk.a

.PHONY: all clean install install-headers install-libs
.SUFFIXES: .o .libk.o .c .cpp .S

all: $(BINARIES)

libc.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

.c.o:
	$(CC) -MD -c $< -o $@ --std=gnu11 $(CFLAGS)

.cpp.o:
	$(CC) -MD -c $< -o $@ --std=c++17 $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.c.libk.o:
	$(CC) -MD -c $< -o $@ --std=gnu11 $(LIBK_CFLAGS)

.cpp.libk.o:
	$(CC) -MD -c $< -o $@ --std=c++17 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

.S.libk.o:
	$(CC) -MD -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

clean:
	rm -f $(BINARIES) *.a
	rm -f *.o */*.o */*/*.o */*/*/*.o
	rm -f *.d */*.d */*/*.d */*/*/*.d

install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserv=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)

-include $(OBJS:.o=.d)
-include $(LIBK_OBJS:.o=.d)
