

#ifndef DALVIK_INTERP_STACK_H_
#define DALVIK_INTERP_STACK_H_

#include "jni.h"
#include <stdarg.h>



struct StackSaveArea;

//#define PAD_SAVE_AREA       /* help debug stack trampling */

struct StackSaveArea {
#ifdef PAD_SAVE_AREA
    u4          pad0, pad1, pad2;
#endif

#ifdef EASY_GDB
    /* make it easier to trek through stack frames in GDB */
    StackSaveArea* prevSave;
#endif

    /* saved frame pointer for previous frame, or NULL if this is at bottom */
    u4*         prevFrame;

    /* saved program counter (from method in caller's frame) */
    const u2*   savedPc;

    /* pointer to method we're *currently* executing; handy for exceptions */
    const Method* method;

    union {
        /* for JNI native methods: bottom of local reference segment */
        u4          localRefCookie;

        /* for interpreted methods: saved current PC, for exception stack
         * traces and debugger traces */
        const u2*   currentPc;
    } xtra;

    /* Native return pointer for JIT, or 0 if interpreted */
    const u2* returnAddr;
#ifdef PAD_SAVE_AREA
    u4          pad3, pad4, pad5;
#endif
};

/* move between the stack save area and the frame pointer */
#define SAVEAREA_FROM_FP(_fp)   ((StackSaveArea*)(_fp) -1)
#define FP_FROM_SAVEAREA(_save) ((u4*) ((StackSaveArea*)(_save) +1))

/* when calling a function, get a pointer to outs[0] */
#define OUTS_FROM_FP(_fp, _argCount) \
    ((u4*) ((u1*)SAVEAREA_FROM_FP(_fp) - sizeof(u4) * (_argCount)))

/* reserve this many bytes for handling StackOverflowError */
#define STACK_OVERFLOW_RESERVE  768

INLINE bool dvmIsBreakFrame(const u4* fp)
{
    return SAVEAREA_FROM_FP(fp)->method == NULL;
}

bool dvmInitInterpStack(Thread* thread, int stackSize);

bool dvmPushJNIFrame(Thread* thread, const Method* method);

bool dvmPushLocalFrame(Thread* thread, const Method* method);
bool dvmPopLocalFrame(Thread* thread);

void dvmCallMethod(Thread* self, const Method* method, Object* obj,
    JValue* pResult, ...);
void dvmCallMethodV(Thread* self, const Method* method, Object* obj,
    bool fromJni, JValue* pResult, va_list args);
void dvmCallMethodA(Thread* self, const Method* method, Object* obj,
    bool fromJni, JValue* pResult, const jvalue* args);

Object* dvmInvokeMethod(Object* invokeObj, const Method* meth,
    ArrayObject* argList, ArrayObject* params, ClassObject* returnType,
    bool noAccessCheck);

extern "C" int dvmLineNumFromPC(const Method* method, u4 relPc);

int dvmComputeExactFrameDepth(const void* fp);
int dvmComputeVagueFrameDepth(Thread* thread, const void* fp);

void* dvmGetCallerFP(const void* curFrame);

ClassObject* dvmGetCallerClass(const void* curFrame);

ClassObject* dvmGetCaller2Class(const void* curFrame);

ClassObject* dvmGetCaller3Class(const void* curFrame);

void dvmFillStackTraceArray(const void* fp, const Method** array, size_t length);

extern "C" void dvmHandleStackOverflow(Thread* self, const Method* method);
extern "C" void dvmCleanupStackOverflow(Thread* self, const Object* exception);

/* debugging; dvmDumpThread() is probably a better starting point */
void dvmDumpThreadStack(const DebugOutputTarget* target, Thread* thread);
void dvmDumpRunningThreadStack(const DebugOutputTarget* target, Thread* thread);

#endif  // DALVIK_INTERP_STACK_H_
