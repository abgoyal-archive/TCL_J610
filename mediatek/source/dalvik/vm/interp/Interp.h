

#ifndef DALVIK_INTERP_INTERP_H_
#define DALVIK_INTERP_INTERP_H_

INLINE void dvmExportPC(const u2* pc, const u4* fp)
{
    SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc;
}

#define GET_OPCODE(_inst) (((_inst & 0xff) == OP_DISPATCH_FF) ? \
                           (0x100 + ((_inst >> 8) & 0xff)) : (_inst & 0xff))

void dvmInterpret(Thread* thread, const Method* method, JValue* pResult);

extern "C" void dvmThrowVerificationError(const Method* method,
                                          int kind, int ref);

bool dvmBreakpointStartup(void);
void dvmBreakpointShutdown(void);
void dvmInitInterpreterState(Thread* self);

void dvmInitBreakpoints();
void dvmAddBreakAddr(Method* method, unsigned int instrOffset);
void dvmClearBreakAddr(Method* method, unsigned int instrOffset);
bool dvmAddSingleStep(Thread* thread, int size, int depth);
void dvmClearSingleStep(Thread* thread);

extern "C" u1 dvmGetOriginalOpcode(const u2* addr);

void dvmFlushBreakpoints(ClassObject* clazz);

extern "C" void dvmCheckBefore(const u2 *dPC, u4 *fp, Thread* self);
extern "C" void dvmReportExceptionThrow(Thread* self, Object* exception);
extern "C" void dvmReportPreNativeInvoke(const Method* methodToCall, Thread* self, u4* fp);
extern "C" void dvmReportPostNativeInvoke(const Method* methodToCall, Thread* self, u4* fp);
extern "C" void dvmReportInvoke(Thread* self, const Method* methodToCall);
extern "C" void dvmReportReturn(Thread* self);

void dvmDisableSubMode(Thread* thread, ExecutionSubModes subMode);
extern "C" void dvmEnableSubMode(Thread* thread, ExecutionSubModes subMode);
void dvmDisableAllSubMode(ExecutionSubModes subMode);
void dvmEnableAllSubMode(ExecutionSubModes subMode);
void dvmAddToSuspendCounts(Thread* thread, int delta, int dbgDelta);
void dvmCheckInterpStateConsistency();
void dvmInitializeInterpBreak(Thread* thread);

void dvmArmSafePointCallback(Thread* thread, SafePointCallback funct,
                             void* arg);


#ifndef DVM_NO_ASM_INTERP
extern void* dvmAsmInstructionStart[];
extern void* dvmAsmAltInstructionStart[];
#endif

#endif  // DALVIK_INTERP_INTERP_H_
