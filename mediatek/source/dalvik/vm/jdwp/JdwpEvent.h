
#ifndef DALVIK_JDWP_JDWPEVENT_H_
#define DALVIK_JDWP_JDWPEVENT_H_

#include "JdwpConstants.h"
#include "ExpandBuf.h"

union JdwpEventMod {
    u1      modKind;                /* JdwpModKind */
    struct {
        u1          modKind;
        int         count;
    } count;
    struct {
        u1          modKind;
        u4          exprId;
    } conditional;
    struct {
        u1          modKind;
        ObjectId    threadId;
    } threadOnly;
    struct {
        u1          modKind;
        RefTypeId   refTypeId;
    } classOnly;
    struct {
        u1          modKind;
        char*       classPattern;
    } classMatch;
    struct {
        u1          modKind;
        char*       classPattern;
    } classExclude;
    struct {
        u1          modKind;
        JdwpLocation loc;
    } locationOnly;
    struct {
        u1          modKind;
        u1          caught;
        u1          uncaught;
        RefTypeId   refTypeId;
    } exceptionOnly;
    struct {
        u1          modKind;
        RefTypeId   refTypeId;
        FieldId     fieldId;
    } fieldOnly;
    struct {
        u1          modKind;
        ObjectId    threadId;
        int         size;           /* JdwpStepSize */
        int         depth;          /* JdwpStepDepth */
    } step;
    struct {
        u1          modKind;
        ObjectId    objectId;
    } instanceOnly;
};

struct JdwpEvent {
    JdwpEvent* prev;           /* linked list */
    JdwpEvent* next;

    JdwpEventKind eventKind;      /* what kind of event is this? */
    JdwpSuspendPolicy suspendPolicy;  /* suspend all, none, or self? */
    int modCount;       /* #of entries in mods[] */
    u4 requestId;      /* serial#, reported to debugger */

    JdwpEventMod mods[1];        /* MUST be last field in struct */
};

JdwpEvent* dvmJdwpEventAlloc(int numMods);
void dvmJdwpEventFree(JdwpEvent* pEvent);

JdwpError dvmJdwpRegisterEvent(JdwpState* state, JdwpEvent* pEvent);

void dvmJdwpUnregisterEventById(JdwpState* state, u4 requestId);

void dvmJdwpUnregisterAll(JdwpState* state);

bool dvmJdwpSendRequest(JdwpState* state, ExpandBuf* pReq);

#endif  // DALVIK_JDWP_JDWPEVENT_H_
