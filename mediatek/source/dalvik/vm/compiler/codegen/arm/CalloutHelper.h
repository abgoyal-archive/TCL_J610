

#ifndef DALVIK_VM_COMPILER_CODEGEN_ARM_CALLOUT_HELPER_H_
#define DALVIK_VM_COMPILER_CODEGEN_ARM_CALLOUT_HELPER_H_

#include "Dalvik.h"


/* Load a statically compiled function address as a constant */
#define LOAD_FUNC_ADDR(cUnit, reg, addr) loadConstant(cUnit, reg, addr)

/* Conversions */
extern "C" float __aeabi_i2f(int op1);             // OP_INT_TO_FLOAT
extern "C" int __aeabi_f2iz(float op1);            // OP_FLOAT_TO_INT
extern "C" float __aeabi_d2f(double op1);          // OP_DOUBLE_TO_FLOAT
extern "C" double __aeabi_f2d(float op1);          // OP_FLOAT_TO_DOUBLE
extern "C" double __aeabi_i2d(int op1);            // OP_INT_TO_DOUBLE
extern "C" int __aeabi_d2iz(double op1);           // OP_DOUBLE_TO_INT
extern "C" float __aeabi_l2f(long op1);            // OP_LONG_TO_FLOAT
extern "C" double __aeabi_l2d(long op1);           // OP_LONG_TO_DOUBLE
s8 dvmJitf2l(float op1);                // OP_FLOAT_TO_LONG
s8 dvmJitd2l(double op1);               // OP_DOUBLE_TO_LONG

/* Single-precision FP arithmetics */
extern "C" float __aeabi_fadd(float a, float b);   // OP_ADD_FLOAT[_2ADDR]
extern "C" float __aeabi_fsub(float a, float b);   // OP_SUB_FLOAT[_2ADDR]
extern "C" float __aeabi_fdiv(float a, float b);   // OP_DIV_FLOAT[_2ADDR]
extern "C" float __aeabi_fmul(float a, float b);   // OP_MUL_FLOAT[_2ADDR]
extern "C" float fmodf(float a, float b);          // OP_REM_FLOAT[_2ADDR]

/* Double-precision FP arithmetics */
extern "C" double __aeabi_dadd(double a, double b); // OP_ADD_DOUBLE[_2ADDR]
extern "C" double __aeabi_dsub(double a, double b); // OP_SUB_DOUBLE[_2ADDR]
extern "C" double __aeabi_ddiv(double a, double b); // OP_DIV_DOUBLE[_2ADDR]
extern "C" double __aeabi_dmul(double a, double b); // OP_MUL_DOUBLE[_2ADDR]
extern "C" double fmod(double a, double b);         // OP_REM_DOUBLE[_2ADDR]

/* Integer arithmetics */
extern "C" int __aeabi_idivmod(int op1, int op2);  // OP_REM_INT[_2ADDR|_LIT8|_LIT16]
extern "C" int __aeabi_idiv(int op1, int op2);     // OP_DIV_INT[_2ADDR|_LIT8|_LIT16]

/* Long long arithmetics - OP_REM_LONG[_2ADDR] & OP_DIV_LONG[_2ADDR] */
extern "C" long long __aeabi_ldivmod(long long op1, long long op2);

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

/* Originally declared in compiler/codegen/arm/Assemble.c */
const Method *dvmJitToPatchPredictedChain(const Method *method,
                                          Thread *self,
                                          PredictedChainingCell *cell,
                                          const ClassObject *clazz);



/* Originally declared in alloc/Alloc.h */
Object* dvmAllocObject(ClassObject* clazz, int flags);  // OP_NEW_INSTANCE

extern "C" double sqrt(double x);  // INLINE_MATH_SQRT


#endif  // DALVIK_VM_COMPILER_CODEGEN_ARM_CALLOUT_HELPER_H_
