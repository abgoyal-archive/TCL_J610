
#ifndef DALVIK_ALLOC_HEAP_INTERNAL_H_
#define DALVIK_ALLOC_HEAP_INTERNAL_H_

#include "MarkSweep.h"

struct HeapSource;

struct GcHeap {
    HeapSource *heapSource;

    /* Linked lists of subclass instances of java/lang/ref/Reference
     * that we find while recursing.  The "next" pointers are hidden
     * in the Reference objects' pendingNext fields.  These lists are
     * cleared and rebuilt each time the GC runs.
     */
    Object *softReferences;
    Object *weakReferences;
    Object *finalizerReferences;
    Object *phantomReferences;

    /* The list of Reference objects that need to be enqueued.
     */
    Object *clearedReferences;

    /* The current state of the mark step.
     * Only valid during a GC.
     */
    GcMarkContext markContext;

    /* GC's card table */
    u1* cardTableBase;
    size_t cardTableLength;
    size_t cardTableMaxLength;
    size_t cardTableOffset;

    /* Is the GC running?  Used to avoid recursive calls to GC.
     */
    bool gcRunning;

    /*
     * Debug control values
     */

    int ddmHpifWhen;
    int ddmHpsgWhen;
    int ddmHpsgWhat;
    int ddmNhsgWhen;
    int ddmNhsgWhat;
};

bool dvmLockHeap(void);
void dvmUnlockHeap(void);


#define HEAP_LOG_TAG      LOG_TAG "-heap"

#if LOG_NDEBUG
#define LOGV_HEAP(...)    ((void)0)
#define LOGD_HEAP(...)    ((void)0)
#else
#define LOGV_HEAP(...)    LOG(LOG_VERBOSE, HEAP_LOG_TAG, __VA_ARGS__)
#define LOGD_HEAP(...)    LOG(LOG_DEBUG, HEAP_LOG_TAG, __VA_ARGS__)
#endif
#define LOGI_HEAP(...) \
    do { \
        if (!gDvm.zygote) { LOG(LOG_INFO, HEAP_LOG_TAG, __VA_ARGS__); } \
    } while (0)

#define LOGW_HEAP(...)    LOG(LOG_WARN, HEAP_LOG_TAG, __VA_ARGS__)
#define LOGE_HEAP(...)    LOG(LOG_ERROR, HEAP_LOG_TAG, __VA_ARGS__)

#define FRACTIONAL_MB(n)    (n) / (1024 * 1024), \
                            ((((n) % (1024 * 1024)) / 1024) * 1000) / 1024
#define FRACTIONAL_PCT(n,max)    ((n) * 100) / (max), \
                                 (((n) * 1000) / (max)) % 10

#endif  // DALVIK_ALLOC_HEAP_INTERNAL_H_
