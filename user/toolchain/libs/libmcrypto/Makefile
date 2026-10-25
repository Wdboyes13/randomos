include ../../../../mk/tools.mk

CCFLAGS := $(DFLCFLAGS) -I$(USRROOT)/toolchain/libc/include -Wall \
		   -Wextra -Werror -fPIC
LDFLAGS := -shared -Wl,-soname,/lib/libmcrypto.so -Wl,-shared -m64 -nostdlib -ffreestanding -nostartfiles -Wl,--hash-style=both

LIB := libmcrypto.so
LIBSRC := mcrypto.c
LIBOBJ := $(LIBSRC:.c=.o)

DEPS := $(LIBSRC:.c=.d)

$(LIB): $(LIBOBJ)
	$(CC) $(LDFLAGS) $^ -o $@
	debugfs -w -R "rm /lib/$(LIB)" $(DRIVE)
	debugfs -w -R "write $(LIB) /lib/$(LIB)" $(DRIVE)

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
clean:
	rm -f $(LIB) $(LIBOBJ) $(DEPS)

.PHONY: clean
-include $(DEPS)