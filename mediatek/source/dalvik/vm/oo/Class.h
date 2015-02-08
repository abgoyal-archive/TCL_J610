
#ifndef DALVIK_OO_CLASS_H_
#define DALVIK_OO_CLASS_H_

enum ClassPathEntryKind {
    kCpeUnknown = 0,
    kCpeJar,
    kCpeDex,
    kCpeLastEntry       /* used as sentinel at end of array */
};

struct ClassPathEntry {
    ClassPathEntryKind kind;
    char*   fileName;
    void*   ptr;            /* JarFile* or DexFile* */
};

bool dvmClassStartup(void);
void dvmClassShutdown(void);
bool dvmPrepBootClassPath(bool isNormalStart);

int dvmGetBootPathSize(void);
StringObject* dvmGetBootPathResource(const char* name, int idx);
void dvmDumpBootClassPath(void);

bool dvmClassPathContains(const ClassPathEntry* cpe, const char* path);

void dvmSetClassSerialNumber(ClassObject* clazz);

ClassObject* dvmFindPrimitiveClass(char type);

ClassObject* dvmFindClass(const char* descriptor, Object* loader);
ClassObject* dvmFindClassNoInit(const char* descriptor, Object* loader);

ClassObject* dvmFindSystemClass(const char* descriptor);
ClassObject* dvmFindSystemClassNoInit(const char* descriptor);

ClassObject* dvmFindLoadedClass(const char* descriptor);

ClassObject* dvmDefineClass(DvmDex* pDvmDex, const char* descriptor,
    Object* classLoader);

bool dvmLinkClass(ClassObject* clazz);

INLINE bool dvmIsClassInitialized(const ClassObject* clazz) {
    return (clazz->status == CLASS_INITIALIZED);
}
bool dvmIsClassInitializing(const ClassObject* clazz);

extern "C" bool dvmInitClass(ClassObject* clazz);

Object* dvmGetSystemClassLoader(void);

ClassObject* dvmLookupClass(const char* descriptor, Object* loader,
    bool unprepOkay);
void dvmFreeClassInnards(ClassObject* clazz);
bool dvmAddClassToHash(ClassObject* clazz);
void dvmAddInitiatingLoader(ClassObject* clazz, Object* loader);
bool dvmLoaderInInitiatingList(const ClassObject* clazz, const Object* loader);

void dvmSetNativeFunc(Method* method, DalvikBridgeFunc func, const u2* insns);

void dvmSetRegisterMap(Method* method, const RegisterMap* pMap);

void dvmMakeCodeReadWrite(Method* meth);
void dvmMakeCodeReadOnly(Method* meth);

void dvmSetBootPathExtraDex(DvmDex* pDvmDex);

void dvmDumpClass(const ClassObject* clazz, int flags);
void dvmDumpAllClasses(int flags);
void dvmDumpLoaderStats(const char* msg);
int  dvmGetNumLoadedClasses();

/* flags for dvmDumpClass / dvmDumpAllClasses */
#define kDumpClassFullDetail    1
#define kDumpClassClassLoader   (1 << 1)
#define kDumpClassInitialized   (1 << 2)


INLINE char* dvmCopyDescriptorStringFromMethod(const Method* method,
        DexStringCache *pCache)
{
    const char* result =
        dexProtoGetMethodDescriptor(&method->prototype, pCache);
    return dexStringCacheEnsureCopy(pCache, result);
}

INLINE int dvmComputeMethodArgsSize(const Method* method)
{
    return dexProtoComputeArgsSize(&method->prototype);
}

INLINE int dvmCompareMethodProtos(const Method* method1,
        const Method* method2)
{
    return dexProtoCompare(&method1->prototype, &method2->prototype);
}

INLINE int dvmCompareMethodParameterProtos(const Method* method1,
        const Method* method2)
{
    return dexProtoCompareParameters(&method1->prototype, &method2->prototype);
}

int dvmCompareMethodNamesAndProtos(const Method* method1,
        const Method* method2);

int dvmCompareMethodNamesAndParameterProtos(const Method* method1,
        const Method* method2);

INLINE int dvmCompareDescriptorAndMethodProto(const char* descriptor,
    const Method* method)
{
    // Sense is reversed.
    return -dexProtoCompareToDescriptor(&method->prototype, descriptor);
}

int dvmCompareNameProtoAndMethod(const char* name,
    const DexProto* proto, const Method* method);

int dvmCompareNameDescriptorAndMethod(const char* name,
    const char* descriptor, const Method* method);

size_t dvmClassObjectSize(const ClassObject *clazz);

#endif  // DALVIK_OO_CLASS_H_
