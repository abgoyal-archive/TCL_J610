
#ifndef DALVIK_OO_RESOLVE_H_
#define DALVIK_OO_RESOLVE_H_

enum MethodType {
    METHOD_UNKNOWN  = 0,
    METHOD_DIRECT,      // <init>, private
    METHOD_STATIC,      // static
    METHOD_VIRTUAL,     // virtual, super
    METHOD_INTERFACE    // interface
};

extern "C" ClassObject* dvmResolveClass(const ClassObject* referrer,
                                        u4 classIdx,
                                        bool fromUnverifiedConstant);

extern "C" Method* dvmResolveMethod(const ClassObject* referrer, u4 methodIdx,
                                    MethodType methodType);

Method* dvmResolveInterfaceMethod(const ClassObject* referrer, u4 methodIdx);

extern "C" InstField* dvmResolveInstField(const ClassObject* referrer,
                                          u4 ifieldIdx);

extern "C" StaticField* dvmResolveStaticField(const ClassObject* referrer,
                                              u4 sfieldIdx);

extern "C" StringObject* dvmResolveString(const ClassObject* referrer, u4 stringIdx);

const char* dvmMethodTypeStr(MethodType methodType);

#endif  // DALVIK_OO_RESOLVE_H_
