

#ifndef DALVIK_ALLOC_VISIT_H_
#define DALVIK_ALLOC_VISIT_H_

#include "Dalvik.h"

enum RootType {
  ROOT_UNKNOWN = 0,
  ROOT_JNI_GLOBAL,
  ROOT_JNI_LOCAL,
  ROOT_JAVA_FRAME,
  ROOT_NATIVE_STACK,
  ROOT_STICKY_CLASS,
  ROOT_THREAD_BLOCK,
  ROOT_MONITOR_USED,
  ROOT_THREAD_OBJECT,
  ROOT_INTERNED_STRING,
  ROOT_DEBUGGER,
  ROOT_VM_INTERNAL,
  ROOT_JNI_MONITOR,
};

typedef void Visitor(void *addr, void *arg);

typedef void RootVisitor(void *addr, u4 threadId, RootType type, void *arg);

void dvmVisitObject(Visitor *visitor, Object *obj, void *arg);

void dvmVisitRoots(RootVisitor *visitor, void *arg);

#endif  // DALVIK_ALLOC_VISIT_H_
