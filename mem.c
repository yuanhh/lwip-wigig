#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mem.h"

void *mmap_alloc(size_t len)
{
    int fd;
    int result;
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_ANONYMOUS | MAP_SHARED;
    void *addr;

    fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0666);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    result = lseek(fd, len - 1, SEEK_SET);
    if (result == -1) {
        perror("lseek");
        exit(EXIT_FAILURE);
    }
    write(fd, "", 1);

    addr = mmap(NULL, len, protection, visibility, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    close(fd);
    memset(addr, 0, len);
    return addr;
}

void mmap_free(void *addr)
{
    int fd;
    struct stat sb;

    fd = open(FILEPATH, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (fstat(fd, &sb) == -1 || !S_ISREG (sb.st_mode)) {
        perror("fstat");
        exit(EXIT_FAILURE);
    }
    memset(addr, 0, sb.st_size);
    munmap(addr, sb.st_size);
    close(fd);
}
