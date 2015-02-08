

#ifndef DALVIK_OPTIMIZE_H_
#define DALVIK_OPTIMIZE_H_

void dvmOptimizeClass(ClassObject* clazz, bool essentialOnly);

void dvmUpdateCodeUnit(const Method* meth, u2* ptr, u2 newVal);

ClassObject* dvmOptResolveClass(ClassObject* referrer, u4 classIdx,
    VerifyError* pFailure);
Method* dvmOptResolveMethod(ClassObject* referrer, u4 methodIdx,
    MethodType methodType, VerifyError* pFailure);
Method* dvmOptResolveInterfaceMethod(ClassObject* referrer, u4 methodIdx);
InstField* dvmOptResolveInstField(ClassObject* referrer, u4 ifieldIdx,
    VerifyError* pFailure);
StaticField* dvmOptResolveStaticField(ClassObject* referrer, u4 sfieldIdx,
    VerifyError* pFailure);

#endif  // DALVIK_OPTIMIZE_H_
