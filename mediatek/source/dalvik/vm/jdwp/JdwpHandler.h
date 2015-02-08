
#ifndef DALVIK_JDWP_JDWPHANDLER_H_
#define DALVIK_JDWP_JDWPHANDLER_H_

#include "Common.h"
#include "ExpandBuf.h"

struct JdwpReqHeader {
    u4  length;
    u4  id;
    u1  cmdSet;
    u1  cmd;
};

void dvmJdwpProcessRequest(JdwpState* state, const JdwpReqHeader* pHeader,
    const u1* buf, int dataLen, ExpandBuf* pReply);

/* helper function */
void dvmJdwpAddLocation(ExpandBuf* pReply, const JdwpLocation* pLoc);

#endif  // DALVIK_JDWP_JDWPHANDLER_H_
