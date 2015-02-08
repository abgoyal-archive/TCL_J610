


#ifndef LIBDEX_DEXCATCH_H_
#define LIBDEX_DEXCATCH_H_

#include "DexFile.h"
#include "Leb128.h"

struct DexCatchHandler {
    u4          typeIdx;    /* type index of the caught exception type */
    u4          address;    /* handler address */
};

u4 dexGetFirstHandlerOffset(const DexCode* pCode);

/* Get count of handler lists for the given DexCode. */
u4 dexGetHandlersSize(const DexCode* pCode);

struct DexCatchIterator {
    const u1* pEncodedData;
    bool catchesAll;
    u4 countRemaining;
    DexCatchHandler handler;
};

DEX_INLINE void dexCatchIteratorClear(DexCatchIterator* pIterator) {
    pIterator->pEncodedData = NULL;
    pIterator->catchesAll = false;
    pIterator->countRemaining = 0;
    pIterator->handler.typeIdx = 0;
    pIterator->handler.address = 0;
}

/* Initialize a DexCatchIterator with a direct pointer to encoded handlers. */
DEX_INLINE void dexCatchIteratorInitToPointer(DexCatchIterator* pIterator,
    const u1* pEncodedData)
{
    s4 count = readSignedLeb128(&pEncodedData);

    if (count <= 0) {
        pIterator->catchesAll = true;
        count = -count;
    } else {
        pIterator->catchesAll = false;
    }

    pIterator->pEncodedData = pEncodedData;
    pIterator->countRemaining = count;
}

/* Initialize a DexCatchIterator to a particular handler offset. */
DEX_INLINE void dexCatchIteratorInit(DexCatchIterator* pIterator,
    const DexCode* pCode, u4 offset)
{
    dexCatchIteratorInitToPointer(pIterator,
            dexGetCatchHandlerData(pCode) + offset);
}

/* Get the next item from a DexCatchIterator. Returns NULL if at end. */
DEX_INLINE DexCatchHandler* dexCatchIteratorNext(DexCatchIterator* pIterator) {
    if (pIterator->countRemaining == 0) {
        if (! pIterator->catchesAll) {
            return NULL;
        }

        pIterator->catchesAll = false;
        pIterator->handler.typeIdx = kDexNoIndex;
    } else {
        u4 typeIdx = readUnsignedLeb128(&pIterator->pEncodedData);
        pIterator->handler.typeIdx = typeIdx;
        pIterator->countRemaining--;
    }

    pIterator->handler.address = readUnsignedLeb128(&pIterator->pEncodedData);
    return &pIterator->handler;
}

u4 dexCatchIteratorGetEndOffset(DexCatchIterator* pIterator,
    const DexCode* pCode);

/* Helper for dexFindCatchHandler(). Do not call directly. */
int dexFindCatchHandlerOffset0(u2 triesSize, const DexTry* pTries,
        u4 address);

DEX_INLINE bool dexFindCatchHandler(DexCatchIterator *pIterator,
        const DexCode* pCode, u4 address) {
    u2 triesSize = pCode->triesSize;
    int offset = -1;

    // Short-circuit the overwhelmingly common cases.
    switch (triesSize) {
        case 0: {
            break;
        }
        case 1: {
            const DexTry* tries = dexGetTries(pCode);
            u4 start = tries[0].startAddr;

            if (address < start) {
                break;
            }

            u4 end = start + tries[0].insnCount;

            if (address >= end) {
                break;
            }

            offset = tries[0].handlerOff;
            break;
        }
        default: {
            offset = dexFindCatchHandlerOffset0(triesSize, dexGetTries(pCode),
                    address);
        }
    }

    if (offset < 0) {
        dexCatchIteratorClear(pIterator); // This squelches warnings.
        return false;
    } else {
        dexCatchIteratorInit(pIterator, pCode, offset);
        return true;
    }
}

#endif  // LIBDEX_DEXCATCH_H_
