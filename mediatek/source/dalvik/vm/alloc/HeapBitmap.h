
#ifndef DALVIK_HEAP_BITMAP_H_
#define DALVIK_HEAP_BITMAP_H_

#include <limits.h>
#include <stdint.h>

#define HB_OBJECT_ALIGNMENT 8
#define HB_BITS_PER_WORD (sizeof(unsigned long) * CHAR_BIT)

#define HB_OFFSET_TO_INDEX(offset_) \
    ((uintptr_t)(offset_) / HB_OBJECT_ALIGNMENT / HB_BITS_PER_WORD)
#define HB_INDEX_TO_OFFSET(index_) \
    ((uintptr_t)(index_) * HB_OBJECT_ALIGNMENT * HB_BITS_PER_WORD)

#define HB_OFFSET_TO_BYTE_INDEX(offset_) \
  (HB_OFFSET_TO_INDEX(offset_) * sizeof(*((HeapBitmap *)0)->bits))

#define HB_OFFSET_TO_MASK(offset_) \
    (1 << \
        (31-(((uintptr_t)(offset_) / HB_OBJECT_ALIGNMENT) % HB_BITS_PER_WORD)))

struct HeapBitmap {
    /* The bitmap data, which points to an mmap()ed area of zeroed
     * anonymous memory.
     */
    unsigned long *bits;

    /* The size of the used memory pointed to by bits, in bytes.  This
     * value changes when the bitmap is shrunk.
     */
    size_t bitsLen;

    /* The real size of the memory pointed to by bits.  This is the
     * number of bytes we requested from the allocator and does not
     * change.
     */
    size_t allocLen;

    /* The base address, which corresponds to the first bit in
     * the bitmap.
     */
    uintptr_t base;

    /* The highest pointer value ever returned by an allocation
     * from this heap.  I.e., the highest address that may correspond
     * to a set bit.  If there are no bits set, (max < base).
     */
    uintptr_t max;
};

typedef void BitmapCallback(Object *obj, void *arg);
typedef void BitmapScanCallback(Object *obj, void *finger, void *arg);
typedef void BitmapSweepCallback(size_t numPtrs, void **ptrs, void *arg);

bool dvmHeapBitmapInit(HeapBitmap *hb, const void *base, size_t maxSize,
        const char *name);

void dvmHeapBitmapDelete(HeapBitmap *hb);

void dvmHeapBitmapZero(HeapBitmap *hb);

bool dvmHeapBitmapCoversAddress(const HeapBitmap *hb, const void *obj);

void dvmHeapBitmapWalk(const HeapBitmap *bitmap,
                       BitmapCallback *callback, void *callbackArg);

void dvmHeapBitmapScanWalk(HeapBitmap *bitmap,
                           BitmapScanCallback *callback, void *arg);

void dvmHeapBitmapSweepWalk(const HeapBitmap *liveHb, const HeapBitmap *markHb,
                            uintptr_t base, uintptr_t max,
                            BitmapSweepCallback *callback, void *callbackArg);

#endif  // DALVIK_HEAP_BITMAP_H_
