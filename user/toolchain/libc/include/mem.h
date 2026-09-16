#pragma once
#include <sys/types.h>
#include <fs.h>

#define PAGE_PRESENT  (1ULL << 0)
#define PAGE_WRITE    (1ULL << 1)
#define PAGE_USER     (1ULL << 2)
#define PAGE_HUGE     (1ULL << 7)
#define PAGE_PWT      (1ULL << 3)
#define PAGE_PCD      (1ULL << 4)
#define PAGE_UNCACHE  (PAGE_PCD | PAGE_PWT)

#define MAP_ANYPHYS   (1ULL << 60)
#define MAP_CONT      (1ULL << 61)
#define MAP_ANYVIRT   (1ULL << 62)
#define MAP_USRMAP    (1ULL << 63)

#define UNMAP_KEEPPHYS (1ULL << 0)

#define MAP_ANON 0x0001
#define MAP_ANONYMOUS MAP_ANON
// ignored for now
#define MAP_PRIVATE 0x0002
#define MAP_SHARED 0x0004

#define PROT_NONE  0x01 // ignored
#define PROT_READ  0x02 // ignored
#define PROT_WRITE 0x04
#define PROT_EXEC  0x08 // ignored for now

void* mmap(void* addr, usize len, int prot, int flags, int fd, off_t off);
int munmap(void* addr, usize len);
void* smmap(void* addr, u64 phys, u64 npages, u64 flags);
int smunmap(void* addr, u64 npages, usize flags);
void* sbrk(intptr_t incr);

void* malloc(usize size);
void* realloc(void* ptr, usize size);
void* calloc(usize count, usize size);
void  free(void* ptr);