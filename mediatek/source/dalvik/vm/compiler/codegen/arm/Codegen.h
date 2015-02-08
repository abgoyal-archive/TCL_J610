


#include "compiler/CompilerIR.h"
#include "CalloutHelper.h"

#if defined(_CODEGEN_C)
static ArmLIR *opRegImm(CompilationUnit *cUnit, OpKind op, int rDestSrc1,
                        int value);
static ArmLIR *opRegReg(CompilationUnit *cUnit, OpKind op, int rDestSrc1,
                        int rSrc2);

/* Forward decalraton the portable versions due to circular dependency */
static bool genArithOpFloatPortable(CompilationUnit *cUnit, MIR *mir,
                                    RegLocation rlDest, RegLocation rlSrc1,
                                    RegLocation rlSrc2);

static bool genArithOpDoublePortable(CompilationUnit *cUnit, MIR *mir,
                                     RegLocation rlDest, RegLocation rlSrc1,
                                     RegLocation rlSrc2);

static bool genConversionPortable(CompilationUnit *cUnit, MIR *mir);

#if defined(__ARM_ARCH_5__)
static void genMonitorPortable(CompilationUnit *cUnit, MIR *mir);
#endif

static void genInterpSingleStep(CompilationUnit *cUnit, MIR *mir);

#endif


#if defined(WITH_SELF_VERIFICATION)
/* Self Verification memory instruction decoder */
extern "C" void dvmSelfVerificationMemOpDecode(int lr, int* sp);
#endif

extern void dvmCompilerSetupResourceMasks(ArmLIR *lir);

extern ArmLIR* dvmCompilerRegCopyNoInsert(CompilationUnit *cUnit, int rDest,
                                          int rSrc);
