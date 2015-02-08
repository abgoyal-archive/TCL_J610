


#ifndef LIBDEX_DEXDEBUGINFO_H_
#define LIBDEX_DEXDEBUGINFO_H_

#include "DexFile.h"

typedef int (*DexDebugNewPositionCb)(void *cnxt, u4 address, u4 lineNum);

typedef void (*DexDebugNewLocalCb)(void *cnxt, u2 reg, u4 startAddress,
        u4 endAddress, const char *name, const char *descriptor,
        const char *signature);

void dexDecodeDebugInfo(
            const DexFile* pDexFile,
            const DexCode* pDexCode,
            const char* classDescriptor,
            u4 protoIdx,
            u4 accessFlags,
            DexDebugNewPositionCb posCb, DexDebugNewLocalCb localCb,
            void* cnxt);

#endif  // LIBDEX_DEXDEBUGINFO_H_
