SUBDIRS=kernel

CURDIR=$(shell pwd)

TOOLSDIR=$(CURDIR)/tools

all: boot.img

# NOTE: You need to be root to execute this.
floppy: boot.img
	dd if=boot.img of=/dev/fd0a bs=4k

boot.img: kern
	@echo "Building boot.img"
	head -c 750 /dev/zero > pad
	cat grub/boot/grub/stage1 grub/boot/grub/stage2 pad kernel/kern/kernel.bin > tmp.img
	head -c `wc -c tmp.img | awk '{ print 1474560 - $$1 }'` /dev/zero > pad
	cat tmp.img pad > boot.img
	rm -f pad tmp.img
	@echo ""
	@ls -la kernel/kern/kernel.bin | awk '{print "GRUB Command: kernel 200+" (int($$5 / 512) + 1) }'

kern:
	@for _dir in ${SUBDIRS}; do \
	    cd $$_dir;              \
	    $(MAKE);                \
	done

fetch:
	cd tars; $(MAKE) fetch

tools: fetch
	@if [ ! -d tools ] ; then \
	    mkdir tools;          \
	fi
	cd tars; $(MAKE) PREFIX=$(TOOLSDIR)

clean:
	@for _dir in ${SUBDIRS}; do \
	    cd $$_dir;              \
	    $(MAKE) clean;          \
	done
	cd tars; $(MAKE) clean
	rm -f *.core core pad *.img

spotless: clean
	rm -rf build tools
