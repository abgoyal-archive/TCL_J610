


#ifndef LIBDEX_DEXPROTO_H_
#define LIBDEX_DEXPROTO_H_

#include "DexFile.h"

struct DexStringCache {
    char* value;          /* the latest value */
    size_t allocatedSize; /* size of the allocated buffer, if allocated */
    char buffer[120];     /* buffer used to hold small-enough results */
};

void dexStringCacheAlloc(DexStringCache* pCache, size_t length);

void dexStringCacheInit(DexStringCache* pCache);

void dexStringCacheRelease(DexStringCache* pCache);

char* dexStringCacheEnsureCopy(DexStringCache* pCache, const char* value);

char* dexStringCacheAbandon(DexStringCache* pCache, const char* value);

struct DexProto {
    const DexFile* dexFile;     /* file the idx refers to */
    u4 protoIdx;                /* index into proto_ids table of dexFile */
};

DEX_INLINE void dexProtoSetFromMethodId(DexProto* pProto,
    const DexFile* pDexFile, const DexMethodId* pMethodId)
{
    pProto->dexFile = pDexFile;
    pProto->protoIdx = pMethodId->protoIdx;
}

const char* dexProtoGetShorty(const DexProto* pProto);

const char* dexProtoGetMethodDescriptor(const DexProto* pProto,
    DexStringCache* pCache);

char* dexProtoCopyMethodDescriptor(const DexProto* pProto);

const char* dexProtoGetParameterDescriptors(const DexProto* pProto,
    DexStringCache* pCache);

DEX_INLINE const char* dexGetDescriptorFromMethodId(const DexFile* pDexFile,
        const DexMethodId* pMethodId, DexStringCache* pCache)
{
    DexProto proto;

    dexProtoSetFromMethodId(&proto, pDexFile, pMethodId);
    return dexProtoGetMethodDescriptor(&proto, pCache);
}

DEX_INLINE char* dexCopyDescriptorFromMethodId(const DexFile* pDexFile,
    const DexMethodId* pMethodId)
{
    DexProto proto;

    dexProtoSetFromMethodId(&proto, pDexFile, pMethodId);
    return dexProtoCopyMethodDescriptor(&proto);
}

const char* dexProtoGetReturnType(const DexProto* pProto);

size_t dexProtoGetParameterCount(const DexProto* pProto);

int dexProtoComputeArgsSize(const DexProto* pProto);

int dexProtoCompare(const DexProto* pProto1, const DexProto* pProto2);

int dexProtoCompareParameters(const DexProto* pProto1,
        const DexProto* pProto2);

int dexProtoCompareToDescriptor(const DexProto* proto, const char* descriptor);

int dexProtoCompareToParameterDescriptors(const DexProto* proto,
        const char* descriptors);

struct DexParameterIterator {
    const DexProto* proto;
    const DexTypeList* parameters;
    int parameterCount;
    int cursor;
};

void dexParameterIteratorInit(DexParameterIterator* pIterator,
        const DexProto* pProto);

u4 dexParameterIteratorNextIndex(DexParameterIterator* pIterator);

const char* dexParameterIteratorNextDescriptor(
        DexParameterIterator* pIterator);

#endif  // LIBDEX_DEXPROTO_H_
