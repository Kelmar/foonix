# WARNING: Order is important!
#SUBDIRS=binutils gcc
SUBDIRS=binutils

all: fetch
	@for _dir in ${SUBDIRS}; do	\
	    cd $$_dir;			\
	    $(MAKE) PREFIX=$(PREFIX);	\
	    cd ..;			\
	done

foo:
	@for _dir in ${SUBDIRS}; do	\
	    cd $$_dir;			\
	    $(MAKE) PREFIX=$PREFIX	\
	    cd ..;			\
	done

clean:
	@for _dir in ${SUBDIRS}; do	\
	    cd $$_dir;			\
	    $(MAKE) clean;		\
	    cd ..;			\
	done

spotless: clean
	@for _dir in ${SUBDIRS}; do	\
	    cd $$_dir;			\
	    $(MAKE) spotless;		\
	    cd ..;			\
	done

fetch:
	@for _dir in ${SUBDIRS}; do	\
	    cd $$_dir;			\
	    $(MAKE) fetch;		\
	    cd ..;			\
	done
