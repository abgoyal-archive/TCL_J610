

#ifndef DALVIK_MTERP_MTERP_H_
#define DALVIK_MTERP_MTERP_H_

#include "Dalvik.h"
#include "interp/InterpDefs.h"
#if defined(WITH_JIT)
#include "interp/Jit.h"
#endif

extern "C" bool dvmCheckAsmConstants(void);

extern "C" bool dvmMterpStdRun(Thread* self);
extern "C" void dvmMterpStdBail(Thread* self, bool changeInterp);

extern "C" void dvmMterpPrintMethod(Method* method);

#endif  // DALVIK_MTERP_MTERP_H_
