


#ifndef LIBDEX_DEXUTF_H_
#define LIBDEX_DEXUTF_H_

#include "DexFile.h"

DEX_INLINE u2 dexGetUtf16FromUtf8(const char** pUtf8Ptr)
{
    unsigned int one, two, three;

    one = *(*pUtf8Ptr)++;
    if ((one & 0x80) != 0) {
        /* two- or three-byte encoding */
        two = *(*pUtf8Ptr)++;
        if ((one & 0x20) != 0) {
            /* three-byte encoding */
            three = *(*pUtf8Ptr)++;
            return ((one & 0x0f) << 12) |
                   ((two & 0x3f) << 6) |
                   (three & 0x3f);
        } else {
            /* two-byte encoding */
            return ((one & 0x1f) << 6) |
                   (two & 0x3f);
        }
    } else {
        /* one-byte encoding */
        return one;
    }
}

int dexUtf8Cmp(const char* s1, const char* s2);

/* for dexIsValidMemberNameUtf8(), a bit vector indicating valid low ascii */
extern u4 DEX_MEMBER_VALID_LOW_ASCII[4];

/* Helper for dexIsValidMemberUtf8(); do not call directly. */
bool dexIsValidMemberNameUtf8_0(const char** pUtf8Ptr);

DEX_INLINE bool dexIsValidMemberNameUtf8(const char** pUtf8Ptr) {
    u1 c = (u1) **pUtf8Ptr;
    if (c <= 0x7f) {
        // It's low-ascii, so check the table.
        u4 wordIdx = c >> 5;
        u4 bitIdx = c & 0x1f;
        (*pUtf8Ptr)++;
        return (DEX_MEMBER_VALID_LOW_ASCII[wordIdx] & (1 << bitIdx)) != 0;
    }

    /*
     * It's a multibyte encoded character. Call a non-inline function
     * for the heavy lifting.
     */
    return dexIsValidMemberNameUtf8_0(pUtf8Ptr);
}

/* Return whether the given string is a valid field or method name. */
bool dexIsValidMemberName(const char* s);

/* Return whether the given string is a valid type descriptor. */
bool dexIsValidTypeDescriptor(const char* s);

bool dexIsValidClassName(const char* s, bool dotSeparator);

bool dexIsReferenceDescriptor(const char* s);

bool dexIsClassDescriptor(const char* s);

bool dexIsFieldDescriptor(const char* s);

#endif  // LIBDEX_DEXUTF_H_
