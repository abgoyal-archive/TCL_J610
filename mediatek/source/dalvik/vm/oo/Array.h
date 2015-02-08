
#ifndef DALVIK_OO_ARRAY_H_
#define DALVIK_OO_ARRAY_H_

ClassObject* dvmFindArrayClass(const char* descriptor, Object* loader);

ClassObject* dvmFindArrayClassForElement(ClassObject* elemClassObj);

extern "C" ArrayObject* dvmAllocArrayByClass(ClassObject* arrayClass,
    size_t length, int allocFlags);

ArrayObject* dvmAllocPrimitiveArray(char type, size_t length, int allocFlags);

ArrayObject* dvmAllocMultiArray(ClassObject* arrayClass, int curDim,
    const int* dimensions);

INLINE bool dvmIsArray(const ArrayObject* arrayObj)
{
    return ( ((Object*)arrayObj)->clazz->descriptor[0] == '[' );
}

INLINE bool dvmIsObjectArrayClass(const ClassObject* clazz)
{
    const char* descriptor = clazz->descriptor;
    return descriptor[0] == '[' && (descriptor[1] == 'L' ||
                                    descriptor[1] == '[');
}

INLINE bool dvmIsObjectArray(const ArrayObject* arrayObj)
{
    return dvmIsObjectArrayClass(arrayObj->clazz);
}

INLINE bool dvmIsArrayClass(const ClassObject* clazz)
{
    return (clazz->descriptor[0] == '[');
}

bool dvmCopyObjectArray(ArrayObject* dstArray, const ArrayObject* srcArray,
    ClassObject* dstElemClass);

bool dvmUnboxObjectArray(ArrayObject* dstArray, const ArrayObject* srcArray,
    ClassObject* dstElemClass);

size_t dvmArrayObjectSize(const ArrayObject *array);

size_t dvmArrayClassElementWidth(const ClassObject* clazz);

#endif  // DALVIK_OO_ARRAY_H_
