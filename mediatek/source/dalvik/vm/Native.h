
#ifndef DALVIK_NATIVE_H_
#define DALVIK_NATIVE_H_

struct DalvikNativeMethod {
    const char* name;
    const char* signature;
    DalvikNativeFunc  fnPtr;
};

struct DalvikNativeClass {
    const char* classDescriptor;
    const DalvikNativeMethod* methodInfo;
    u4          classDescriptorHash;          /* initialized at runtime */
};


/* init/shutdown */
bool dvmNativeStartup(void);
void dvmNativeShutdown(void);


extern "C" void dvmPlatformInvoke(void* pEnv, ClassObject* clazz, int argInfo,
    int argc, const u4* argv, const char* signature, void* func, JValue* pResult);

u4 dvmPlatformInvokeHints(const DexProto* proto);

char* dvmCreateSystemLibraryName(char* libName);
bool dvmLoadNativeCode(const char* fileName, Object* classLoader,
        char** detail);


void dvmResolveNativeMethod(const u4* args, JValue* pResult,
    const Method* method, struct Thread* self);

void dvmUnregisterJNINativeMethods(ClassObject* clazz);

#define GET_ARG_LONG(_args, _elem)          dvmGetArgLong(_args, _elem)

INLINE s8 dvmGetArgLong(const u4* args, int elem)
{
    s8 val;
    memcpy(&val, &args[elem], sizeof(val));
    return val;
}

#endif  // DALVIK_NATIVE_H_
