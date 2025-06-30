SUBDIRS = align taggy_demux

.PHONY: all clean align taggy

align:
	$(MAKE) -C align -f Makefile all; \

taggy:
	$(MAKE) -C taggy_demux -f Makefile all; \

all:
	test -d bin || mkdir -p bin
	for dir in $(SUBDIRS); do \
		 $(MAKE) -C $$dir -f Makefile $@; \
	done

clean:
	for dir in $(SUBDIRS); do \
		 $(MAKE) -C $$dir -f Makefile $@; \
	done
