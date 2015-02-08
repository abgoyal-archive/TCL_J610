

#ifndef DALVIK_ALLOC_VISITINLINES_H_
#define DALVIK_ALLOC_VISITINLINES_H_

static void visitFields(Visitor *visitor, Object *obj, void *arg)
{
    assert(visitor != NULL);
    assert(obj != NULL);
    assert(obj->clazz != NULL);
    if (obj->clazz->refOffsets != CLASS_WALK_SUPER) {
        size_t refOffsets = obj->clazz->refOffsets;
        while (refOffsets != 0) {
            size_t rshift = CLZ(refOffsets);
            size_t offset = CLASS_OFFSET_FROM_CLZ(rshift);
            Object **ref = (Object **)BYTE_OFFSET(obj, offset);
            (*visitor)(ref, arg);
            refOffsets &= ~(CLASS_HIGH_BIT >> rshift);
        }
    } else {
        for (ClassObject *clazz = obj->clazz;
             clazz != NULL;
             clazz = clazz->super) {
            InstField *field = clazz->ifields;
            for (int i = 0; i < clazz->ifieldRefCount; ++i, ++field) {
                size_t offset = field->byteOffset;
                Object **ref = (Object **)BYTE_OFFSET(obj, offset);
                (*visitor)(ref, arg);
            }
        }
    }
}

static void visitStaticFields(Visitor *visitor, ClassObject *clazz,
                              void *arg)
{
    assert(visitor != NULL);
    assert(clazz != NULL);
    for (int i = 0; i < clazz->sfieldCount; ++i) {
        char ch = clazz->sfields[i].signature[0];
        if (ch == '[' || ch == 'L') {
            (*visitor)(&clazz->sfields[i].value.l, arg);
        }
    }
}

static void visitInterfaces(Visitor *visitor, ClassObject *clazz,
                            void *arg)
{
    assert(visitor != NULL);
    assert(clazz != NULL);
    for (int i = 0; i < clazz->interfaceCount; ++i) {
        (*visitor)(&clazz->interfaces[i], arg);
    }
}

static void visitClassObject(Visitor *visitor, Object *obj, void *arg)
{
    ClassObject *asClass;

    assert(visitor != NULL);
    assert(obj != NULL);
    assert(obj->clazz != NULL);
    assert(!strcmp(obj->clazz->descriptor, "Ljava/lang/Class;"));
    (*visitor)(&obj->clazz, arg);
    asClass = (ClassObject *)obj;
    if (IS_CLASS_FLAG_SET(asClass, CLASS_ISARRAY)) {
        (*visitor)(&asClass->elementClass, arg);
    }
    if (asClass->status > CLASS_IDX) {
        (*visitor)(&asClass->super, arg);
    }
    (*visitor)(&asClass->classLoader, arg);
    visitFields(visitor, obj, arg);
    visitStaticFields(visitor, asClass, arg);
    if (asClass->status > CLASS_IDX) {
      visitInterfaces(visitor, asClass, arg);
    }
}

static void visitArrayObject(Visitor *visitor, Object *obj, void *arg)
{
    assert(visitor != NULL);
    assert(obj != NULL);
    assert(obj->clazz != NULL);
    (*visitor)(&obj->clazz, arg);
    if (IS_CLASS_FLAG_SET(obj->clazz, CLASS_ISOBJECTARRAY)) {
        ArrayObject *array = (ArrayObject *)obj;
        Object **contents = (Object **)(void *)array->contents;
        for (size_t i = 0; i < array->length; ++i) {
            (*visitor)(&contents[i], arg);
        }
    }
}

static void visitDataObject(Visitor *visitor, Object *obj, void *arg)
{
    assert(visitor != NULL);
    assert(obj != NULL);
    assert(obj->clazz != NULL);
    (*visitor)(&obj->clazz, arg);
    visitFields(visitor, obj, arg);
}

static void visitReferenceObject(Visitor *visitor, Object *obj, void *arg)
{
    assert(visitor != NULL);
    assert(obj != NULL);
    assert(obj->clazz != NULL);
    visitDataObject(visitor, obj, arg);
    size_t offset = gDvm.offJavaLangRefReference_referent;
    Object **ref = (Object **)BYTE_OFFSET(obj, offset);
    (*visitor)(ref, arg);
}

static void visitObject(Visitor *visitor, Object *obj, void *arg)
{
    assert(visitor != NULL);
    assert(obj != NULL);
    assert(obj->clazz != NULL);
    if (dvmIsClassObject(obj)) {
        visitClassObject(visitor, obj, arg);
    } else if (IS_CLASS_FLAG_SET(obj->clazz, CLASS_ISARRAY)) {
        visitArrayObject(visitor, obj, arg);
    } else if (IS_CLASS_FLAG_SET(obj->clazz, CLASS_ISREFERENCE)) {
        visitReferenceObject(visitor, obj, arg);
    } else {
        visitDataObject(visitor, obj, arg);
    }
}

#endif  // DALVIK_ALLOC_VISITINLINES_H_
