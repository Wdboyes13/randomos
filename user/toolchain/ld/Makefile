include ../../../mk/tools.mk

CFLAGS= $(DFLCFLAGS) -I$(USRROOT)/toolchain/libc/include -fno-pie -Wall -Wextra -std=gnu11 -Iinclude/ -DEXT_EZLD_BUILD="\"$(shell date +%y.%m.%d)\""
LDFLAGS := -m elf_x86_64 --no-pie --dynamic-linker=/lib/ld.so --allow-shlib-undefined --hash-style=both $(USRROOT)/toolchain/libc/libc.so  $(USRROOT)/toolchain/libc/crt0.o

EXEC := ld
SRC := $(shell find . -name "*.c")
OBJ := $(SRC:.c=.o)

$(EXEC): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@
	debugfs -w -R "rm /bin/$@" $(DRIVE)
	debugfs -w -R "write $@ /bin/$@" $(DRIVE)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: clean