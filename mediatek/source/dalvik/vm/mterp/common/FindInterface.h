

/* common includes */
#include "Dalvik.h"

extern "C" {

INLINE Method* dvmFindInterfaceMethodInCache(ClassObject* thisClass,
    u4 methodIdx, const Method* method, DvmDex* methodClassDex)
{
#define ATOMIC_CACHE_CALC \
    dvmInterpFindInterfaceMethod(thisClass, methodIdx, method, methodClassDex)

    return (Method*) ATOMIC_CACHE_LOOKUP(methodClassDex->pInterfaceCache,
                DEX_INTERFACE_CACHE_SIZE, thisClass, methodIdx);

#undef ATOMIC_CACHE_CALC
}

}
