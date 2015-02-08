

#ifndef DALVIK_VERIFYSUBS_H_
#define DALVIK_VERIFYSUBS_H_

typedef u4 InsnFlags;

#define kInsnFlagWidthMask      0x0000ffff
#define kInsnFlagInTry          (1 << 16)
#define kInsnFlagBranchTarget   (1 << 17)
#define kInsnFlagGcPoint        (1 << 18)
#define kInsnFlagVisited        (1 << 30)
#define kInsnFlagChanged        (1 << 31)

/* add opcode widths to InsnFlags */
bool dvmComputeCodeWidths(const Method* meth, InsnFlags* insnFlags,
    int* pNewInstanceCount);

/* set the "in try" flag for sections of code wrapped with a "try" block */
bool dvmSetTryFlags(const Method* meth, InsnFlags* insnFlags);

/* verification failure reporting */
#define LOG_VFY(...)                dvmLogVerifyFailure(NULL, __VA_ARGS__)
#define LOG_VFY_METH(_meth, ...)    dvmLogVerifyFailure(_meth, __VA_ARGS__)

/* log verification failure with optional method info */
void dvmLogVerifyFailure(const Method* meth, const char* format, ...)
#if defined(__GNUC__)
    __attribute__ ((format(printf, 2, 3)))
#endif
    ;

/* log verification failure due to resolution trouble */
void dvmLogUnableToResolveClass(const char* missingClassDescr,
    const Method* meth);

/* extract the relative branch offset from a branch instruction */
bool dvmGetBranchOffset(const Method* meth, const InsnFlags* insnFlags,
    int curOffset, s4* pOffset, bool* pConditional);

/* return a RegType enumeration value that "value" just fits into */
char dvmDetermineCat1Const(s4 value);

/* debugging */
bool dvmWantVerboseVerification(const Method* meth);

#endif  // DALVIK_VERIFYSUBS_H_
