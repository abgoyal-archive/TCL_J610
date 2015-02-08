

#ifndef DALVIK_OO_OBJECTINLINES_H_
#define DALVIK_OO_OBJECTINLINES_H_

INLINE void dvmSetObjectArrayElement(const ArrayObject* obj, int index,
                                     Object* val) {
    ((Object **)(void *)(obj)->contents)[index] = val;
    if (val != NULL) {
        dvmWriteBarrierArray(obj, index, index + 1);
    }
}


#define BYTE_OFFSET(_ptr, _offset)  ((void*) (((u1*)(_ptr)) + (_offset)))

INLINE JValue* dvmFieldPtr(const Object* obj, int offset) {
    return ((JValue*)BYTE_OFFSET(obj, offset));
}

INLINE bool dvmGetFieldBoolean(const Object* obj, int offset) {
    if (obj == 0)  return false;
    return ((JValue*)BYTE_OFFSET(obj, offset))->z;
}
INLINE s1 dvmGetFieldByte(const Object* obj, int offset) {
    if (obj == 0)  return (s1) 0;
    return ((JValue*)BYTE_OFFSET(obj, offset))->b;
}
INLINE s2 dvmGetFieldShort(const Object* obj, int offset) {
    if (obj == 0)  return (s2) 0;
    return ((JValue*)BYTE_OFFSET(obj, offset))->s;
}
INLINE u2 dvmGetFieldChar(const Object* obj, int offset) {
    if (obj == 0)  return (u2) 0;
    return ((JValue*)BYTE_OFFSET(obj, offset))->c;
}
INLINE s4 dvmGetFieldInt(const Object* obj, int offset) {
    if (obj == 0)  return (s4) 0;
    return ((JValue*)BYTE_OFFSET(obj, offset))->i;
}
INLINE s8 dvmGetFieldLong(const Object* obj, int offset) {
    if (obj == 0)  return (s8) 0;
    return ((JValue*)BYTE_OFFSET(obj, offset))->j;
}
INLINE float dvmGetFieldFloat(const Object* obj, int offset) {
    if (obj == 0)  return (float) 0;
    return ((JValue*)BYTE_OFFSET(obj, offset))->f;
}
INLINE double dvmGetFieldDouble(const Object* obj, int offset) {
    if (obj == 0)  return (double) 0;
    return ((JValue*)BYTE_OFFSET(obj, offset))->d;
}
INLINE Object* dvmGetFieldObject(const Object* obj, int offset) {
    return ((JValue*)BYTE_OFFSET(obj, offset))->l;
}
INLINE bool dvmGetFieldBooleanVolatile(const Object* obj, int offset) {
    if (obj == 0)  return false;
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    return (bool)android_atomic_acquire_load(ptr);
}
INLINE s1 dvmGetFieldByteVolatile(const Object* obj, int offset) {
    if (obj == 0)  return (s1) 0;
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    return (s1)android_atomic_acquire_load(ptr);
}
INLINE s2 dvmGetFieldShortVolatile(const Object* obj, int offset) {
    if (obj == 0)  return (s2) 0;
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    return (s2)android_atomic_acquire_load(ptr);
}
INLINE u2 dvmGetFieldCharVolatile(const Object* obj, int offset) {
    if (obj == 0)  return (u2) 0;
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    return (u2)android_atomic_acquire_load(ptr);
}
INLINE s4 dvmGetFieldIntVolatile(const Object* obj, int offset) {
    if (obj == 0)  return (s4) 0;
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    return android_atomic_acquire_load(ptr);
}
INLINE float dvmGetFieldFloatVolatile(const Object* obj, int offset) {
    if (obj == 0)  return (float) 0;
    union { s4 ival; float fval; } alias;
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    alias.ival = android_atomic_acquire_load(ptr);
    return alias.fval;
}
INLINE s8 dvmGetFieldLongVolatile(const Object* obj, int offset) {
    if (obj == 0)  return (s8) 0;
    const s8* addr = (const s8*)BYTE_OFFSET(obj, offset);
    s8 val = dvmQuasiAtomicRead64(addr);
    ANDROID_MEMBAR_FULL();
    return val;
}
INLINE double dvmGetFieldDoubleVolatile(const Object* obj, int offset) {
    if (obj == 0)  return (double) 0;
    union { s8 lval; double dval; } alias;
    const s8* addr = (const s8*)BYTE_OFFSET(obj, offset);
    alias.lval = dvmQuasiAtomicRead64(addr);
    ANDROID_MEMBAR_FULL();
    return alias.dval;
}
INLINE Object* dvmGetFieldObjectVolatile(const Object* obj, int offset) {
    if (obj == 0)  return (Object *) 0;
    Object** ptr = &((JValue*)BYTE_OFFSET(obj, offset))->l;
    return (Object*)android_atomic_acquire_load((int32_t*)ptr);
}

