##########################################################################
#
# Tool chain Makefile common targets.
#
##########################################################################
#
# Loosly based on the FreeBSD ports collection bsd.ports.mk file.
#
# Common variables:
# DISTFILES : (REQUIRED) List of distrobution files to download.
# PATCHFILES: (OPTIONAL) List of patches to apply.
#
# The utility requires that a "sums" directory to exist with a a file
# ending in a ".sums" extension with the same first part as each file
# listed in DISTFILES.  The make file uses this to verify the correctness
# of downloaded files.
#
# A "files" and "work" directory are created.  The files directory holds
# the DISTFILES that are downloaded.  The work directory is where the
# the DISTFILES are unpacked and patched.
#
# If a tool needs to be patched, then the patches must be placed in a
# "patches" directory.  Patches are applied in order they are listed
# in the PATCHFILES variable.
#
##########################################################################

#
# Some things left TODO:
# * Detect if we should use "wget" or "fetch" to get sources.
# * Actually build the targets! >_<
# * Pull prefix and target from environment.
#

##########################################################################

# Change this if you use a different utility. (i.e. "fetch")
FETCH=fetch

TARGET=i586-elf

##########################################################################

all: build

clean:
	rm -rf work

spotless:
	rm -rf files

scratch: work-files
	@if [ ! -d work/build ] ; then					\
	    rm -rf work/build;						\
	    mkdir work/build;						\
	fi

# Unpack and patch tool chain files
work-files: fetch
	@if [ ! -d work ] ; then					\
	    rm -rf work;						\
	    mkdir work;							\
	fi
	@if [ ! -f work/.extract_done ] ; then				\
	    cd work;							\
	    for _file in ${DISTFILES}; do				\
		file=../files/$$_file;					\
		echo "Unpacking: $$_file";				\
		tar -zxf $$file;					\
	    done;							\
	    touch .extract_done;					\
	    rm -f .patch_done;						\
	fi
	@if [ ! -f work/.patch_done ] ; then				\
	    cd work;							\
	    for _patch in ${PATCHFILES}; do				\
		file=../patches/$$_patch;				\
		echo "Applying $$_patch";				\
		patch -p0 < $$file;					\
	    done;							\
	    touch .patch_done;						\
	fi

# Fetch tool chain sources
fetch:
	@if [ ! -d files ] ; then 					\
	    rm -rf files;						\
	    mkdir files;						\
	fi
	@cd files;							\
	for _file in ${DISTFILES}; do					\
	    file=$$_file;						\
	    if [ ! -f $$file ] ; then					\
		$(FETCH) $(PRIMARY_SITE)$$file;				\
	    fi;								\
	done;
#
#	    if ! cksum -c ../sums/$$file.sums ; then			\
#		rm -f $$file;						\
#		$(FETCH) $(PRIMARY_SITE)$$file;				\
#	    fi;								\
#
