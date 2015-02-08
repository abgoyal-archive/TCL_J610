


#ifndef LIBDEX_DEXDATAMAP_H_
#define LIBDEX_DEXDATAMAP_H_

#include "DexFile.h"

struct DexDataMap {
    u4 count;    /* number of items currently in the map */
    u4 max;      /* maximum number of items that may be held */
    u4* offsets; /* array of item offsets */
    u2* types;   /* corresponding array of item types */
};

DexDataMap* dexDataMapAlloc(u4 maxCount);

void dexDataMapFree(DexDataMap* map);

void dexDataMapAdd(DexDataMap* map, u4 offset, u2 type);

int dexDataMapGet(DexDataMap* map, u4 offset);

bool dexDataMapVerify(DexDataMap* map, u4 offset, u2 type);

DEX_INLINE bool dexDataMapVerify0Ok(DexDataMap* map, u4 offset, u2 type) {
    if (offset == 0) {
        return true;
    }

    return dexDataMapVerify(map, offset, type);
}

#endif  // LIBDEX_DEXDATAMAP_H_
