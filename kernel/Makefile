DEFAULT_HOST!=../default-host.sh
HOST?=$(DEFAULT_HOST)
HOSTARCH!=../target-triplet-to-arch.sh $(HOST)

ARCHDIR=arch/$(HOSTARCH)

DESTDIR?=../sysroot
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=/boot
INCLUDEDIR?=$(PREFIX)/include

CFLAGS?=-g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

CFLAGS:=$(CFLAGS) -m32 -ffreestanding -Wall -Wextra -Werror -D__is_kernel -Iinclude
CPPFLAGS:=$(CPPFLAGS) -fno-rtti -fno-exceptions
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk++ -lk -lgcc

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS=$(KERNEL_ARCH_OBJS) kernel/main.o

OBJS=$(ARCHDIR)/crti.o $(ARCHDIR)/crtbegin.o $(KERNEL_OBJS) $(ARCHDIR)/crtend.o $(ARCHDIR)/crtn.o

LINK_LIST=$(LDFLAGS) $(ARCHDIR)/crti.o $(ARCHDIR)/crtbegin.o $(KERNEL_OBJS) $(LIBS) $(ARCHDIR)/crtend.o $(ARCHDIR)/crtn.o

.PHONY: all clean install install-headers install-kernel
.SUFFIXES: .o .c .S

all: foonix.kernel

foonix.kernel: $(OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)
	grub-file --is-x86-multiboot foonix.kernel

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

.cpp.o:
	$(CC) -MD -c $< -o $@ -std=c++17 $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -f foonix.kernel
	rm -f *.o */*.o */*/*.o */*/*/*.o
	rm -f *.d */*.d */*/*.d */*/*/*.d
	rm -f *.core core

install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-kernel:
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp foonix.kernel $(DESTDIR)$(BOOTDIR)

-include $(OBJS:.o=.d)
