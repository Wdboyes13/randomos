include ../../mk/tools.mk

# compile and link are separate steps and the link goes straight to
# ld.lld, same reason as the kernel makefile: clang would delegate to
# host gcc and every machine interprets that differently
CCFLAGS := $(DFLCFLAGS) \
    -I$(USRROOT)/toolchain/libc/include \
	-I$(USRROOT)/toolchain/libs/libmcrypto -fno-pie \
	-Wall -Wextra -Werror
LDFLAGS := -m elf_x86_64 --no-pie --dynamic-linker=/lib/ld.so --allow-shlib-undefined --hash-style=both

PROGS := init sh echo cat \
		 ls mkfile poweroff \
		 reboot sleep clear \
		 rm wm fbtest dspinfo \
		 wintest keytest mousetest \
		 id whoami serialtest environ \
		 random man rtop ioctest dump \
		 login nl
LIBS := $(USRROOT)/toolchain/libc/libc.so  $(USRROOT)/toolchain/libc/crt0.o

DEPS := $(PROGS:=.d)

PROGFILES := $(PROGS:=.elf)
PROGFILES_C := $(PROGS_C:=.elf)

all: $(PROGFILES)

$(PROGFILES): %.elf: %.c $(LIBS)
	$(CC) $(CCFLAGS) -c $< -o $(basename $@).o
	$(LD) $(LDFLAGS) $(basename $@).o $(LIBS) -o $@
	debugfs -w -R "rm /bin/$$(basename $@ .elf)" $(DRIVE)
	debugfs -w -R "write $@ /bin/$$(basename $@ .elf)" $(DRIVE)

clean:
	rm -f $(PROGFILES) $(PROGS:=.o) $(DEPS) $(DRIVE)

.PHONY: all clean
-include $(DEPS)