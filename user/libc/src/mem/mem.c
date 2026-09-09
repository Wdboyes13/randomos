#include <mem.h>
#include <fs.h>
#include <sys/syscall.h>

void* mmap(void* addr, usize len, int prot, int flags, int fd, off_t off) {
    u64 sflags = 0; // we ignore both PROT_READ and PROT_EXEC for now
                    // PROT_EXECs ignored behaviour should change soon but
                    // for now we don't have NX enabled

    (void)flags;(void)fd;

    off_t page_off = off & 0xfff;
    // off_t page_base = off & ~0xfffULL;

    u64 npgs = (page_off + len + 4095) / 4096;
    if (prot & PROT_WRITE) sflags |= PAGE_WRITE;
    // kernel does MAP_ANYPHYS|MAP_USRMAP|PAGE_USER anyways on all mmap syscalls but just do it manually
    // just in case

    if (addr) {
        return (char*)smmap(addr, 0, npgs, MAP_ANYPHYS | MAP_USRMAP | PAGE_USER | sflags) + page_off;
    } else {
        return (char*)smmap(NULL, 0, npgs, MAP_ANYPHYS | MAP_USRMAP | PAGE_USER | MAP_ANYVIRT | sflags) + page_off;
    }
}

int munmap(void* addr, usize len) {
    u64 npgs = (len + 4095) / 4096;
    return smunmap(addr, npgs, 0);
}

void* smmap(void* addr, u64 phys, u64 npages, u64 flags) {
    return (void*)__syscall4(SYS_MMAP, (u64)addr, phys, npages, flags);
}

int smunmap(void* addr, u64 npages, usize flags) {
    return __syscall3(SYS_MUNMAP, (u64)addr, npages, flags);
}

static char* heap_end;
void* sbrk(intptr_t incr) {
    if (incr < 0) return (void*)-1;
    void* old = heap_end;
    if (incr == 0) return old;
    usize npages = (incr + 4095) / 4096;
    void* p = mmap(heap_end, npages * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (!p || p != heap_end) return (void*)-1;
    heap_end += npages * 4096;
    return old;
}