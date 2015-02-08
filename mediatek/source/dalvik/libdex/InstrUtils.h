

#ifndef LIBDEX_INSTRUTILS_H_
#define LIBDEX_INSTRUTILS_H_

#include "DexFile.h"
#include "DexOpcodes.h"

enum InstructionFormat {
    kFmt00x = 0,    // unknown format (also used for "breakpoint" opcode)
    kFmt10x,        // op
    kFmt12x,        // op vA, vB
    kFmt11n,        // op vA, #+B
    kFmt11x,        // op vAA
    kFmt10t,        // op +AA
    kFmt20bc,       // [opt] op AA, thing@BBBB
    kFmt20t,        // op +AAAA
    kFmt22x,        // op vAA, vBBBB
    kFmt21t,        // op vAA, +BBBB
    kFmt21s,        // op vAA, #+BBBB
    kFmt21h,        // op vAA, #+BBBB00000[00000000]
    kFmt21c,        // op vAA, thing@BBBB
    kFmt23x,        // op vAA, vBB, vCC
    kFmt22b,        // op vAA, vBB, #+CC
    kFmt22t,        // op vA, vB, +CCCC
    kFmt22s,        // op vA, vB, #+CCCC
    kFmt22c,        // op vA, vB, thing@CCCC
    kFmt22cs,       // [opt] op vA, vB, field offset CCCC
    kFmt30t,        // op +AAAAAAAA
    kFmt32x,        // op vAAAA, vBBBB
    kFmt31i,        // op vAA, #+BBBBBBBB
    kFmt31t,        // op vAA, +BBBBBBBB
    kFmt31c,        // op vAA, string@BBBBBBBB
    kFmt35c,        // op {vC,vD,vE,vF,vG}, thing@BBBB
    kFmt35ms,       // [opt] invoke-virtual+super
    kFmt3rc,        // op {vCCCC .. v(CCCC+AA-1)}, thing@BBBB
    kFmt3rms,       // [opt] invoke-virtual+super/range
    kFmt51l,        // op vAA, #+BBBBBBBBBBBBBBBB
    kFmt35mi,       // [opt] inline invoke
    kFmt3rmi,       // [opt] inline invoke/range
    kFmt33x,        // exop vAA, vBB, vCCCC
    kFmt32s,        // exop vAA, vBB, #+CCCC
    kFmt40sc,       // [opt] exop AAAA, thing@BBBBBBBB
    kFmt41c,        // exop vAAAA, thing@BBBBBBBB
    kFmt52c,        // exop vAAAA, vBBBB, thing@CCCCCCCC
    kFmt5rc,        // exop {vCCCC .. v(CCCC+AAAA-1)}, thing@BBBBBBBB
};

enum InstructionIndexType {
    kIndexUnknown = 0,
    kIndexNone,         // has no index
    kIndexVaries,       // "It depends." Used for throw-verification-error
    kIndexTypeRef,      // type reference index
    kIndexStringRef,    // string reference index
    kIndexMethodRef,    // method reference index
    kIndexFieldRef,     // field reference index
    kIndexInlineMethod, // inline method index (for inline linked methods)
    kIndexVtableOffset, // vtable offset (for static linked methods)
    kIndexFieldOffset   // field offset (for static linked fields)
};

typedef u1 InstructionWidth;

typedef u1 OpcodeFlags;
enum OpcodeFlagsBits {
    kInstrCanBranch     = 1,        // conditional or unconditional branch
    kInstrCanContinue   = 1 << 1,   // flow can continue to next statement
    kInstrCanSwitch     = 1 << 2,   // switch statement
    kInstrCanThrow      = 1 << 3,   // could cause an exception to be thrown
    kInstrCanReturn     = 1 << 4,   // returns, no additional statements
    kInstrInvoke        = 1 << 5,   // a flavor of invoke
};

struct InstructionInfoTables {
    u1*                formats;    /* InstructionFormat elements */
    u1*                indexTypes; /* InstructionIndexType elements */
    OpcodeFlags*       flags;
    InstructionWidth*  widths;
};

extern InstructionInfoTables gDexOpcodeInfo;

struct DecodedInstruction {
    u4      vA;
    u4      vB;
    u8      vB_wide;        /* for kFmt51l */
    u4      vC;
    u4      arg[5];         /* vC/D/E/F/G in invoke or filled-new-array */
    Opcode  opcode;
    InstructionIndexType indexType;
};

DEX_INLINE size_t dexGetWidthFromOpcode(Opcode opcode)
{
    assert((u4) opcode < kNumPackedOpcodes);
    return gDexOpcodeInfo.widths[opcode];
}

size_t dexGetWidthFromInstruction(const u2* insns);

DEX_INLINE OpcodeFlags dexGetFlagsFromOpcode(Opcode opcode)
{
    assert((u4) opcode < kNumPackedOpcodes);
    return gDexOpcodeInfo.flags[opcode];
}

DEX_INLINE bool dexIsGoto(OpcodeFlags flags)
{
    return (flags & (kInstrCanBranch | kInstrCanContinue)) == kInstrCanBranch;
}

DEX_INLINE InstructionFormat dexGetFormatFromOpcode(Opcode opcode)
{
    assert((u4) opcode < kNumPackedOpcodes);
    return (InstructionFormat) gDexOpcodeInfo.formats[opcode];
}

DEX_INLINE InstructionIndexType dexGetIndexTypeFromOpcode(Opcode opcode)
{
    assert((u4) opcode < kNumPackedOpcodes);
    return (InstructionIndexType) gDexOpcodeInfo.indexTypes[opcode];
}

void dexDecodeInstruction(const u2* insns, DecodedInstruction* pDec);

#endif  // LIBDEX_INSTRUTILS_H_
