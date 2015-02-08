

#ifndef DALVIK_ATOMIC_H_
#define DALVIK_ATOMIC_H_

#include <cutils/atomic.h>          /* use common Android atomic ops */
#include <cutils/atomic-inline.h>   /* and some uncommon ones */


extern "C" int64_t dvmQuasiAtomicSwap64(int64_t value, volatile int64_t* addr);

extern "C" int64_t dvmQuasiAtomicSwap64Sync(int64_t value,
                                            volatile int64_t* addr);

extern "C" int64_t dvmQuasiAtomicRead64(volatile const int64_t* addr);

int dvmQuasiAtomicCas64(int64_t oldvalue, int64_t newvalue,
        volatile int64_t* addr);

#endif  // DALVIK_ATOMIC_H_
