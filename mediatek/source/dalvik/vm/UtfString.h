

#ifndef DALVIK_STRING_H_
#define DALVIK_STRING_H_

#include <string>
#include <vector>

//#define USE_GLOBAL_STRING_DEFS
#ifdef USE_GLOBAL_STRING_DEFS
# define STRING_FIELDOFF_VALUE      gDvm.offJavaLangString_value
# define STRING_FIELDOFF_OFFSET     gDvm.offJavaLangString_offset
# define STRING_FIELDOFF_COUNT      gDvm.offJavaLangString_count
# define STRING_FIELDOFF_HASHCODE   gDvm.offJavaLangString_hashCode
#else
# define STRING_FIELDOFF_VALUE      8
# define STRING_FIELDOFF_HASHCODE   12
# define STRING_FIELDOFF_OFFSET     16
# define STRING_FIELDOFF_COUNT      20
#endif

u4 dvmComputeUtf8Hash(const char* str);

u4 dvmComputeStringHash(StringObject* strObj);

ArrayObject* dvmCreateStringArray(const std::vector<std::string>& strings);

StringObject* dvmCreateStringFromCstr(const char* utf8Str);

StringObject* dvmCreateStringFromCstr(const std::string& utf8Str);

StringObject* dvmCreateStringFromCstrAndLength(const char* utf8Str,
    u4 utf16Length);

size_t dvmUtf8Len(const char* utf8Str);

void dvmConvertUtf8ToUtf16(u2* utf16Str, const char* utf8Str);

StringObject* dvmCreateStringFromUnicode(const u2* unichars, int len);

char* dvmCreateCstrFromString(const StringObject* jstr);

void dvmGetStringUtfRegion(const StringObject* jstr,
        int start, int len, char* buf);

int dvmHashcmpStrings(const void* vstrObj1, const void* vstrObj2);

#endif  // DALVIK_STRING_H_
