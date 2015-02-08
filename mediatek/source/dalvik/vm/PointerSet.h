
#ifndef DALVIK_POINTERSET_H_
#define DALVIK_POINTERSET_H_

struct PointerSet;   /* private */

PointerSet* dvmPointerSetAlloc(int initialSize);

void dvmPointerSetFree(PointerSet* pSet);

void dvmPointerSetClear(PointerSet* pSet);

int dvmPointerSetGetCount(const PointerSet* pSet);

const void* dvmPointerSetGetEntry(const PointerSet* pSet, int i);

bool dvmPointerSetAddEntry(PointerSet* pSet, const void* ptr);

bool dvmPointerSetRemoveEntry(PointerSet* pSet, const void* ptr);

bool dvmPointerSetHas(const PointerSet* pSet, const void* ptr, int* pIndex);

INLINE int dvmPointerSetFind(const PointerSet* pSet, const void* ptr) {
    int idx;
    if (!dvmPointerSetHas(pSet, ptr, &idx))
        idx = -1;
    return idx;
}

void dvmPointerSetIntersect(PointerSet* pSet, const void** ptrArray, int count);

void dvmPointerSetDump(const PointerSet* pSet);

#endif  // DALVIK_POINTERSET_H_
