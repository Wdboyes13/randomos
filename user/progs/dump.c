#include <io.h>
#include <fs.h>
#include <sys/sysfn.h>
#include <str.h>
#include <mem.h>

#define CHUNK 512
int main(int ac, char** av) {
    if (ac < 3) {
        printf("usage: dump <N> <FILE>\n");
        return 1;
    }

    int fd = open(av[2], O_RDONLY, 0);
    if (fd < 0) {
        printf("cannot open %s\n", av[2]);
        return 1;
    }

    char* eptr = NULL;
    long sz = strtol(av[1], &eptr, 10);
    if (*eptr != '\0' || sz < 0) {
        printf("bad number\n");
        return 1;
    }

    char* buf = malloc(sz);
    if (!buf) {
        printf("failed to get buffer\n");
        return 1;
    }

    ssize n = read(fd, buf, sz);
    if (n < 0 || n != sz) {
        printf("read failed\n");
        free(buf);
        return 1;
    }

    for (ssize i = 0; i < sz; i++) {
        printf("%02x ", buf[i]);
    }

    free(buf);
    return 0;
}