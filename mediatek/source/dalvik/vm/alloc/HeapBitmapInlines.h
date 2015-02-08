

#ifndef DALVIK_HEAP_BITMAPINLINES_H_
#define DALVIK_HEAP_BITMAPINLINES_H_

static unsigned long dvmHeapBitmapSetAndReturnObjectBit(HeapBitmap *hb, const void *obj) __attribute__((used));
static void dvmHeapBitmapSetObjectBit(HeapBitmap *hb, const void *obj) __attribute__((used));
static void dvmHeapBitmapClearObjectBit(HeapBitmap *hb, const void *obj) __attribute__((used));

static unsigned long _heapBitmapModifyObjectBit(HeapBitmap *hb, const void *obj,
                                                bool setBit, bool returnOld)
{
    const uintptr_t offset = (uintptr_t)obj - hb->base;
    const size_t index = HB_OFFSET_TO_INDEX(offset);
    const unsigned long mask = HB_OFFSET_TO_MASK(offset);

    assert(hb->bits != NULL);
    assert((uintptr_t)obj >= hb->base);
    assert(index < hb->bitsLen / sizeof(*hb->bits));
    if (setBit) {
        if ((uintptr_t)obj > hb->max) {
            hb->max = (uintptr_t)obj;
        }
        if (returnOld) {
            unsigned long *p = hb->bits + index;
            const unsigned long word = *p;
            *p |= mask;
            return word & mask;
        } else {
            hb->bits[index] |= mask;
        }
    } else {
        hb->bits[index] &= ~mask;
    }
    return false;
}

static unsigned long dvmHeapBitmapSetAndReturnObjectBit(HeapBitmap *hb,
                                                        const void *obj)
{
    return _heapBitmapModifyObjectBit(hb, obj, true, true);
}

static void dvmHeapBitmapSetObjectBit(HeapBitmap *hb, const void *obj)
{
    _heapBitmapModifyObjectBit(hb, obj, true, false);
}

static void dvmHeapBitmapClearObjectBit(HeapBitmap *hb, const void *obj)
{
    _heapBitmapModifyObjectBit(hb, obj, false, false);
}

static unsigned long dvmHeapBitmapIsObjectBitSet(const HeapBitmap *hb,
                                                 const void *obj)
{
    assert(dvmHeapBitmapCoversAddress(hb, obj));
    assert(hb->bits != NULL);
    assert((uintptr_t)obj >= hb->base);
    if ((uintptr_t)obj <= hb->max) {
        const uintptr_t offset = (uintptr_t)obj - hb->base;
        return hb->bits[HB_OFFSET_TO_INDEX(offset)] & HB_OFFSET_TO_MASK(offset);
    } else {
        return 0;
    }
}

#endif  // DALVIK_HEAP_BITMAPINLINES_H_
