

#ifndef DALVIK_NATIVE_INTERNALNATIVEPRIV_H_
#define DALVIK_NATIVE_INTERNALNATIVEPRIV_H_

#ifdef NDEBUG
# define RETURN_VOID()           do { (void)(pResult); return; } while(0)
#else
# define RETURN_VOID()           do { pResult->i = 0xfefeabab; return; }while(0)
#endif
#define RETURN_BOOLEAN(_val)    do { pResult->i = (_val); return; } while(0)
#define RETURN_INT(_val)        do { pResult->i = (_val); return; } while(0)
#define RETURN_LONG(_val)       do { pResult->j = (_val); return; } while(0)
#define RETURN_FLOAT(_val)      do { pResult->f = (_val); return; } while(0)
#define RETURN_DOUBLE(_val)     do { pResult->d = (_val); return; } while(0)
#define RETURN_PTR(_val)        do { pResult->l = (Object*)(_val); return; } while(0)

#define MAKE_INTRINSIC_TRAMPOLINE(INTRINSIC_FN) \
    extern bool INTRINSIC_FN(u4 arg0, u4 arg1, u4 arg2, u4 arg3, \
            JValue* pResult); \
    INTRINSIC_FN(args[0], args[1], args[2], args[3], pResult);

bool dvmVerifyObjectInClass(Object* obj, ClassObject* clazz);

ClassObject* dvmFindClassByName(StringObject* nameObj, Object* loader,
    bool doInit);

u4 dvmFixMethodFlags(u4 flags);

void dvmFreeDexOrJar(void* vptr);

extern const DalvikNativeMethod dvm_java_lang_Object[];
extern const DalvikNativeMethod dvm_java_lang_Class[];
extern const DalvikNativeMethod dvm_java_lang_Double[];
extern const DalvikNativeMethod dvm_java_lang_Float[];
extern const DalvikNativeMethod dvm_java_lang_Math[];
extern const DalvikNativeMethod dvm_java_lang_Runtime[];
extern const DalvikNativeMethod dvm_java_lang_String[];
extern const DalvikNativeMethod dvm_java_lang_System[];
extern const DalvikNativeMethod dvm_java_lang_Throwable[];
extern const DalvikNativeMethod dvm_java_lang_VMClassLoader[];
extern const DalvikNativeMethod dvm_java_lang_VMThread[];
extern const DalvikNativeMethod dvm_java_lang_reflect_AccessibleObject[];
extern const DalvikNativeMethod dvm_java_lang_reflect_Array[];
extern const DalvikNativeMethod dvm_java_lang_reflect_Constructor[];
extern const DalvikNativeMethod dvm_java_lang_reflect_Field[];
extern const DalvikNativeMethod dvm_java_lang_reflect_Method[];
extern const DalvikNativeMethod dvm_java_lang_reflect_Proxy[];
extern const DalvikNativeMethod dvm_java_util_concurrent_atomic_AtomicLong[];
extern const DalvikNativeMethod dvm_dalvik_bytecode_OpcodeInfo[];
extern const DalvikNativeMethod dvm_dalvik_system_SamplingProfiler[];
extern const DalvikNativeMethod dvm_dalvik_system_VMDebug[];
extern const DalvikNativeMethod dvm_dalvik_system_DexFile[];
extern const DalvikNativeMethod dvm_dalvik_system_VMRuntime[];
extern const DalvikNativeMethod dvm_dalvik_system_Zygote[];
extern const DalvikNativeMethod dvm_dalvik_system_VMStack[];
extern const DalvikNativeMethod dvm_org_apache_harmony_dalvik_ddmc_DdmServer[];
extern const DalvikNativeMethod dvm_org_apache_harmony_dalvik_ddmc_DdmVmInternal[];
extern const DalvikNativeMethod dvm_org_apache_harmony_dalvik_NativeTestTarget[];
extern const DalvikNativeMethod dvm_sun_misc_Unsafe[];

#endif  // DALVIK_NATIVE_INTERNALNATIVEPRIV_H_
