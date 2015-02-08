
#ifndef DALVIK_SYNC_H_
#define DALVIK_SYNC_H_

#define LW_SHAPE_THIN 0
#define LW_SHAPE_FAT 1
#define LW_SHAPE_MASK 0x1
#define LW_SHAPE(x) ((x) & LW_SHAPE_MASK)

#define LW_HASH_STATE_UNHASHED 0
#define LW_HASH_STATE_HASHED 1
#define LW_HASH_STATE_HASHED_AND_MOVED 3
#define LW_HASH_STATE_MASK 0x3
#define LW_HASH_STATE_SHIFT 1
#define LW_HASH_STATE(x) (((x) >> LW_HASH_STATE_SHIFT) & LW_HASH_STATE_MASK)

#define LW_MONITOR(x) \
  ((Monitor*)((x) & ~((LW_HASH_STATE_MASK << LW_HASH_STATE_SHIFT) | \
                      LW_SHAPE_MASK)))

#define LW_LOCK_OWNER_MASK 0xffff
#define LW_LOCK_OWNER_SHIFT 3
#define LW_LOCK_OWNER(x) (((x) >> LW_LOCK_OWNER_SHIFT) & LW_LOCK_OWNER_MASK)

#define LW_LOCK_COUNT_MASK 0x1fff
#define LW_LOCK_COUNT_SHIFT 19
#define LW_LOCK_COUNT(x) (((x) >> LW_LOCK_COUNT_SHIFT) & LW_LOCK_COUNT_MASK)

struct Object;
struct Monitor;
struct Thread;

#define IS_LOCK_FAT(lock)   (LW_SHAPE(*(lock)) == LW_SHAPE_FAT)

extern "C" void dvmLockObject(Thread* self, Object* obj);

extern "C" bool dvmUnlockObject(Thread* self, Object* obj);

void dvmObjectWait(Thread* self, Object* obj,
    s8 timeout, s4 nanos, bool interruptShouldThrow);
void dvmObjectNotify(Thread* self, Object* obj);
void dvmObjectNotifyAll(Thread* self, Object* obj);

u4 dvmIdentityHashCode(Object* obj);

void dvmThreadSleep(u8 msec, u4 nsec);

void dvmThreadInterrupt(Thread* thread);

/* create a new Monitor struct */
Monitor* dvmCreateMonitor(Object* obj);

void dvmSweepMonitorList(Monitor** mon, int (*isUnmarkedObject)(void*));

/* free monitor list */
void dvmFreeMonitorList(void);

Object* dvmGetMonitorObject(Monitor* mon);

Thread* dvmGetObjectLockHolder(Object* obj);

bool dvmHoldsLock(Thread* thread, Object* obj);

int dvmRelativeCondWait(pthread_cond_t* cond, pthread_mutex_t* mutex,
                         s8 msec, s4 nsec);

#endif  // DALVIK_SYNC_H_
