

#ifndef DALVIK_VFYBASICBLOCK_H_
#define DALVIK_VFYBASICBLOCK_H_

#include "PointerSet.h"

struct VerifierData;


struct VfyBasicBlock {
    u4              firstAddr;      /* address of first instruction */
    u4              lastAddr;       /* address of last instruction */
    PointerSet*     predecessors;   /* set of basic blocks that can flow here */
    BitVector*      liveRegs;       /* liveness for each register */
    bool            changed;        /* input set has changed, must re-eval */
    bool            visited;        /* block has been visited at least once */
};

bool dvmComputeVfyBasicBlocks(struct VerifierData* vdata);

void dvmFreeVfyBasicBlocks(struct VerifierData* vdata);

#endif  // DALVIK_VFYBASICBLOCK_H_
