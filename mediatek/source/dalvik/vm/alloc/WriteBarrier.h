

#ifndef DALVIK_ALLOC_WRITEBARRIER_H_
#define DALVIK_ALLOC_WRITEBARRIER_H_


INLINE void dvmWriteBarrierField(const Object *obj, void *addr)
{
    dvmMarkCard(obj);
}

INLINE void dvmWriteBarrierObject(const Object *obj)
{
    dvmMarkCard(obj);
}

INLINE void dvmWriteBarrierArray(const ArrayObject *obj,
                                 size_t start, size_t end)
{
    dvmMarkCard((Object *)obj);
}

#endif  // DALVIK_ALLOC_WRITEBARRIER_H_
