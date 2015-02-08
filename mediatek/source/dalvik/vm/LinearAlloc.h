
#ifndef DALVIK_LINEARALLOC_H_
#define DALVIK_LINEARALLOC_H_

#define ENFORCE_READ_ONLY   false

struct LinearAllocHdr {
    int     curOffset;          /* offset where next data goes */
    pthread_mutex_t lock;       /* controls updates to this struct */

    char*   mapAddr;            /* start of mmap()ed region */
    int     mapLength;          /* length of region */
    int     firstOffset;        /* for chasing through */

    short*  writeRefCount;      /* for ENFORCE_READ_ONLY */
};


LinearAllocHdr* dvmLinearAllocCreate(Object* classLoader);

void dvmLinearAllocDestroy(Object* classLoader);

void* dvmLinearAlloc(Object* classLoader, size_t size);

void* dvmLinearRealloc(Object* classLoader, void* mem, size_t newSize);

/* don't call these directly */
void dvmLinearSetReadOnly(Object* classLoader, void* mem);
void dvmLinearSetReadWrite(Object* classLoader, void* mem);

INLINE void dvmLinearReadOnly(Object* classLoader, void* mem)
{
    if (ENFORCE_READ_ONLY && mem != NULL)
        dvmLinearSetReadOnly(classLoader, mem);
}

INLINE void dvmLinearReadWrite(Object* classLoader, void* mem)
{
    if (ENFORCE_READ_ONLY && mem != NULL)
        dvmLinearSetReadWrite(classLoader, mem);
}

void dvmLinearFree(Object* classLoader, void* mem);

char* dvmLinearStrdup(Object* classLoader, const char* str);

void dvmLinearAllocDump(Object* classLoader);

bool dvmLinearAllocContains(const void* start, size_t length);

#endif  // DALVIK_LINEARALLOC_H_
