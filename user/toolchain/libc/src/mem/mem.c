#include <mem.h>
#include <fs.h>
#include <sys/syscall.h>

void* mmap(void* addr, usize len, int prot, int flags, int fd, off_t off) {
    u64 sflags = 0; // we ignore both PROT_READ and PROT_EXEC for now
                    // PROT_EXECs ignored behaviour should change soon but
                    // for now we don't have NX enabled

    if (len == 0) return (void*)-1;
    /* we only support anonymous mappings; file-backed mmap would silently
     * return zero pages and callers would read nulls instead of file data.
     * NB: dlmalloc redefines PROT_/MAP_ with different values, so accept
     * either ABI here. */
    if (!(flags & (MAP_ANONYMOUS | 0x2)) && fd >= 0) return (void*)-1;

    (void)flags;

    off_t page_off = off & 0xfff;
    // off_t page_base = off & ~0xfffULL;

    u64 npgs = (page_off + len + 4095) / 4096;
    /* accept both libc (PROT_WRITE=0x04) and dlmalloc (PROT_WRITE=2) ABIs */
    if (prot & (PROT_WRITE | 0x2)) sflags |= PAGE_WRITE;
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
extern u64 __uvmm_map_low__;
void* sbrk(intptr_t incr) {
    if (!heap_end) {
        heap_end = (char*)__uvmm_map_low__;
        if (!heap_end) return (void*)-1;
    }
    if (incr < 0) return (void*)-1;
    void* old = heap_end;
    if (incr == 0) return old;
    usize npages = ((usize)incr + 4095) / 4096;
    void* p = mmap(heap_end, npages * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    /* mmap with a fixed hint may still place us elsewhere; only contiguous
     * extension keeps MORECORE_CONTIGUOUS valid */
    if (!p || p == (void*)-1 || p != heap_end) return (void*)-1;
    heap_end += npages * 4096;
    return old;
}