#ifndef __SHM_H__
#define __SHM_H__

#include <unistd.h>

#define FILEPATH "/tmp/mmapped.bin"

void *mmap_alloc(size_t len);
void mmap_free(void *addr);

#endif
