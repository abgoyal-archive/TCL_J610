

#ifndef DALVIK_VM_LOOP_H_
#define DALVIK_VM_LOOP_H_

#include "Dalvik.h"
#include "CompilerInternals.h"

typedef struct LoopAnalysis {
    BitVector *isIndVarV;               // length == numSSAReg
    GrowableList *ivList;               // induction variables
    GrowableList *arrayAccessInfo;      // hoisted checks for array accesses
    int numBasicIV;                     // number of basic induction variables
    int ssaBIV;                         // basic IV in SSA name
    bool isCountUpLoop;                 // count up or down loop
    Opcode loopBranchOpcode;            // OP_IF_XXX for the loop back branch
    int endConditionReg;                // vB in "vA op vB"
    LIR *branchToBody;                  // branch over to the body from entry
    LIR *branchToPCR;                   // branch over to the PCR cell
    bool bodyIsClean;                   // loop body cannot throw any exceptions
} LoopAnalysis;

bool dvmCompilerFilterLoopBlocks(CompilationUnit *cUnit);

#define BAIL_LOOP_COMPILATION() if (cUnit->jitMode == kJitLoop) {       \
                                    cUnit->quitLoopMode = true;         \
                                    return false;                       \
                                }

#endif  // DALVIK_VM_LOOP_H_
