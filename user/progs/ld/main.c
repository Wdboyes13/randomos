#include <stdio.h>
#include <io.h>
#include <sys/elf.h>
#include <mem.h>
#include <fs.h>
#include <sys/sysfn.h>

typedef struct {
    Elf64_Ehdr ehdr;
    Elf64_Shdr* shdrs;
    int fd;
} ldobj_t;

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

        ssize n = read(obj->fd, &objs->ehdr, sizeof(objs->ehdr));       
        if (n < 0) {
            fprintf(stderr, "Failed to read ELF header\n");
            for (usize j = 0; j < i; j++) {
                free(objs[i].shdrs);
                close(objs[i].fd);
            }
            free(objs);
            return NULL;
        }

        if (objs->ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
            objs->ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
            objs->ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
            objs->ehdr.e_ident[EI_MAG3] != ELFMAG3 ||
            objs->ehdr.e_ident[EI_CLASS] != ELFCLASS64 ||
            objs->ehdr.e_ident[EI_DATA] != ELFDATA2LSB ||
            objs->ehdr.e_type != ET_REL ||
            objs->ehdr.e_machine != EM_X86_64 ||
            objs->ehdr.e_version != EV_CURRENT ||
            objs->ehdr.e_shnum == 0) {
                fprintf(stderr, "Invalid or unsupported object\n");
                for (usize j = 0; j < i; j++) {
                    free(objs[i].shdrs);
                    close(objs[i].fd);
                }
                free(objs);
                return NULL;
        }

        if (lseek(objs->fd, objs->ehdr.e_shoff, SEEK_SET) < 0) {
            fprintf(stderr, "Failed to get section headers\n");
            for (usize j = 0; j < i; j++) {
                free(objs[i].shdrs);
                close(objs[i].fd);
            }
            free(objs);
            return NULL;
        }

        usize shdrs_sz = sizeof(*objs->shdrs) * obj->ehdr.e_shnum;
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

        
        if ((n = read(objs->fd, obj->shdrs, shdrs_sz)) < 0 || (usize)n < shdrs_sz) {
            fprintf(stderr, "Failed to read section headers\n");
            for (usize j = 0; j < i; j++) {
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
    return 0;
}