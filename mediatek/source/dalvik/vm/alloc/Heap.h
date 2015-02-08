
#ifndef DALVIK_ALLOC_HEAP_H_
#define DALVIK_ALLOC_HEAP_H_

struct GcSpec {
  /* If true, only the application heap is threatened. */
  bool isPartial;
  /* If true, the trace is run concurrently with the mutator. */
  bool isConcurrent;
  /* Toggles for the soft reference clearing policy. */
  bool doPreserve;
  /* A name for this garbage collection mode. */
  const char *reason;
};

/* Not enough space for an "ordinary" Object to be allocated. */
extern const GcSpec *GC_FOR_MALLOC;

/* Automatic GC triggered by exceeding a heap occupancy threshold. */
extern const GcSpec *GC_CONCURRENT;

/* Explicit GC via Runtime.gc(), VMRuntime.gc(), or SIGUSR1. */
extern const GcSpec *GC_EXPLICIT;

/* Final attempt to reclaim memory before throwing an OOM. */
extern const GcSpec *GC_BEFORE_OOM;

bool dvmHeapStartup(void);

bool dvmHeapStartupAfterZygote(void);

void dvmHeapShutdown(void);

void dvmHeapThreadShutdown(void);

#if 0       // needs to be in Alloc.h so debug code can find it.
size_t dvmObjectSizeInHeap(const Object *obj);
#endif

void dvmCollectGarbageInternal(const GcSpec *spec);

void dvmWaitForConcurrentGcToComplete(void);

bool dvmIsValidObject(const Object* obj);

#endif  // DALVIK_ALLOC_HEAP_H_
