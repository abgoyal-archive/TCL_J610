

#ifndef DALVIK_DEXVERIFY_H_
#define DALVIK_DEXVERIFY_H_

enum DexClassVerifyMode {
    VERIFY_MODE_UNKNOWN = 0,
    VERIFY_MODE_NONE,
    VERIFY_MODE_REMOTE,
    VERIFY_MODE_ALL
};

/* some verifier counters, for debugging */
struct VerifierStats {
    size_t methodsExamined;    /* number of methods examined */
    size_t monEnterMethods;    /* number of methods with monitor-enter */
    size_t instrsExamined;     /* incr on first visit of instruction */
    size_t instrsReexamined;   /* incr on each repeat visit of instruction */
    size_t copyRegCount;       /* calls from updateRegisters->copyRegisters */
    size_t mergeRegCount;      /* calls from updateRegisters->merge */
    size_t mergeRegChanged;    /* calls from updateRegisters->merge, changed */
    size_t uninitSearches;     /* times we've had to search the uninit table */
    size_t biggestAlloc;       /* largest RegisterLine table alloc */
};

#define VERIFY_GC_INST_MASK (kInstrCanBranch | kInstrCanSwitch |\
                             kInstrCanThrow | kInstrCanReturn)

bool dvmVerifyClass(ClassObject* clazz);

void dvmFreeRegisterMap(RegisterMap* pMap);

#endif  // DALVIK_DEXVERIFY_H_
