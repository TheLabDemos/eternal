#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "mmap_win32.h"

void *mmap(void *start, size_t length, int prot , int flags, int fd, off_t offset) {
	char *buf;

	if(!(buf = malloc(length))) return (void*)MAP_FAILED;

	read(fd, buf, length);

	return buf;
}

int munmap(void *start, size_t length) {
	free(start);
	return 0;
}
