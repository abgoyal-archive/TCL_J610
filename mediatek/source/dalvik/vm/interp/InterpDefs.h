
#ifndef DALVIK_INTERP_DEFS_H_
#define DALVIK_INTERP_DEFS_H_

#if defined(WITH_JIT)
#define JIT_CALLEE_SAVE_DOUBLE_COUNT 8

#endif

extern void dvmInterpretPortable(Thread* self);

extern void dvmMterpStd(Thread* self);

Object* dvmGetThisPtr(const Method* method, const u4* fp);

void dvmInterpCheckTrackedRefs(Thread* self, const Method* method,
    int debugTrackedRefStart);

extern "C" s4 dvmInterpHandlePackedSwitch(const u2* switchData, s4 testVal);
extern "C" s4 dvmInterpHandleSparseSwitch(const u2* switchData, s4 testVal);

extern "C" bool dvmInterpHandleFillArrayData(ArrayObject* arrayObject,
                                  const u2* arrayData);

Method* dvmInterpFindInterfaceMethod(ClassObject* thisClass, u4 methodIdx,
    const Method* method, DvmDex* methodClassDex);

static inline bool dvmDebuggerOrProfilerActive()
{
    return gDvm.debuggerActive || gDvm.activeProfilers != 0;
}

#if defined(WITH_JIT)
static inline bool dvmJitDebuggerOrProfilerActive()
{
    return (gDvmJit.pProfTable != NULL) || dvmDebuggerOrProfilerActive();
}

static inline bool dvmJitHideTranslation()
{
    return (gDvm.sumThreadSuspendCount != 0) ||
           (gDvmJit.codeCacheFull == true) ||
           (gDvmJit.pProfTable == NULL);
}

#endif

#endif  // DALVIK_INTERP_DEFS_H_
