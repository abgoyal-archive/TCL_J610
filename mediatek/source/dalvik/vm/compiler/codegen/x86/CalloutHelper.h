

#ifndef DALVIK_VM_COMPILER_CODEGEN_X86_CALLOUT_HELPER_H_
#define DALVIK_VM_COMPILER_CODEGEN_X86_CALLOUT_HELPER_H_

#include "Dalvik.h"


/* Load a statically compiled function address as a constant */
#define LOAD_FUNC_ADDR(cUnit, reg, addr) loadConstant(cUnit, reg, addr)

/* Originally declared in Sync.h */
bool dvmUnlockObject(struct Thread* self, struct Object* obj); //OP_MONITOR_EXIT

/* Originally declared in oo/TypeCheck.h */
bool dvmCanPutArrayElement(const ClassObject* elemClass,   // OP_APUT_OBJECT
                           const ClassObject* arrayClass);
int dvmInstanceofNonTrivial(const ClassObject* instance,   // OP_CHECK_CAST &&
                            const ClassObject* clazz);     // OP_INSTANCE_OF

/* Originally declared in oo/Array.h */
ArrayObject* dvmAllocArrayByClass(ClassObject* arrayClass, // OP_NEW_ARRAY
                                  size_t length, int allocFlags);

/* Originally declared in interp/InterpDefs.h */
bool dvmInterpHandleFillArrayData(ArrayObject* arrayObject,// OP_FILL_ARRAY_DATA
                                  const u2* arrayData);

/* Originally declared in alloc/Alloc.h */
Object* dvmAllocObject(ClassObject* clazz, int flags);  // OP_NEW_INSTANCE

extern "C" double sqrt(double x);  // INLINE_MATH_SQRT

#endif  // DALVIK_VM_COMPILER_CODEGEN_X86_CALLOUT_HELPER_H_
