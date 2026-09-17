CC := clang --target=x86_64-elf
LD := ld.lld

CFLAGS := $(DFLCFLAGS) -Wall -Wextra -O2 -std=c17 \
	-fwrapv -ftrivial-auto-var-init=zero -fno-common \
	-ffunction-sections -fdata-sections -Wno-unused-function \
	-I. -I./include -I./x86 -I./asm -I./disasm \
	-I$(USRROOT)/toolchain/libc/include \
	-I./output -I$(USRROOT)/toolchain/libs/zlib

LDFLAGS := -m elf_x86_64 --no-pie --dynamic-linker=/lib/ld.so --allow-shlib-undefined --hash-style=both
LDLIBS  := -L$(USRROOT)/toolchain/libc -L$(USRROOT)/toolchain/libs/zlib -lc -lz $(USRROOT)/toolchain/libc/crt0.o

SRCS := $(wildcard asm/*.c) \
		$(wildcard common/*.c) \
		$(wildcard nasmlib/*.c) \
		$(wildcard output/*.c) \
		$(wilcard stdlib/*.c) \
		macros/macros.c x86/insnsa.c \
		x86/insnsb.c x86/insnsn.c \
		x86/regs.c x86/regvals.c \
		x86/regflags.c x86/iflag.c

OBJS := $(SRCS:.c=.o)
DEPS := $(SRCS:.c=.d)
OUT := nasm

$(OUT): $(OBJS)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@
	debugfs -w -R "rm /bin/$(OUT)" $(DRIVE)
	debugfs -w -R "write $(OUT) /bin/$(OUT)" $(DRIVE)
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
clean:
	rm -f $(OBJS) $(DEPS) $(OUT)

-include $(DEPS)
.PHONY: clean