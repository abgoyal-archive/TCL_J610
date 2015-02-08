


#ifndef LIBDEX_DEXCLASS_H_
#define LIBDEX_DEXCLASS_H_

#include "DexFile.h"
#include "Leb128.h"

/* expanded form of a class_data_item header */
struct DexClassDataHeader {
    u4 staticFieldsSize;
    u4 instanceFieldsSize;
    u4 directMethodsSize;
    u4 virtualMethodsSize;
};

/* expanded form of encoded_field */
struct DexField {
    u4 fieldIdx;    /* index to a field_id_item */
    u4 accessFlags;
};

/* expanded form of encoded_method */
struct DexMethod {
    u4 methodIdx;    /* index to a method_id_item */
    u4 accessFlags;
    u4 codeOff;      /* file offset to a code_item */
};

struct DexClassData {
    DexClassDataHeader header;
    DexField*          staticFields;
    DexField*          instanceFields;
    DexMethod*         directMethods;
    DexMethod*         virtualMethods;
};

bool dexReadAndVerifyClassDataHeader(const u1** pData, const u1* pLimit,
        DexClassDataHeader *pHeader);

bool dexReadAndVerifyClassDataField(const u1** pData, const u1* pLimit,
        DexField* pField, u4* lastIndex);

bool dexReadAndVerifyClassDataMethod(const u1** pData, const u1* pLimit,
        DexMethod* pMethod, u4* lastIndex);

DexClassData* dexReadAndVerifyClassData(const u1** pData, const u1* pLimit);

DEX_INLINE const DexCode* dexGetCode(const DexFile* pDexFile,
    const DexMethod* pDexMethod)
{
    if (pDexMethod->codeOff == 0)
        return NULL;
    return (const DexCode*) (pDexFile->baseAddr + pDexMethod->codeOff);
}


DEX_INLINE void dexReadClassDataHeader(const u1** pData,
        DexClassDataHeader *pHeader) {
    pHeader->staticFieldsSize = readUnsignedLeb128(pData);
    pHeader->instanceFieldsSize = readUnsignedLeb128(pData);
    pHeader->directMethodsSize = readUnsignedLeb128(pData);
    pHeader->virtualMethodsSize = readUnsignedLeb128(pData);
}

DEX_INLINE void dexReadClassDataField(const u1** pData, DexField* pField,
        u4* lastIndex) {
    u4 index = *lastIndex + readUnsignedLeb128(pData);

    pField->accessFlags = readUnsignedLeb128(pData);
    pField->fieldIdx = index;
    *lastIndex = index;
}

DEX_INLINE void dexReadClassDataMethod(const u1** pData, DexMethod* pMethod,
        u4* lastIndex) {
    u4 index = *lastIndex + readUnsignedLeb128(pData);

    pMethod->accessFlags = readUnsignedLeb128(pData);
    pMethod->codeOff = readUnsignedLeb128(pData);
    pMethod->methodIdx = index;
    *lastIndex = index;
}

#endif  // LIBDEX_DEXCLASS_H_