INLINE void dvmSetFieldBoolean(Object* obj, int offset, bool val) {
    if (obj == 0)  return;
    ((JValue*)BYTE_OFFSET(obj, offset))->i = val;
}
INLINE void dvmSetFieldByte(Object* obj, int offset, s1 val) {
    if (obj == 0)  return;
    ((JValue*)BYTE_OFFSET(obj, offset))->i = val;
}
INLINE void dvmSetFieldShort(Object* obj, int offset, s2 val) {
    if (obj == 0)  return;
    ((JValue*)BYTE_OFFSET(obj, offset))->i = val;
}
INLINE void dvmSetFieldChar(Object* obj, int offset, u2 val) {
    if (obj == 0)  return;
    ((JValue*)BYTE_OFFSET(obj, offset))->i = val;
}
INLINE void dvmSetFieldInt(Object* obj, int offset, s4 val) {
    if (obj == 0)  return;
    ((JValue*)BYTE_OFFSET(obj, offset))->i = val;
}
INLINE void dvmSetFieldFloat(Object* obj, int offset, float val) {
    if (obj == 0)  return;
    ((JValue*)BYTE_OFFSET(obj, offset))->f = val;
}
INLINE void dvmSetFieldLong(Object* obj, int offset, s8 val) {
    if (obj == 0)  return;
    ((JValue*)BYTE_OFFSET(obj, offset))->j = val;
}
INLINE void dvmSetFieldDouble(Object* obj, int offset, double val) {
    if (obj == 0)  return;
    ((JValue*)BYTE_OFFSET(obj, offset))->d = val;
}
INLINE void dvmSetFieldObject(Object* obj, int offset, Object* val) {
    if (obj == 0)  return;
    JValue* lhs = (JValue*)BYTE_OFFSET(obj, offset);
    lhs->l = val;
    if (val != NULL) {
        dvmWriteBarrierField(obj, &lhs->l);
    }
}
INLINE void dvmSetFieldIntVolatile(Object* obj, int offset, s4 val) {
    if (obj == 0)  return;
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    /*
     * TODO: add an android_atomic_synchronization_store() function and
     * use it in the 32-bit volatile set handlers.  On some platforms we
     * can use a fast atomic instruction and avoid the barriers.
     */
    ANDROID_MEMBAR_STORE();
    *ptr = val;
    ANDROID_MEMBAR_FULL();
}
INLINE void dvmSetFieldBooleanVolatile(Object* obj, int offset, bool val) {
    if (obj == 0)  return;
    dvmSetFieldIntVolatile(obj, offset, val);
}
INLINE void dvmSetFieldByteVolatile(Object* obj, int offset, s1 val) {
    if (obj == 0)  return;
    dvmSetFieldIntVolatile(obj, offset, val);
}
INLINE void dvmSetFieldShortVolatile(Object* obj, int offset, s2 val) {
    if (obj == 0)  return;
    dvmSetFieldIntVolatile(obj, offset, val);
}
INLINE void dvmSetFieldCharVolatile(Object* obj, int offset, u2 val) {
    if (obj == 0)  return;
    dvmSetFieldIntVolatile(obj, offset, val);
}
INLINE void dvmSetFieldFloatVolatile(Object* obj, int offset, float val) {
    if (obj == 0)  return;
    union { s4 ival; float fval; } alias;
    alias.fval = val;
    dvmSetFieldIntVolatile(obj, offset, alias.ival);
}
INLINE void dvmSetFieldLongVolatile(Object* obj, int offset, s8 val) {
    if (obj == 0)  return;
    s8* addr = (s8*)BYTE_OFFSET(obj, offset);
    dvmQuasiAtomicSwap64Sync(val, addr);
}
INLINE void dvmSetFieldDoubleVolatile(Object* obj, int offset, double val) {
    if (obj == 0)  return;
    union { s8 lval; double dval; } alias;
    alias.dval = val;
    dvmSetFieldLongVolatile(obj, offset, alias.lval);
}
INLINE void dvmSetFieldObjectVolatile(Object* obj, int offset, Object* val) {
    if (obj == 0)  return;
    Object** ptr = &((JValue*)BYTE_OFFSET(obj, offset))->l;
    ANDROID_MEMBAR_STORE();
    *ptr = val;
    ANDROID_MEMBAR_FULL();
    if (val != NULL) {
        dvmWriteBarrierField(obj, ptr);
    }
}

