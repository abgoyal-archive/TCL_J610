
#ifndef DALVIK_JNIINTERNAL_H_
#define DALVIK_JNIINTERNAL_H_

#include "jni.h"

/* system init/shutdown */
bool dvmJniStartup(void);
void dvmJniShutdown(void);

struct JavaVMExt;

struct JNIEnvExt {
    const struct JNINativeInterface* funcTable;     /* must be first */

    const struct JNINativeInterface* baseFuncTable;

    u4      envThreadId;
    Thread* self;

    /* if nonzero, we are in a "critical" JNI call */
    int     critical;

    struct JNIEnvExt* prev;
    struct JNIEnvExt* next;
};

struct JavaVMExt {
    const struct JNIInvokeInterface* funcTable;     /* must be first */

    const struct JNIInvokeInterface* baseFuncTable;

    /* head of list of JNIEnvs associated with this VM */
    JNIEnvExt*      envList;
    pthread_mutex_t envListLock;
};

enum DalvikJniReturnType {
    DALVIK_JNI_RETURN_VOID = 0,     /* must be zero */
    DALVIK_JNI_RETURN_FLOAT = 1,
    DALVIK_JNI_RETURN_DOUBLE = 2,
    DALVIK_JNI_RETURN_S8 = 3,
    DALVIK_JNI_RETURN_S4 = 4,
    DALVIK_JNI_RETURN_S2 = 5,
    DALVIK_JNI_RETURN_U2 = 6,
    DALVIK_JNI_RETURN_S1 = 7
};

#define DALVIK_JNI_NO_ARG_INFO  0x80000000
#define DALVIK_JNI_RETURN_MASK  0x70000000
#define DALVIK_JNI_RETURN_SHIFT 28
#define DALVIK_JNI_COUNT_MASK   0x0f000000
#define DALVIK_JNI_COUNT_SHIFT  24


INLINE void dvmPopJniLocals(Thread* self, StackSaveArea* saveArea)
{
    self->jniLocalRefTable.segmentState.all = saveArea->xtra.localRefCookie;
}

INLINE void dvmSetJniEnvThreadId(JNIEnv* pEnv, Thread* self)
{
    ((JNIEnvExt*)pEnv)->envThreadId = self->threadId;
    ((JNIEnvExt*)pEnv)->self = self;
}

void dvmCallJNIMethod(const u4* args, JValue* pResult,
    const Method* method, Thread* self);
void dvmCheckCallJNIMethod(const u4* args, JValue* pResult,
    const Method* method, Thread* self);

void dvmUseJNIBridge(Method* method, void* func);


void dvmUseCheckedJniEnv(JNIEnvExt* pEnv);
void dvmUseCheckedJniVm(JavaVMExt* pVm);
void dvmLateEnableCheckedJni(void);

Object* dvmDecodeIndirectRef(Thread* self, jobject jobj);

jobjectRefType dvmGetJNIRefType(Thread* self, jobject jobj);

const Method* dvmGetCurrentJNIMethod(void);

JNIEnv* dvmCreateJNIEnv(Thread* self);
void dvmDestroyJNIEnv(JNIEnv* env);

JNIEnvExt* dvmGetJNIEnvForThread(void);

void dvmReleaseJniMonitors(Thread* self);

void dvmDumpJniReferenceTables(void);

#endif  // DALVIK_JNIINTERNAL_H_
