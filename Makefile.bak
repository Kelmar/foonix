SUBDIRS=kernel

CURDIR=$(shell pwd)

#TOOLSDIR=$(CURDIR)/tools
TOOLSDIR=$(HOME)/.local

.PHONY: all clean kernel libc libc++

all: boot.iso

sysroot: libc libc++ kernel grub/grub.cfg
	mkdir -p sysroot/boot/grub
	mkdir -p sysroot/usr/include
	mkdir -p sysroot/usr/lib
	cd kernel; $(MAKE) install
	cp grub/grub.cfg sysroot/boot/grub

boot.iso: sysroot kernel libc grub/grub.cfg
	@echo "Building boot.iso"
	grub-mkrescue -o boot.iso sysroot

kernel: libc libc++
	cd kernel; $(MAKE)

libc:
	cd libc; $(MAKE)
	cd libc; $(MAKE) install

libc++: libc
	cd libc++; $(MAKE)
	cd libc++; $(MAKE) install

fetch:
	cd tars; $(MAKE) fetch

tools: fetch
	@if [ ! -d tools ] ; then \
	    mkdir tools;          \
	fi
	cd tars; $(MAKE) PREFIX=$(TOOLSDIR)

clean:
	cd kernel; $(MAKE) clean
	cd libc; $(MAKE) clean
	cd libc++; $(MAKE) clean
	rm -rf sysroot
	rm -f *.core core pad *.iso

spotless: clean
	rm -rf build tools