INLINE JValue* dvmStaticFieldPtr(const StaticField* sfield) {
    return (JValue*)&sfield->value;
}

INLINE bool dvmGetStaticFieldBoolean(const StaticField* sfield) {
    return sfield->value.z;
}
INLINE s1 dvmGetStaticFieldByte(const StaticField* sfield) {
    return sfield->value.b;
}
INLINE s2 dvmGetStaticFieldShort(const StaticField* sfield) {
    return sfield->value.s;
}
INLINE u2 dvmGetStaticFieldChar(const StaticField* sfield) {
    return sfield->value.c;
}
INLINE s4 dvmGetStaticFieldInt(const StaticField* sfield) {
    return sfield->value.i;
}
INLINE float dvmGetStaticFieldFloat(const StaticField* sfield) {
    return sfield->value.f;
}
INLINE s8 dvmGetStaticFieldLong(const StaticField* sfield) {
    return sfield->value.j;
}
INLINE double dvmGetStaticFieldDouble(const StaticField* sfield) {
    return sfield->value.d;
}
INLINE Object* dvmGetStaticFieldObject(const StaticField* sfield) {
    return sfield->value.l;
}
INLINE bool dvmGetStaticFieldBooleanVolatile(const StaticField* sfield) {
    const s4* ptr = &(sfield->value.i);
    return (bool)android_atomic_acquire_load((s4*)ptr);
}
INLINE s1 dvmGetStaticFieldByteVolatile(const StaticField* sfield) {
    const s4* ptr = &(sfield->value.i);
    return (s1)android_atomic_acquire_load((s4*)ptr);
}
INLINE s2 dvmGetStaticFieldShortVolatile(const StaticField* sfield) {
    const s4* ptr = &(sfield->value.i);
    return (s2)android_atomic_acquire_load((s4*)ptr);
}
INLINE u2 dvmGetStaticFieldCharVolatile(const StaticField* sfield) {
    const s4* ptr = &(sfield->value.i);
    return (u2)android_atomic_acquire_load((s4*)ptr);
}
INLINE s4 dvmGetStaticFieldIntVolatile(const StaticField* sfield) {
    const s4* ptr = &(sfield->value.i);
    return android_atomic_acquire_load((s4*)ptr);
}
INLINE float dvmGetStaticFieldFloatVolatile(const StaticField* sfield) {
    union { s4 ival; float fval; } alias;
    const s4* ptr = &(sfield->value.i);
    alias.ival = android_atomic_acquire_load((s4*)ptr);
    return alias.fval;
}
INLINE s8 dvmGetStaticFieldLongVolatile(const StaticField* sfield) {
    const s8* addr = &sfield->value.j;
    s8 val = dvmQuasiAtomicRead64(addr);
    ANDROID_MEMBAR_FULL();
    return val;
}
INLINE double dvmGetStaticFieldDoubleVolatile(const StaticField* sfield) {
    union { s8 lval; double dval; } alias;
    const s8* addr = &sfield->value.j;
    alias.lval = dvmQuasiAtomicRead64(addr);
    ANDROID_MEMBAR_FULL();
    return alias.dval;
}
INLINE Object* dvmGetStaticFieldObjectVolatile(const StaticField* sfield) {
    Object* const* ptr = &(sfield->value.l);
    return (Object*)android_atomic_acquire_load((int32_t*)ptr);
}

