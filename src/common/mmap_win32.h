#ifndef _MMAP_WIN32_H_
#define _MMAP_WIN32_H_

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAP_FAILED	(void*)(-1)
#define open	_open
#define close	_close
#define read	_read
#define write	_write
#define lseek	_lseek
#define tell	_tell

#define PROT_READ	0
#define PROT_WRITE	0
#define MAP_PRIVATE	0

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

void *mmap(void *start, size_t length, int prot , int flags, int fd, off_t offset);
int munmap(void *start, size_t length);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _MMAP_WIN32_H_ */