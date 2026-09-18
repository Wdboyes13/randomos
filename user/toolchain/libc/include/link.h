#pragma once
#include <sys/elf.h>

struct dl_phdr_info {
    Elf64_Addr dlpi_addr;
    const char *dlpi_name;
    const Elf64_Phdr *dlpi_phdr;
    Elf64_Half dlpi_phnum;
    unsigned long long int dlpi_adds;
    unsigned long long int dlpi_subs;
    usize dlpi_tls_modid;
    void *dlpi_tls_data;
};

int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *, usize, void *), void *data);