

#ifndef DALVIK_REGISTERMAP_H_
#define DALVIK_REGISTERMAP_H_

#include "analysis/VerifySubs.h"
#include "analysis/CodeVerify.h"

enum RegisterMapFormat {
    kRegMapFormatUnknown = 0,
    kRegMapFormatNone,          /* indicates no map data follows */
    kRegMapFormatCompact8,      /* compact layout, 8-bit addresses */
    kRegMapFormatCompact16,     /* compact layout, 16-bit addresses */
    kRegMapFormatDifferential,  /* compressed, differential encoding */

    kRegMapFormatOnHeap = 0x80, /* bit flag, indicates allocation on heap */
};

struct RegisterMap {
    /* header */
    u1      format;         /* enum RegisterMapFormat; MUST be first entry */
    u1      regWidth;       /* bytes per register line, 1+ */
    u1      numEntries[2];  /* number of entries */

    /* raw data starts here; need not be aligned */
    u1      data[1];
};

bool dvmRegisterMapStartup(void);
void dvmRegisterMapShutdown(void);

INLINE RegisterMapFormat dvmRegisterMapGetFormat(const RegisterMap* pMap) {
    return (RegisterMapFormat)(pMap->format & ~(kRegMapFormatOnHeap));
}

INLINE void dvmRegisterMapSetFormat(RegisterMap* pMap, RegisterMapFormat format)
{
    pMap->format &= kRegMapFormatOnHeap;
    pMap->format |= format;
}

INLINE bool dvmRegisterMapGetOnHeap(const RegisterMap* pMap) {
    return (pMap->format & kRegMapFormatOnHeap) != 0;
}

INLINE u1 dvmRegisterMapGetRegWidth(const RegisterMap* pMap) {
    return pMap->regWidth;
}

INLINE void dvmRegisterMapSetRegWidth(RegisterMap* pMap, int regWidth) {
    pMap->regWidth = regWidth;
}

INLINE void dvmRegisterMapSetOnHeap(RegisterMap* pMap, bool val) {
    if (val)
        pMap->format |= kRegMapFormatOnHeap;
    else
        pMap->format &= ~(kRegMapFormatOnHeap);
}

INLINE u2 dvmRegisterMapGetNumEntries(const RegisterMap* pMap) {
    return pMap->numEntries[0] | (pMap->numEntries[1] << 8);
}

INLINE void dvmRegisterMapSetNumEntries(RegisterMap* pMap, u2 numEntries) {
    pMap->numEntries[0] = (u1) numEntries;
    pMap->numEntries[1] = numEntries >> 8;
}

const u1* dvmRegisterMapGetLine(const RegisterMap* pMap, int addr);

INLINE void dvmReleaseRegisterMapLine(const RegisterMap* pMap, const u1* data)
{}


struct RegisterMapMethodPool {
    u2      methodCount;            /* chiefly used as a sanity check */

    /* stream of per-method data starts here */
    u4      methodData[1];
};

struct RegisterMapClassPool {
    u4      numClasses;

    /* offset table starts here, 32-bit aligned; offset==0 means no data */
    u4      classDataOffset[1];
};

const void* dvmRegisterMapGetClassData(const DexFile* pDexFile, u4 classIdx,
    u4* pNumMaps);

const RegisterMap* dvmRegisterMapGetNext(const void** pPtr);

struct RegisterMapBuilder {
    /* public */
    void*       data;
    size_t      size;

    /* private */
    MemMapping  memMap;
};

RegisterMapBuilder* dvmGenerateRegisterMaps(DvmDex* pDvmDex);

void dvmFreeRegisterMapBuilder(RegisterMapBuilder* pBuilder);

RegisterMap* dvmGenerateRegisterMapV(VerifierData* vdata);

const RegisterMap* dvmGetExpandedRegisterMap0(Method* method);
INLINE const RegisterMap* dvmGetExpandedRegisterMap(Method* method)
{
    const RegisterMap* curMap = method->registerMap;
    if (curMap == NULL)
        return NULL;
    RegisterMapFormat format = dvmRegisterMapGetFormat(curMap);
    if (format == kRegMapFormatCompact8 || format == kRegMapFormatCompact16) {
        return curMap;
    } else {
        return dvmGetExpandedRegisterMap0(method);
    }
}

/* dump stats gathered during register map creation process */
void dvmRegisterMapDumpStats(void);

#endif  // DALVIK_REGISTERMAP_H_
