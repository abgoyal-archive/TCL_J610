

#ifndef DALVIK_CODEVERIFY_H_
#define DALVIK_CODEVERIFY_H_

#include "analysis/VerifySubs.h"
#include "analysis/VfyBasicBlock.h"

enum {
    kRegTypeUnknown = 0,    /* initial state; use value=0 so calloc works */
    kRegTypeUninit = 1,     /* MUST be odd to distinguish from pointer */
    kRegTypeConflict,       /* merge clash makes this reg's type unknowable */

    /*
     * Category-1nr types.  The order of these is chiseled into a couple
     * of tables, so don't add, remove, or reorder if you can avoid it.
     */
#define kRegType1nrSTART    kRegTypeZero
    kRegTypeZero,           /* 32-bit 0, could be Boolean, Int, Float, or Ref */
    kRegTypeOne,            /* 32-bit 1, could be Boolean, Int, Float */
    kRegTypeBoolean,        /* must be 0 or 1 */
    kRegTypeConstPosByte,   /* const derived byte, known positive */
    kRegTypeConstByte,      /* const derived byte */
    kRegTypeConstPosShort,  /* const derived short, known positive */
    kRegTypeConstShort,     /* const derived short */
    kRegTypeConstChar,      /* const derived char */
    kRegTypeConstInteger,   /* const derived integer */
    kRegTypePosByte,        /* byte, known positive (can become char) */
    kRegTypeByte,
    kRegTypePosShort,       /* short, known positive (can become char) */
    kRegTypeShort,
    kRegTypeChar,
    kRegTypeInteger,
    kRegTypeFloat,
#define kRegType1nrEND      kRegTypeFloat

    kRegTypeConstLo,        /* const derived wide, lower half */
    kRegTypeConstHi,        /* const derived wide, upper half */
    kRegTypeLongLo,         /* lower-numbered register; endian-independent */
    kRegTypeLongHi,
    kRegTypeDoubleLo,
    kRegTypeDoubleHi,

    /*
     * Enumeration max; this is used with "full" (32-bit) RegType values.
     *
     * Anything larger than this is a ClassObject or uninit ref.  Mask off
     * all but the low 8 bits; if you're left with kRegTypeUninit, pull
     * the uninit index out of the high 24.  Because kRegTypeUninit has an
     * odd value, there is no risk of a particular ClassObject pointer bit
     * pattern being confused for it (assuming our class object allocator
     * uses word alignment).
     */
    kRegTypeMAX
};
#define kRegTypeUninitMask  0xff
#define kRegTypeUninitShift 8

typedef u4 RegType;

typedef u4 MonitorEntries;
#define kMaxMonitorStackDepth   (sizeof(MonitorEntries) * 8)

struct RegisterLine {
    RegType*        regTypes;
    MonitorEntries* monitorEntries;
    u4*             monitorStack;
    unsigned int    monitorStackTop;
    BitVector*      liveRegs;
};

struct UninitInstanceMap {
    int numEntries;
    struct {
        int             addr;   /* code offset, or -1 for method arg ("this") */
        ClassObject*    clazz;  /* class created at this address */
    } map[1];
};
#define kUninitThisArgAddr  (-1)
#define kUninitThisArgSlot  0

struct VerifierData {
    /*
     * The method we're working on.
     */
    const Method*   method;

    /*
     * Number of code units of instructions in the method.  A cache of the
     * value calculated by dvmGetMethodInsnsSize().
     */
    u4              insnsSize;

    /*
     * Number of registers we track for each instruction.  This is equal
     * to the method's declared "registersSize".  (Does not include the
     * pending return value.)
     */
    u4              insnRegCount;

    /*
     * Instruction widths and flags, one entry per code unit.
     */
    InsnFlags*      insnFlags;

    /*
     * Uninitialized instance map, used for tracking the movement of
     * objects that have been allocated but not initialized.
     */
    UninitInstanceMap* uninitMap;

