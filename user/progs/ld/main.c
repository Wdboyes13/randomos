#define STB_DS_IMPLEMENTATION
#include <stdio.h>
#include <io.h>
#include <sys/elf.h>
#include <mem.h>
#include <fs.h>
#include <sys/sysfn.h>
#include "stb_ds.h"

typedef struct {
    Elf64_Ehdr ehdr;
    Elf64_Shdr* shdrs;
    char* shstrtab;
    int fd;
} ldobj_t;

typedef struct {
    ldobj_t* first;
    Elf64_Shdr* fshdr;
} sectent;

typedef struct {
    char* key;
    sectent value;
} sectmap;

ldobj_t* parse_objs(char** objps, usize nobjs) {
    ldobj_t* objs = malloc(sizeof(*objs) * nobjs);
    if (!objs) {
        fprintf(stderr, "failed to allocate object metadata\n");
        return NULL;
    }

    for (usize i = 0; i < nobjs; i++) {
        ldobj_t* obj = &objs[i]; 
        obj->fd = open(objps[i], O_RDONLY, 0);
        if (obj->fd < 0) {
            fprintf(stderr, "failed to open file \"%s\"\n", objs[i]);
            for (usize j = 0; j < i; j++) {
                free(objs[i].shdrs);
                close(objs[i].fd);
            }
            free(objs);
            return NULL;
        }

        ssize n = read(obj->fd, &obj->ehdr, sizeof(obj->ehdr));       
        if (n < 0) {
            fprintf(stderr, "Failed to read ELF header\n");
            for (usize j = 0; j < i; j++) {
                free(objs[i].shdrs);
                close(objs[i].fd);
            }
            free(objs);
            return NULL;
        }

        if (obj->ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
            obj->ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
            obj->ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
            obj->ehdr.e_ident[EI_MAG3] != ELFMAG3 ||
            obj->ehdr.e_ident[EI_CLASS] != ELFCLASS64 ||
            obj->ehdr.e_ident[EI_DATA] != ELFDATA2LSB ||
            obj->ehdr.e_type != ET_REL ||
            obj->ehdr.e_machine != EM_X86_64 ||
            obj->ehdr.e_version != EV_CURRENT ||
            obj->ehdr.e_shnum == 0) {
                fprintf(stderr, "Invalid or unsupported object\n");
                for (usize j = 0; j < i; j++) {
                    free(objs[i].shdrs);
                    close(objs[i].fd);
                }
                free(objs);
                return NULL;
        }

        if (lseek(obj->fd, obj->ehdr.e_shoff, SEEK_SET) < 0) {
            fprintf(stderr, "Failed to get section headers\n");
            for (usize j = 0; j < i; j++) {
                free(objs[i].shdrs);
                close(objs[i].fd);
            }
            free(objs);
            return NULL;
        }

        usize shdrs_sz = sizeof(*obj->shdrs) * obj->ehdr.e_shnum;
        obj->shdrs = malloc(shdrs_sz);
        if (!obj->shdrs) {
            fprintf(stderr, "Failed to allocate section headers\n");
            for (usize j = 0; j < i; j++) {
                free(objs[i].shdrs);
                close(objs[i].fd);
            }
            free(objs);
            return NULL;
        }

        
        if ((n = read(obj->fd, obj->shdrs, shdrs_sz)) < 0 || (usize)n < shdrs_sz) {
            fprintf(stderr, "Failed to read section headers\n");
            for (usize j = 0; j < i; j++) {
                free(objs[i].shdrs);
                close(objs[i].fd);
            }
            free(objs);
            return NULL;
        }

        Elf64_Shdr* shstrtab_shdr = &obj->shdrs[obj->ehdr.e_shstrndx];
        
        if (lseek(obj->fd, shstrtab_shdr->sh_offset, SEEK_SET) < 0) {
            fprintf(stderr, "Failed to get section header string table\n");
            for (usize j = 0; j < i; j++) {
                free(objs[i].shdrs);
                close(objs[i].fd);
            }
            free(objs);
            return NULL;
        }

        obj->shstrtab = malloc(shstrtab_shdr->sh_size);
        if (!obj->shstrtab) {
            fprintf(stderr, "Failed to allocate section header string table\n");
            for (usize j = 0; j < i; j++) {
                free(objs[i].shdrs);
                close(objs[i].fd);
            }
            free(objs);
            return NULL;
        }

        if ((n = read(obj->fd, obj->shstrtab, shstrtab_shdr->sh_size)) < 0 || (usize)n < shdrs_sz) {
            fprintf(stderr, "Failed to read section header string table\n");
            for (usize j = 0; j < i; j++) {
                free(objs[i].shstrtab);
                free(objs[i].shdrs);
                close(objs[i].fd);
            }
            free(objs);
            return NULL;
        }
    }

    return objs;
}

int main(int ac, char** av) {
    if (ac < 2) {
        fprintf(stderr, "please provide at least 1 object file to link\n");
        return 1;
    }

    usize nobjs = ac-1;
    ldobj_t* objs = parse_objs(av+1, nobjs);
    if (!objs) {
        return 1;
    }

    // next we should look through all section headers, see what we need to merge
    // perform merges (saving old information when needed)
    // then update relavent addresses such as entry points, relocation addresses, and symbol tables
    // after that we should construct phdrs and the ehdr
    // then we should finally emit our binary
    // also see the TODO file in the folder for a different way of saying this
    // since that one was much better though out i think lol

    sectmap* smap = NULL;
    for (usize i = 0; i < nobjs; i++) {
        ldobj_t* obj = &objs[i];
        for (usize i = 0; i < obj->ehdr.e_shnum; i++) {
            if (shgeti(smap, &obj->shstrtab[obj->shdrs[i].sh_name]) < 0) {
                sectent s = {obj, &obj->shdrs[i]};
                shput(smap, &obj->shstrtab[obj->shdrs[i].sh_name], s);
            }
        }
    }
    return 0;
}