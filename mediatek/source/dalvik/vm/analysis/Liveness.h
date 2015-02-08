

#ifndef DALVIK_LIVENESS_H_
#define DALVIK_LIVENESS_H_

struct VerifierData;

bool dvmComputeLiveness(struct VerifierData* vdata);

#endif  // DALVIK_LIVENESS_H_
