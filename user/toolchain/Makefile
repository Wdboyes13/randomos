SUBDIRS := libc libs/libmcrypto libs/zlib nasm
all:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir 'CC=$(CC)' 'LD=$(LD)' 'AS=$(AS)' 'AR=$(AR)' 'NM=$(NM)' 'DRIVE=$(DRIVE)' 'DFLCFLAGS=$(DFLCFLAGS)' USRROOT=$(USRROOT); \
	done

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir 'CC=$(CC)' 'LD=$(LD)' 'AS=$(AS)' 'AR=$(AR)' 'NM=$(NM)' 'DRIVE=$(DRIVE)' 'DFLCFLAGS=$(DFLCFLAGS)' USRROOT=$(shell pwd) clean; \
	done