INLINE void dvmSetStaticFieldBoolean(StaticField* sfield, bool val) {
    sfield->value.i = val;
}
INLINE void dvmSetStaticFieldByte(StaticField* sfield, s1 val) {
    sfield->value.i = val;
}
INLINE void dvmSetStaticFieldShort(StaticField* sfield, s2 val) {
    sfield->value.i = val;
}
INLINE void dvmSetStaticFieldChar(StaticField* sfield, u2 val) {
    sfield->value.i = val;
}
INLINE void dvmSetStaticFieldInt(StaticField* sfield, s4 val) {
    sfield->value.i = val;
}
INLINE void dvmSetStaticFieldFloat(StaticField* sfield, float val) {
    sfield->value.f = val;
}
INLINE void dvmSetStaticFieldLong(StaticField* sfield, s8 val) {
    sfield->value.j = val;
}
INLINE void dvmSetStaticFieldDouble(StaticField* sfield, double val) {
    sfield->value.d = val;
}
INLINE void dvmSetStaticFieldObject(StaticField* sfield, Object* val) {
    sfield->value.l = val;
    if (val != NULL) {
        dvmWriteBarrierField(sfield->clazz, &sfield->value.l);
    }
}
INLINE void dvmSetStaticFieldIntVolatile(StaticField* sfield, s4 val) {
    s4* ptr = &sfield->value.i;
    ANDROID_MEMBAR_STORE();
    *ptr = val;
    ANDROID_MEMBAR_FULL();
}
INLINE void dvmSetStaticFieldBooleanVolatile(StaticField* sfield, bool val) {
    dvmSetStaticFieldIntVolatile(sfield, val);
}
INLINE void dvmSetStaticFieldByteVolatile(StaticField* sfield, s1 val) {
    dvmSetStaticFieldIntVolatile(sfield, val);
}
INLINE void dvmSetStaticFieldShortVolatile(StaticField* sfield, s2 val) {
    dvmSetStaticFieldIntVolatile(sfield, val);
}
INLINE void dvmSetStaticFieldCharVolatile(StaticField* sfield, u2 val) {
    dvmSetStaticFieldIntVolatile(sfield, val);
}
INLINE void dvmSetStaticFieldFloatVolatile(StaticField* sfield, float val) {
    union { s4 ival; float fval; } alias;
    alias.fval = val;
    dvmSetStaticFieldIntVolatile(sfield, alias.ival);
}
INLINE void dvmSetStaticFieldLongVolatile(StaticField* sfield, s8 val) {
    s8* addr = &sfield->value.j;
    dvmQuasiAtomicSwap64Sync(val, addr);
}
INLINE void dvmSetStaticFieldDoubleVolatile(StaticField* sfield, double val) {
    union { s8 lval; double dval; } alias;
    alias.dval = val;
    dvmSetStaticFieldLongVolatile(sfield, alias.lval);
}
INLINE void dvmSetStaticFieldObjectVolatile(StaticField* sfield, Object* val) {
    Object** ptr = &(sfield->value.l);
    ANDROID_MEMBAR_STORE();
    *ptr = val;
    ANDROID_MEMBAR_FULL();
    if (val != NULL) {
        dvmWriteBarrierField(sfield->clazz, &sfield->value.l);
    }
}

#endif  // DALVIK_OO_OBJECTINLINES_H_
