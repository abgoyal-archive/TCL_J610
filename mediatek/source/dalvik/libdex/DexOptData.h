


#ifndef _LIBDEX_DEXOPTDATA
#define _LIBDEX_DEXOPTDATA

#include "libdex/DexFile.h"

bool dexParseOptData(const u1* data, size_t length, DexFile* pDexFile);

u4 dexComputeOptChecksum(const DexOptHeader* pOptHeader);

#endif /* def _LIBDEX_DEXOPTDATA */
