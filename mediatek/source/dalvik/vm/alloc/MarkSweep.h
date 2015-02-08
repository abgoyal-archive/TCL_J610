
#ifndef DALVIK_ALLOC_MARK_SWEEP_H_
#define DALVIK_ALLOC_MARK_SWEEP_H_

#include "alloc/HeapBitmap.h"
#include "alloc/HeapSource.h"

struct GcMarkStack {
    /* Highest address (exclusive)
     */
    const Object **limit;

    /* Current top of the stack (exclusive)
     */
    const Object **top;

    /* Lowest address (inclusive)
     */
    const Object **base;

    /* Maximum stack size, in bytes.
     */
    size_t length;
};

struct GcMarkContext {
    HeapBitmap *bitmap;
    GcMarkStack stack;
    const char *immuneLimit;
    const void *finger;   // only used while scanning/recursing.
};

bool dvmHeapBeginMarkStep(bool isPartial);
void dvmHeapMarkRootSet(void);
void dvmHeapReMarkRootSet(void);
void dvmHeapScanMarkedObjects(void);
void dvmHeapReScanMarkedObjects(void);
void dvmHeapProcessReferences(Object **softReferences, bool clearSoftRefs,
                              Object **weakReferences,
                              Object **finalizerReferences,
                              Object **phantomReferences);
void dvmHeapFinishMarkStep(void);
void dvmHeapSweepSystemWeaks(void);
void dvmHeapSweepUnmarkedObjects(bool isPartial, bool isConcurrent,
                                 size_t *numObjects, size_t *numBytes);
void dvmEnqueueClearedReferences(Object **references);

#endif  // DALVIK_ALLOC_MARK_SWEEP_H_
