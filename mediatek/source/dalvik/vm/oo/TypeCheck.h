
#ifndef DALVIK_OO_TYPECHECK_H_
#define DALVIK_OO_TYPECHECK_H_

/* VM startup/shutdown */
bool dvmInstanceofStartup(void);
void dvmInstanceofShutdown(void);


/* used by dvmInstanceof; don't call */
extern "C" int dvmInstanceofNonTrivial(const ClassObject* instance,
                                       const ClassObject* clazz);

INLINE int dvmInstanceof(const ClassObject* instance, const ClassObject* clazz)
{
    if (instance == clazz) {
        if (CALC_CACHE_STATS)
            gDvm.instanceofCache->trivial++;
        return 1;
    } else
        return dvmInstanceofNonTrivial(instance, clazz);
}

int dvmImplements(const ClassObject* clazz, const ClassObject* interface);

INLINE int dvmIsSubClass(const ClassObject* sub, const ClassObject* clazz) {
    do {
        /*printf("###### sub='%s' clazz='%s'\n", sub->name, clazz->name);*/
        if (sub == clazz)
            return 1;
        sub = sub->super;
    } while (sub != NULL);

    return 0;
}

extern "C" bool dvmCanPutArrayElement(const ClassObject* elemClass,
    const ClassObject* arrayClass);

#endif  // DALVIK_OO_TYPECHECK_H_
