
#ifndef DALVIK_REFLECT_REFLECT_H_
#define DALVIK_REFLECT_REFLECT_H_

bool dvmValidateBoxClasses();

ArrayObject* dvmGetDeclaredFields(ClassObject* clazz, bool publicOnly);

Object* dvmGetDeclaredField(ClassObject* clazz, StringObject* nameObj);

ArrayObject* dvmGetDeclaredConstructors(ClassObject* clazz, bool publicOnly);

ArrayObject* dvmGetDeclaredMethods(ClassObject* clazz, bool publicOnly);

Object* dvmGetDeclaredConstructorOrMethod(ClassObject* clazz,
    StringObject* nameObj, ArrayObject* args);

ArrayObject* dvmGetInterfaces(ClassObject* clazz);

Field* dvmSlotToField(ClassObject* clazz, int slot);
Method* dvmSlotToMethod(ClassObject* clazz, int slot);

int dvmConvertPrimitiveValue(PrimitiveType srcType,
    PrimitiveType dstType, const s4* srcPtr, s4* dstPtr);

int dvmConvertArgument(DataObject* arg, ClassObject* type, s4* ins);

DataObject* dvmBoxPrimitive(JValue value, ClassObject* returnType);

bool dvmUnboxPrimitive(Object* value, ClassObject* returnType,
    JValue* pResult);

ClassObject* dvmGetBoxedReturnType(const Method* meth);

Field* dvmGetFieldFromReflectObj(Object* obj);
Method* dvmGetMethodFromReflectObj(Object* obj);
Object* dvmCreateReflectObjForField(const ClassObject* clazz, Field* field);
Object* dvmCreateReflectObjForMethod(const ClassObject* clazz, Method* method);

INLINE bool dvmIsReflectionMethod(const Method* method)
{
    return (method->clazz == gDvm.classJavaLangReflectMethod);
}

ClassObject* dvmGenerateProxyClass(StringObject* str, ArrayObject* interfaces,
    Object* loader);

Object* dvmCreateReflectMethodObject(const Method* meth);

ArrayObject* dvmGetClassAnnotations(const ClassObject* clazz);
ArrayObject* dvmGetMethodAnnotations(const Method* method);
ArrayObject* dvmGetFieldAnnotations(const Field* field);
ArrayObject* dvmGetParameterAnnotations(const Method* method);

Object* dvmGetClassAnnotation(const ClassObject* clazz, const ClassObject* annotationClazz);
Object* dvmGetMethodAnnotation(const ClassObject* clazz, const Method* method,
        const ClassObject* annotationClazz);
Object* dvmGetFieldAnnotation(const ClassObject* clazz, const Field* method,
        const ClassObject* annotationClazz);

bool dvmIsClassAnnotationPresent(const ClassObject* clazz, const ClassObject* annotationClazz);
bool dvmIsMethodAnnotationPresent(const ClassObject* clazz, const Method* method,
        const ClassObject* annotationClazz);
bool dvmIsFieldAnnotationPresent(const ClassObject* clazz, const Field* method,
        const ClassObject* annotationClazz);

Object* dvmGetAnnotationDefaultValue(const Method* method);

ArrayObject* dvmGetMethodThrows(const Method* method);

ArrayObject* dvmGetClassSignatureAnnotation(const ClassObject* clazz);
ArrayObject* dvmGetMethodSignatureAnnotation(const Method* method);
ArrayObject* dvmGetFieldSignatureAnnotation(const Field* field);

Object* dvmGetEnclosingMethod(const ClassObject* clazz);

ClassObject* dvmGetDeclaringClass(const ClassObject* clazz);

ClassObject* dvmGetEnclosingClass(const ClassObject* clazz);

bool dvmGetInnerClass(const ClassObject* clazz, StringObject** pName,
    int* pAccessFlags);

ArrayObject* dvmGetDeclaredClasses(const ClassObject* clazz);

struct AnnotationValue {
    JValue  value;
    u1      type;
};


struct EncodedArrayIterator {
    const u1* cursor;                    /* current cursor */
    u4 elementsLeft;                     /* number of elements left to read */
    const DexEncodedArray* encodedArray; /* instance being iterated over */
    u4 size;                             /* number of elements in instance */
    const ClassObject* clazz;            /* class to resolve with respect to */
};

void dvmEncodedArrayIteratorInitialize(EncodedArrayIterator* iterator,
        const DexEncodedArray* encodedArray, const ClassObject* clazz);

bool dvmEncodedArrayIteratorHasNext(const EncodedArrayIterator* iterator);

bool dvmEncodedArrayIteratorGetNext(EncodedArrayIterator* iterator,
        AnnotationValue* value);

#endif  // DALVIK_REFLECT_REFLECT_H_
