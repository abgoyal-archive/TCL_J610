
#ifndef DALVIK_OO_ACCESSCHECK_H_
#define DALVIK_OO_ACCESSCHECK_H_

bool dvmCheckClassAccess(const ClassObject* accessFrom,
    const ClassObject* clazz);

bool dvmCheckMethodAccess(const ClassObject* accessFrom, const Method* method);

bool dvmCheckFieldAccess(const ClassObject* accessFrom, const Field* field);

bool dvmInSamePackage(const ClassObject* class1, const ClassObject* class2);

#endif  // DALVIK_OO_ACCESSCHECK_H_