    /*
     * Array of RegisterLine structs, one entry per code unit.  We only need
     * entries for code units that hold the start of an "interesting"
     * instruction.  For register map generation, we're only interested
     * in GC points.
     */
    RegisterLine*   registerLines;

    /*
     * The number of occurrences of specific opcodes.
     */
    size_t          newInstanceCount;
    size_t          monitorEnterCount;

    /*
     * Array of pointers to basic blocks, one entry per code unit.  Used
     * for liveness analysis.
     */
    VfyBasicBlock** basicBlocks;
};


/* table with static merge logic for primitive types */
extern const char gDvmMergeTab[kRegTypeMAX][kRegTypeMAX];


INLINE bool dvmInsnIsOpcode(const InsnFlags* insnFlags, int addr) {
    return (insnFlags[addr] & kInsnFlagWidthMask) != 0;
}

INLINE int dvmInsnGetWidth(const InsnFlags* insnFlags, int addr) {
    return insnFlags[addr] & kInsnFlagWidthMask;
}

INLINE bool dvmInsnIsChanged(const InsnFlags* insnFlags, int addr) {
    return (insnFlags[addr] & kInsnFlagChanged) != 0;
}
INLINE void dvmInsnSetChanged(InsnFlags* insnFlags, int addr, bool changed)
{
    if (changed)
        insnFlags[addr] |= kInsnFlagChanged;
    else
        insnFlags[addr] &= ~kInsnFlagChanged;
}

INLINE bool dvmInsnIsVisited(const InsnFlags* insnFlags, int addr) {
    return (insnFlags[addr] & kInsnFlagVisited) != 0;
}
INLINE void dvmInsnSetVisited(InsnFlags* insnFlags, int addr, bool changed)
{
    if (changed)
        insnFlags[addr] |= kInsnFlagVisited;
    else
        insnFlags[addr] &= ~kInsnFlagVisited;
}

INLINE bool dvmInsnIsVisitedOrChanged(const InsnFlags* insnFlags, int addr) {
    return (insnFlags[addr] & (kInsnFlagVisited|kInsnFlagChanged)) != 0;
}

INLINE bool dvmInsnIsInTry(const InsnFlags* insnFlags, int addr) {
    return (insnFlags[addr] & kInsnFlagInTry) != 0;
}
INLINE void dvmInsnSetInTry(InsnFlags* insnFlags, int addr, bool inTry)
{
    assert(inTry);
    //if (inTry)
        insnFlags[addr] |= kInsnFlagInTry;
    //else
    //    insnFlags[addr] &= ~kInsnFlagInTry;
}

INLINE bool dvmInsnIsBranchTarget(const InsnFlags* insnFlags, int addr) {
    return (insnFlags[addr] & kInsnFlagBranchTarget) != 0;
}
INLINE void dvmInsnSetBranchTarget(InsnFlags* insnFlags, int addr,
    bool isBranch)
{
    assert(isBranch);
    //if (isBranch)
        insnFlags[addr] |= kInsnFlagBranchTarget;
    //else
    //    insnFlags[addr] &= ~kInsnFlagBranchTarget;
}

INLINE bool dvmInsnIsGcPoint(const InsnFlags* insnFlags, int addr) {
    return (insnFlags[addr] & kInsnFlagGcPoint) != 0;
}
INLINE void dvmInsnSetGcPoint(InsnFlags* insnFlags, int addr,
    bool isGcPoint)
{
    assert(isGcPoint);
    //if (isGcPoint)
        insnFlags[addr] |= kInsnFlagGcPoint;
    //else
    //    insnFlags[addr] &= ~kInsnFlagGcPoint;
}


UninitInstanceMap* dvmCreateUninitInstanceMap(const Method* meth,
    const InsnFlags* insnFlags, int newInstanceCount);

void dvmFreeUninitInstanceMap(UninitInstanceMap* uninitMap);

bool dvmVerifyCodeFlow(VerifierData* vdata);

#endif  // DALVIK_CODEVERIFY_H_
