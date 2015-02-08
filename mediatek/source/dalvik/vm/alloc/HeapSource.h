
#ifndef DALVIK_HEAP_SOURCE_H_
#define DALVIK_HEAP_SOURCE_H_

#include "alloc/Heap.h"
#include "alloc/HeapInternal.h" // for GcHeap

#define HEAP_SOURCE_CHUNK_OVERHEAD         (1 * sizeof (size_t))
#define HEAP_SOURCE_WORST_CHUNK_OVERHEAD   (32 * sizeof (size_t))

#define HEAP_SOURCE_MAX_HEAP_COUNT 2

enum HeapSourceValueSpec {
    HS_FOOTPRINT,
    HS_ALLOWED_FOOTPRINT,
    HS_BYTES_ALLOCATED,
    HS_OBJECTS_ALLOCATED
};

GcHeap *dvmHeapSourceStartup(size_t startingSize,
                             size_t maximumSize,
                             size_t growthLimit);

bool dvmHeapSourceStartupAfterZygote(void);

bool dvmHeapSourceStartupBeforeFork(void);

void dvmHeapSourceThreadShutdown(void);

void dvmHeapSourceShutdown(GcHeap **gcHeap);

void dvmHeapSourceGetRegions(uintptr_t *base, uintptr_t *max, size_t numHeaps);

HeapBitmap *dvmHeapSourceGetLiveBits(void);

HeapBitmap *dvmHeapSourceGetMarkBits(void);

void *dvmHeapSourceGetBase(void);

size_t dvmHeapSourceGetValue(HeapSourceValueSpec spec,
                             size_t perHeapStats[], size_t arrayLen);

void *dvmHeapSourceAlloc(size_t n);

void *dvmHeapSourceAllocAndGrow(size_t n);

size_t dvmHeapSourceFreeList(size_t numPtrs, void **ptrs);

bool dvmHeapSourceContains(const void *ptr);

bool dvmHeapSourceContainsAddress(const void *ptr);

size_t dvmHeapSourceChunkSize(const void *ptr);

size_t dvmHeapSourceFootprint(void);

size_t dvmHeapSourceGetIdealFootprint(void);

void dvmHeapSourceGrowForUtilization(void);

void dvmHeapSourceTrimHeaps();

void dvmHeapSourceWalk(void(*callback)(const void *chunkptr, size_t chunklen,
                                      const void *userptr, size_t userlen,
                                      void *arg),
                       void *arg);
size_t dvmHeapSourceGetNumHeaps(void);

void dvmHeapSourceSwapBitmaps(void);

void dvmHeapSourceZeroMarkBitmap(void);

void dvmMarkImmuneObjects(const char *immuneLimit);

void *dvmHeapSourceGetImmuneLimit(bool isPartial);

size_t dvmHeapSourceGetMaximumSize(void);

bool dvmObjectInHeap(const void *ptr);

#endif  // DALVIK_HEAP_SOURCE_H_
