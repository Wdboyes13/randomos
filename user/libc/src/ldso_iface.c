#include <sys/types.h>

typedef enum {
    LDSO_GETAUXVAL,
    LDSO_LDCLEANUP
} ldso_private_t;
extern void* __ldso_getldsoprivate(ldso_private_t type);
void (*__libc_ldso_ldcleanup)(void) = NULL;

u64 getauxval(u64 type) {
    u64 (*ldso_getauxval)(u64 type) = __ldso_getldsoprivate(LDSO_GETAUXVAL);
    if (ldso_getauxval) {
        return ldso_getauxval(type);
    }
    return 0;
}

void __libc_ldsoinit() {
    __libc_ldso_ldcleanup = __ldso_getldsoprivate(LDSO_LDCLEANUP);
}