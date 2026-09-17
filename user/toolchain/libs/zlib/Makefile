CC := clang --target=x86_64-elf
CFLAGS := $(DFLCFLAGS) -I$(USRROOT)/toolchain/libc/include \
		  -Wall -Wextra -Werror -fPIC -I.

LDFLAGS := -shared -Wl,-soname,/lib/libz.so -Wl,-shared -m64 -nostdlib -ffreestanding -nostartfiles -Wl,--hash-style=both

LIB := libz.so
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
DEPS := $(SRCS:.c=.d)

all: $(LIB) copy

$(LIB): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

copy: $(LIB)
	debugfs -w -R "rm /lib/$(LIB)" $(DRIVE)
	debugfs -w -R "write $(LIB) /lib/$(LIB)" $(DRIVE)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(LIB) $(OBJS) $(DEPS)

.PHONY: all copy clean
-include $(DEPS)