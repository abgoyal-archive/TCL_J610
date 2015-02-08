
#ifndef DALVIK_ALLOC_DDMHEAP_H_
#define DALVIK_ALLOC_DDMHEAP_H_

void dvmDdmSendHeapInfo(int reason, bool shouldLock);

void dvmDdmSendHeapSegments(bool shouldLock, bool native);

#endif  // DALVIK_ALLOC_DDMHEAP_H_
