

#ifndef DALVIK_INLINENATIVE_H_
#define DALVIK_INLINENATIVE_H_

/* startup/shutdown */
bool dvmInlineNativeStartup(void);
void dvmInlineNativeShutdown(void);

Method* dvmFindInlinableMethod(const char* classDescriptor,
    const char* methodName, const char* methodSignature);

typedef bool (*InlineOp4Func)(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
    JValue* pResult);

struct InlineOperation {
    InlineOp4Func   func;               /* MUST be first entry */
    const char*     classDescriptor;
    const char*     methodName;
    const char*     methodSignature;
};

enum NativeInlineOps {
    INLINE_EMPTYINLINEMETHOD = 0,
    INLINE_STRING_CHARAT = 1,
    INLINE_STRING_COMPARETO = 2,
    INLINE_STRING_EQUALS = 3,
    INLINE_STRING_FASTINDEXOF_II = 4,
    INLINE_STRING_IS_EMPTY = 5,
    INLINE_STRING_LENGTH = 6,
    INLINE_MATH_ABS_INT = 7,
    INLINE_MATH_ABS_LONG = 8,
    INLINE_MATH_ABS_FLOAT = 9,
    INLINE_MATH_ABS_DOUBLE = 10,
    INLINE_MATH_MIN_INT = 11,
    INLINE_MATH_MAX_INT = 12,
    INLINE_MATH_SQRT = 13,
    INLINE_MATH_COS = 14,
    INLINE_MATH_SIN = 15,
    INLINE_FLOAT_TO_INT_BITS = 16,
    INLINE_FLOAT_TO_RAW_INT_BITS = 17,
    INLINE_INT_BITS_TO_FLOAT = 18,
    INLINE_DOUBLE_TO_LONG_BITS = 19,
    INLINE_DOUBLE_TO_RAW_LONG_BITS = 20,
    INLINE_LONG_BITS_TO_DOUBLE = 21,
};

const InlineOperation* dvmGetInlineOpsTable(void);
int dvmGetInlineOpsTableLength(void);

extern const InlineOperation gDvmInlineOpsTable[];

INLINE bool dvmPerformInlineOp4Std(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
    JValue* pResult, int opIndex)
{
    return (*gDvmInlineOpsTable[opIndex].func)(arg0, arg1, arg2, arg3, pResult);
}

bool dvmPerformInlineOp4Dbg(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
    JValue* pResult, int opIndex);

extern "C" Method* dvmResolveInlineNative(int opIndex);

bool javaLangString_charAt(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                           JValue* pResult);

bool javaLangString_compareTo(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                              JValue* pResult);

bool javaLangString_equals(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                           JValue* pResult);

bool javaLangString_length(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                           JValue* pResult);

bool javaLangString_isEmpty(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                            JValue* pResult);

bool javaLangString_fastIndexOf_II(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                                   JValue* pResult);

bool javaLangMath_abs_int(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                          JValue* pResult);

bool javaLangMath_abs_long(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                           JValue* pResult);

bool javaLangMath_abs_float(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                            JValue* pResult);

bool javaLangMath_abs_double(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                             JValue* pResult);

bool javaLangMath_min_int(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                          JValue* pResult);

bool javaLangMath_max_int(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                          JValue* pResult);

bool javaLangMath_sqrt(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                       JValue* pResult);

bool javaLangMath_cos(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                      JValue* pResult);

bool javaLangMath_sin(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
                      JValue* pResult);

bool javaLangFloat_floatToIntBits(u4 arg0, u4 arg1, u4 arg2, u4 arg,
                                  JValue* pResult);

bool javaLangFloat_floatToRawIntBits(u4 arg0, u4 arg1, u4 arg2, u4 arg,
                                     JValue* pResult);

bool javaLangFloat_intBitsToFloat(u4 arg0, u4 arg1, u4 arg2, u4 arg,
                                  JValue* pResult);

bool javaLangDouble_doubleToLongBits(u4 arg0, u4 arg1, u4 arg2, u4 arg,
                                     JValue* pResult);

bool javaLangDouble_longBitsToDouble(u4 arg0, u4 arg1, u4 arg2, u4 arg,
                                     JValue* pResult);

bool javaLangDouble_doubleToRawLongBits(u4 arg0, u4 arg1, u4 arg2,
                                        u4 arg, JValue* pResult);

bool javaLangDouble_longBitsToDouble(u4 arg0, u4 arg1, u4 arg2, u4 arg,
                                     JValue* pResult);

#endif  // DALVIK_INLINENATIVE_H_
