

#ifndef DALVIK_NATIVE_INTERNALNATIVE_H_
#define DALVIK_NATIVE_INTERNALNATIVE_H_

bool dvmInternalNativeStartup(void);
void dvmInternalNativeShutdown(void);

/* search the internal native set for a match */
DalvikNativeFunc dvmLookupInternalNativeMethod(const Method* method);

/* exception-throwing stub for abstract methods (DalvikNativeFunc) */
extern "C" void dvmAbstractMethodStub(const u4* args, JValue* pResult);

#endif  // DALVIK_NATIVE_INTERNALNATIVE_H_
