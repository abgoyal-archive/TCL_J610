
#ifndef DALVIK_ALLOC_ALLOC_H_
#define DALVIK_ALLOC_ALLOC_H_

#include <stddef.h>

/* flags for dvmMalloc */
enum {
    ALLOC_DEFAULT = 0x00,
    ALLOC_DONT_TRACK = 0x01,  /* don't add to internal tracking list */
    ALLOC_NON_MOVING = 0x02,
};

bool dvmGcStartup(void);
bool dvmCreateStockExceptions(void);
bool dvmGcStartupAfterZygote(void);
void dvmGcShutdown(void);
void dvmGcThreadShutdown(void);
bool dvmGcStartupClasses(void);

bool dvmGcPreZygoteFork(void);

void* dvmMalloc(size_t size, int flags);

extern "C" Object* dvmAllocObject(ClassObject* clazz, int flags);

extern "C" void dvmAddTrackedAlloc(Object* obj, Thread* self);

extern "C" void dvmReleaseTrackedAlloc(Object* obj, Thread* self);

bool dvmIsZygoteObject(const Object* obj);

Object* dvmCloneObject(Object* obj, int flags);

extern "C" void dvmSetFinalizable(Object* obj);

size_t dvmObjectSizeInHeap(const Object* obj);

float dvmGetTargetHeapUtilization(void);

void dvmSetTargetHeapUtilization(float newTarget);

void dvmCollectGarbage(void);

size_t dvmCountInstancesOfClass(const ClassObject *clazz);

size_t dvmCountAssignableInstancesOfClass(const ClassObject *clazz);

void dvmClearGrowthLimit(void);

bool dvmIsHeapAddress(void *address);

bool dvmIsNonMovingObject(const Object* object);

#endif  // DALVIK_ALLOC_ALLOC_H_
