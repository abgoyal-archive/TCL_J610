
#ifndef DALVIK_HEAPDEBUG_H_
#define DALVIK_HEAPDEBUG_H_

enum HeapDebugInfoType {
    kVirtualHeapSize = 0,
    kNativeHeapSize = 1,
    kVirtualHeapAllocated = 2,
    kNativeHeapAllocated = 3,
    kVirtualHeapMaximumSize = 4
};

int dvmGetHeapDebugInfo(HeapDebugInfoType info);

#endif  // DALVIK_HEAPDEBUG_H_
