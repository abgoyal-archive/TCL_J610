
#ifndef DALVIK_INTERP_STATE_H_
#define DALVIK_INTERP_STATE_H_

enum ExecutionMode {
    kExecutionModeUnknown = 0,
    kExecutionModeInterpPortable,
    kExecutionModeInterpFast,
#if defined(WITH_JIT)
    kExecutionModeJit,
#endif
};

enum ExecutionSubModes {
    kSubModeNormal            = 0x0000,   /* No active subMode */
    kSubModeMethodTrace       = 0x0001,
    kSubModeEmulatorTrace     = 0x0002,
    kSubModeInstCounting      = 0x0004,
    kSubModeDebuggerActive    = 0x0008,
    kSubModeSuspendPending    = 0x0010,
    kSubModeCallbackPending   = 0x0020,
    kSubModeCountedStep       = 0x0040,
    kSubModeCheckAlways       = 0x0080,
    kSubModeJitTraceBuild     = 0x4000,
    kSubModeJitSV             = 0x8000,
    kSubModeDebugProfile   = (kSubModeMethodTrace |
                              kSubModeEmulatorTrace |
                              kSubModeInstCounting |
                              kSubModeDebuggerActive)
};


enum InterpBreakFlags {
    kInterpNoBreak            = 0x00,    /* Don't check */
    kInterpSingleStep         = 0x01,    /* Check between each inst */
    kInterpSafePoint          = 0x02,    /* Check at safe points */
};

#define SINGLESTEP_BREAK_MASK ( kSubModeInstCounting | \
                                kSubModeDebuggerActive | \
                                kSubModeCountedStep | \
                                kSubModeCheckAlways | \
                                kSubModeJitSV | \
                                kSubModeJitTraceBuild )

#define SAFEPOINT_BREAK_MASK  ( kSubModeSuspendPending | \
                                kSubModeCallbackPending )

typedef bool (*SafePointCallback)(struct Thread* thread, void* arg);

#define LOCAL_SUBMODE (kSubModeJitTraceBuild)

struct InterpSaveState {
    const u2*       pc;         // Dalvik PC
    u4*             curFrame;   // Dalvik frame pointer
    const Method    *method;    // Method being executed
    DvmDex*         methodClassDex;
    JValue          retval;
    void*           bailPtr;
#if defined(WITH_TRACKREF_CHECKS)
    int             debugTrackedRefStart;
#else
    int             unused;        // Keep struct size constant
#endif
    struct InterpSaveState* prev;  // To follow nested activations
} __attribute__ ((__packed__));

#ifdef WITH_JIT
struct JitToInterpEntries {
    void (*dvmJitToInterpNormal)(void);
    void (*dvmJitToInterpNoChain)(void);
    void (*dvmJitToInterpPunt)(void);
    void (*dvmJitToInterpSingleStep)(void);
    void (*dvmJitToInterpTraceSelect)(void);
#if defined(WITH_SELF_VERIFICATION)
    void (*dvmJitToInterpBackwardBranch)(void);
#else
    void (*unused)(void);  // Keep structure size constant
#endif
};

/* States of the interpreter when serving a JIT-related request */
enum JitState {
    /* Entering states in the debug interpreter */
    kJitNot = 0,               // Non-JIT related reasons */
    kJitTSelectRequest = 1,    // Request a trace (subject to filtering)
    kJitTSelectRequestHot = 2, // Request a hot trace (bypass the filter)
    kJitSelfVerification = 3,  // Self Verification Mode

    /* Operational states in the debug interpreter */
    kJitTSelect = 4,           // Actively selecting a trace
    kJitTSelectEnd = 5,        // Done with the trace - wrap it up
    kJitDone = 6,              // No further JIT actions for interpBreak
};

#if defined(WITH_SELF_VERIFICATION)
enum SelfVerificationState {
    kSVSIdle = 0,           // Idle
    kSVSStart = 1,          // Shadow space set up, running compiled code
    kSVSPunt = 2,           // Exiting compiled code by punting
    kSVSSingleStep = 3,     // Exiting compiled code by single stepping
    kSVSNoProfile = 4,      // Exiting compiled code and don't collect profiles
    kSVSTraceSelect = 5,    // Exiting compiled code and compile the next pc
    kSVSNormal = 6,         // Exiting compiled code normally
    kSVSNoChain = 7,        // Exiting compiled code by no chain
    kSVSBackwardBranch = 8, // Exiting compiled code with backward branch trace
    kSVSDebugInterp = 9,    // Normal state restored, running debug interpreter
};
#endif

/* Number of entries in the 2nd level JIT profiler filter cache */
#define JIT_TRACE_THRESH_FILTER_SIZE 32
/* Number of low dalvik pc address bits to include in 2nd level filter key */
#define JIT_TRACE_THRESH_FILTER_PC_BITS 4
#define MAX_JIT_RUN_LEN 64

enum JitHint {
   kJitHintNone = 0,
   kJitHintTaken = 1,         // Last inst in run was taken branch
   kJitHintNotTaken = 2,      // Last inst in run was not taken branch
   kJitHintNoBias = 3,        // Last inst in run was unbiased branch
};

struct JitCodeDesc {
    unsigned numInsts:8;     // Number of Byte codes in run
    unsigned runEnd:1;       // Run ends with last byte code
    JitHint hint:7;          // Hint to apply to final code of run
    u2 startOffset;          // Starting offset for trace run
};

struct JitTraceRun {
    union {
        JitCodeDesc frag;
        void*       meta;
    } info;
    u4 isCode:1;
    u4 unused:31;
};

#endif

#endif  // DALVIK_INTERP_STATE_H_
