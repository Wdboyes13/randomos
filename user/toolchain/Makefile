SUBDIRS := libc libs/libmcrypto libs/zlib nasm ld
all:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir 'CC=$(CC)' 'LD=$(LD)' 'AS=$(AS)' 'AR=$(AR)' 'NM=$(NM)' 'DRIVE=$(DRIVE)' 'DFLCFLAGS=$(DFLCFLAGS)' USRROOT=$(USRROOT);
	done