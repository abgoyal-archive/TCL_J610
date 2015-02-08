

#ifndef DALVIK_EXCEPTION_H_
#define DALVIK_EXCEPTION_H_

void dvmThrowChainedException(ClassObject* exceptionClass,
    const char* msg, Object* cause);
INLINE void dvmThrowException(ClassObject* exceptionClass,
    const char* msg)
{
    dvmThrowChainedException(exceptionClass, msg, NULL);
}

void dvmThrowExceptionFmtV(ClassObject* exceptionClass,
    const char* fmt, va_list args);
void dvmThrowExceptionFmt(ClassObject* exceptionClass,
    const char* fmt, ...)
#if defined(__GNUC__)
    __attribute__ ((format(printf, 2, 3)))
#endif
    ;
INLINE void dvmThrowExceptionFmt(ClassObject* exceptionClass,
    const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    dvmThrowExceptionFmtV(exceptionClass, fmt, args);
    va_end(args);
}

void dvmThrowChainedExceptionWithClassMessage(
    ClassObject* exceptionClass, const char* messageDescriptor,
    Object* cause);

INLINE void dvmThrowExceptionWithClassMessage(
    ClassObject* exceptionClass, const char* messageDescriptor)
{
    dvmThrowChainedExceptionWithClassMessage(exceptionClass,
            messageDescriptor, NULL);
}

INLINE Object* dvmGetException(Thread* self) {
    return self->exception;
}

INLINE void dvmSetException(Thread* self, Object* exception)
{
    assert(exception != NULL);
    self->exception = exception;
}

INLINE void dvmClearException(Thread* self) {
    self->exception = NULL;
}

void dvmClearOptException(Thread* self);

INLINE bool dvmCheckException(Thread* self) {
    return (self->exception != NULL);
}

bool dvmIsCheckedException(const Object* exception);

void dvmWrapException(const char* newExcepStr);

Object* dvmGetExceptionCause(const Object* exception);

void dvmPrintExceptionStackTrace(void);

void dvmLogExceptionStackTrace(void);

extern "C" int dvmFindCatchBlock(Thread* self, int relPc, Object* exception,
    bool doUnroll, void** newFrame);

void* dvmFillInStackTraceInternal(Thread* thread, bool wantObject, size_t* pCount);
/* return an [I for use by interpreted code */
INLINE Object* dvmFillInStackTrace(Thread* thread) {
    return (Object*) dvmFillInStackTraceInternal(thread, true, NULL);
}
ArrayObject* dvmGetStackTrace(const Object* stackState);
/* return an int* and array count; caller must free() the return value */
INLINE int* dvmFillInStackTraceRaw(Thread* thread, size_t* pCount) {
    return (int*) dvmFillInStackTraceInternal(thread, false, pCount);
}
ArrayObject* dvmGetStackTraceRaw(const int* intVals, size_t stackDepth);
void dvmFillStackTraceElements(const int* intVals, size_t stackDepth, ArrayObject* steArray);

void dvmLogRawStackTrace(const int* intVals, int stackDepth);

void dvmThrowAbstractMethodError(const char* msg);

extern "C" void dvmThrowArithmeticException(const char* msg);

extern "C" void dvmThrowArrayIndexOutOfBoundsException(int length, int index);

extern "C" void dvmThrowArrayStoreExceptionIncompatibleElement(ClassObject* objectType, ClassObject* arrayType);

void dvmThrowArrayStoreExceptionNotArray(ClassObject* actual, const char* label);

void dvmThrowArrayStoreExceptionIncompatibleArrays(ClassObject* source, ClassObject* destination);

void dvmThrowArrayStoreExceptionIncompatibleArrayElement(s4 index, ClassObject* objectType,
        ClassObject* arrayType);

extern "C" void dvmThrowClassCastException(ClassObject* actual, ClassObject* desired);

void dvmThrowClassCircularityError(const char* descriptor);

void dvmThrowClassFormatError(const char* msg);

void dvmThrowClassNotFoundException(const char* name);

void dvmThrowChainedClassNotFoundException(const char* name, Object* cause);

void dvmThrowExceptionInInitializerError(void);

void dvmThrowFileNotFoundException(const char* msg);

void dvmThrowIOException(const char* msg);

void dvmThrowIllegalAccessError(const char* msg);

void dvmThrowIllegalAccessException(const char* msg);

void dvmThrowIllegalArgumentException(const char* msg);

void dvmThrowIllegalMonitorStateException(const char* msg);

void dvmThrowIllegalStateException(const char* msg);

void dvmThrowIllegalThreadStateException(const char* msg);

void dvmThrowIncompatibleClassChangeError(const char* msg);

void dvmThrowIncompatibleClassChangeErrorWithClassMessage(
        const char* descriptor);

void dvmThrowInstantiationException(ClassObject* clazz,
        const char* extraDetail);

extern "C" void dvmThrowInternalError(const char* msg);

void dvmThrowInterruptedException(const char* msg);

void dvmThrowLinkageError(const char* msg);

extern "C" void dvmThrowNegativeArraySizeException(s4 size);

extern "C" void dvmThrowNoClassDefFoundError(const char* descriptor);

void dvmThrowChainedNoClassDefFoundError(const char* descriptor,
        Object* cause);

extern "C" void dvmThrowNoSuchFieldError(const char* msg);

void dvmThrowNoSuchFieldException(const char* msg);

extern "C" void dvmThrowNoSuchMethodError(const char* msg);

extern "C" void dvmThrowNullPointerException(const char* msg);

void dvmThrowOutOfMemoryError(const char* msg);

void dvmThrowRuntimeException(const char* msg);

void dvmThrowStaleDexCacheError(const char* msg);

void dvmThrowStringIndexOutOfBoundsExceptionWithIndex(jsize stringLength,
        jsize requestIndex);

void dvmThrowStringIndexOutOfBoundsExceptionWithRegion(jsize stringLength,
        jsize requestStart, jsize requestLength);

void dvmThrowTypeNotPresentException(const char* descriptor);

void dvmThrowUnsatisfiedLinkError(const char* msg);

void dvmThrowUnsupportedOperationException(const char* msg);

void dvmThrowVerifyError(const char* descriptor);

void dvmThrowVirtualMachineError(const char* msg);

#endif  // DALVIK_EXCEPTION_H_
