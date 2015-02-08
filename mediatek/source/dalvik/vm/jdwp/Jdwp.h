
#ifndef DALVIK_JDWP_JDWP_H_
#define DALVIK_JDWP_JDWP_H_

#include "jdwp/JdwpConstants.h"
#include "jdwp/ExpandBuf.h"
#include "Common.h"
#include "Bits.h"
#include <pthread.h>

struct JdwpState;       /* opaque */

typedef u4 FieldId;     /* static or instance field */
typedef u4 MethodId;    /* any kind of method, including constructors */
typedef u8 ObjectId;    /* any object (threadID, stringID, arrayID, etc) */
typedef u8 RefTypeId;   /* like ObjectID, but unique for Class objects */
typedef u8 FrameId;     /* short-lived stack frame ID */

INLINE FieldId dvmReadFieldId(const u1** pBuf)      { return read4BE(pBuf); }
INLINE MethodId dvmReadMethodId(const u1** pBuf)    { return read4BE(pBuf); }
INLINE ObjectId dvmReadObjectId(const u1** pBuf)    { return read8BE(pBuf); }
INLINE RefTypeId dvmReadRefTypeId(const u1** pBuf)  { return read8BE(pBuf); }
INLINE FrameId dvmReadFrameId(const u1** pBuf)      { return read8BE(pBuf); }
INLINE void dvmSetFieldId(u1* buf, FieldId val)     { return set4BE(buf, val); }
INLINE void dvmSetMethodId(u1* buf, MethodId val)   { return set4BE(buf, val); }
INLINE void dvmSetObjectId(u1* buf, ObjectId val)   { return set8BE(buf, val); }
INLINE void dvmSetRefTypeId(u1* buf, RefTypeId val) { return set8BE(buf, val); }
INLINE void dvmSetFrameId(u1* buf, FrameId val)     { return set8BE(buf, val); }
INLINE void expandBufAddFieldId(ExpandBuf* pReply, FieldId id) {
    expandBufAdd4BE(pReply, id);
}
INLINE void expandBufAddMethodId(ExpandBuf* pReply, MethodId id) {
    expandBufAdd4BE(pReply, id);
}
INLINE void expandBufAddObjectId(ExpandBuf* pReply, ObjectId id) {
    expandBufAdd8BE(pReply, id);
}
INLINE void expandBufAddRefTypeId(ExpandBuf* pReply, RefTypeId id) {
    expandBufAdd8BE(pReply, id);
}
INLINE void expandBufAddFrameId(ExpandBuf* pReply, FrameId id) {
    expandBufAdd8BE(pReply, id);
}


struct JdwpLocation {
    u1          typeTag;        /* class or interface? */
    RefTypeId   classId;        /* method->clazz */
    MethodId    methodId;       /* method in which "idx" resides */
    u8          idx;            /* relative index into code block */
};

enum JdwpTransportType {
    kJdwpTransportUnknown = 0,
    kJdwpTransportSocket,       /* transport=dt_socket */
    kJdwpTransportAndroidAdb,   /* transport=dt_android_adb */
};

struct JdwpStartupParams {
    JdwpTransportType transport;
    bool        server;
    bool        suspend;
    char        host[64];
    short       port;
    /* more will be here someday */
};

JdwpState* dvmJdwpStartup(const JdwpStartupParams* params);

void dvmJdwpShutdown(JdwpState* state);

bool dvmJdwpIsActive(JdwpState* state);

pthread_t dvmJdwpGetDebugThread(JdwpState* state);

s8 dvmJdwpLastDebuggerActivity(JdwpState* state);

//ObjectId dvmJdwpGetWaitForEventThread(JdwpState* state);
void dvmJdwpSetWaitForEventThread(JdwpState* state, ObjectId threadId);
void dvmJdwpClearWaitForEventThread(JdwpState* state);

bool dvmJdwpCheckConnection(JdwpState* state);
bool dvmJdwpAcceptConnection(JdwpState* state);
bool dvmJdwpEstablishConnection(JdwpState* state);
void dvmJdwpCloseConnection(JdwpState* state);
bool dvmJdwpProcessIncoming(JdwpState* state);



bool dvmJdwpPostVMStart(JdwpState* state, bool suspend);

bool dvmJdwpPostLocationEvent(JdwpState* state, const JdwpLocation* pLoc,
    ObjectId thisPtr, int eventFlags);

bool dvmJdwpPostException(JdwpState* state, const JdwpLocation* pThrowLoc,
    ObjectId excepId, RefTypeId excepClassId, const JdwpLocation* pCatchLoc,
    ObjectId thisPtr);

bool dvmJdwpPostThreadChange(JdwpState* state, ObjectId threadId, bool start);

bool dvmJdwpPostClassPrepare(JdwpState* state, int tag, RefTypeId refTypeId,
    const char* signature, int status);

bool dvmJdwpPostVMDeath(JdwpState* state);

void dvmJdwpDdmSendChunkV(JdwpState* state, int type, const struct iovec* iov,
    int iovcnt);

#endif  // DALVIK_JDWP_JDWP_H_
