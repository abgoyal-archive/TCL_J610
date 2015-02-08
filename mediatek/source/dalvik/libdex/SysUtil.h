

#ifndef LIBDEX_SYSUTIL_H_
#define LIBDEX_SYSUTIL_H_

#include <sys/types.h>

#define SYSTEM_PAGE_SIZE        4096

struct MemMapping {
    void*   addr;           /* start of data */
    size_t  length;         /* length of data */

    void*   baseAddr;       /* page-aligned base address */
    size_t  baseLength;     /* length of mapping */
};

void sysCopyMap(MemMapping* dst, const MemMapping* src);

int sysLoadFileInShmem(int fd, MemMapping* pMap);

int sysMapFileInShmemReadOnly(int fd, MemMapping* pMap);

int sysMapFileInShmemWritableReadOnly(int fd, MemMapping* pMap);

int sysMapFileSegmentInShmem(int fd, off_t start, size_t length,
    MemMapping* pMap);

int sysCreatePrivateMap(size_t length, MemMapping* pMap);

int sysChangeMapAccess(void* addr, size_t length, int wantReadWrite,
    MemMapping* pmap);

void sysReleaseShmem(MemMapping* pMap);

int sysWriteFully(int fd, const void* buf, size_t count, const char* logMsg);

int sysCopyFileToFile(int outFd, int inFd, size_t count);

#endif  // LIBDEX_SYSUTIL_H_
