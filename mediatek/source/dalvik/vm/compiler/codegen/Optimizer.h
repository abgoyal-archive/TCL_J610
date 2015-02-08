

#ifndef DALVIK_VM_COMPILER_OPTIMIZATION_H_
#define DALVIK_VM_COMPILER_OPTIMIZATION_H_

#include "Dalvik.h"

enum optControlVector {
    kLoadStoreElimination = 0,
    kLoadHoisting,
    kTrackLiveTemps,
    kSuppressLoads,
    kMethodInlining,
    kMethodJit,
};

/* Forward declarations */
struct CompilationUnit;
struct LIR;

void dvmCompilerApplyLocalOptimizations(struct CompilationUnit *cUnit,
                                        struct LIR *head,
                                        struct LIR *tail);

void dvmCompilerApplyGlobalOptimizations(struct CompilationUnit *cUnit);

#endif  // DALVIK_VM_COMPILER_OPTIMIZATION_H_
