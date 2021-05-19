SUBDIRS=string kern

all:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
	  (cd $$subdir && $(MAKE)) \
	done;

tests:
	(cd test && $(MAKE))

clean:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
	  (cd $$subdir && $(MAKE) clean) \
	done;
	rm -f *.core core
