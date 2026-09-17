SUBDIRS := toolchain progs

all:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir 'CC=$(CC)' 'LD=$(LD)' 'AS=$(AS)' 'AR=$(AR)' 'NM=$(NM)' 'DRIVE=$(DRIVE)' 'DFLCFLAGS=$(DFLCFLAGS)' USRROOT=$(shell pwd); \
	done

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir 'CC=$(CC)' 'LD=$(LD)' 'AS=$(AS)' 'AR=$(AR)' 'NM=$(NM)' 'DRIVE=$(DRIVE)' 'DFLCFLAGS=$(DFLCFLAGS)' USRROOT=$(shell pwd) clean; \
	done