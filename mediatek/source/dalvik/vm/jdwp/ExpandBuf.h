
#ifndef DALVIK_JDWP_EXPANDBUF_H_
#define DALVIK_JDWP_EXPANDBUF_H_

#include "Common.h"     // need u1/u2/u4/u8 types

struct ExpandBuf;   /* private */

/* create a new struct */
ExpandBuf* expandBufAlloc(void);
/* free storage */
void expandBufFree(ExpandBuf* pBuf);

u1* expandBufGetBuffer(ExpandBuf* pBuf);
size_t expandBufGetLength(ExpandBuf* pBuf);

u1* expandBufAddSpace(ExpandBuf* pBuf, int gapSize);
void expandBufAdd1(ExpandBuf* pBuf, u1 val);
void expandBufAdd2BE(ExpandBuf* pBuf, u2 val);
void expandBufAdd4BE(ExpandBuf* pBuf, u4 val);
void expandBufAdd8BE(ExpandBuf* pBuf, u8 val);
void expandBufAddUtf8String(ExpandBuf* pBuf, const u1* str);

#endif  // DALVIK_JDWP_EXPANDBUF_H_
