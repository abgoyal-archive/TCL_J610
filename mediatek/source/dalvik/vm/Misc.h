

#ifndef DALVIK_MISC_H_
#define DALVIK_MISC_H_

#include <string>

#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

#include "Inlines.h"

#define UNUSED_PARAMETER(p)     (void)(p)

INLINE float dvmU4ToFloat(u4 val) {
    union { u4 in; float out; } conv;
    conv.in = val;
    return conv.out;
}
INLINE u4 dvmFloatToU4(float val) {
    union { float in; u4 out; } conv;
    conv.in = val;
    return conv.out;
}

enum HexDumpMode { kHexDumpLocal, kHexDumpMem };
void dvmPrintHexDumpEx(int priority, const char* tag, const void* vaddr,
    size_t length, HexDumpMode mode);

INLINE void dvmPrintHexDump(const void* vaddr, size_t length) {
    dvmPrintHexDumpEx(ANDROID_LOG_INFO, LOG_TAG,
        vaddr, length, kHexDumpLocal);
}

INLINE void dvmPrintHexDumpDbg(const void* vaddr, size_t length,const char* tag)
{
#if !LOG_NDEBUG
    dvmPrintHexDumpEx(ANDROID_LOG_VERBOSE, (tag != NULL) ? tag : LOG_TAG,
        vaddr, length, kHexDumpLocal);
#endif
}

enum DebugTargetKind {
    kDebugTargetUnknown = 0,
    kDebugTargetLog,
    kDebugTargetFile,
};

struct DebugOutputTarget {
    /* where to? */
    DebugTargetKind which;

    /* additional bits */
    union {
        struct {
            int priority;
            const char* tag;
        } log;
        struct {
            FILE* fp;
        } file;
    } data;
};

void dvmCreateLogOutputTarget(DebugOutputTarget* target, int priority,
    const char* tag);
void dvmCreateFileOutputTarget(DebugOutputTarget* target, FILE* fp);

void dvmPrintDebugMessage(const DebugOutputTarget* target, const char* format,
    ...)
#if defined(__GNUC__)
    __attribute__ ((format(printf, 2, 3)))
#endif
    ;

char* dvmDotToSlash(const char* str);

std::string dvmHumanReadableDescriptor(const char* descriptor);

std::string dvmHumanReadableType(const Object* obj);

struct Field;
std::string dvmHumanReadableField(const Field* field);

struct Method;
std::string dvmHumanReadableMethod(const Method* method, bool withSignature);

char* dvmDescriptorToDot(const char* str);

char* dvmDotToDescriptor(const char* str);

char* dvmDescriptorToName(const char* str);

char* dvmNameToDescriptor(const char* str);

u8 dvmGetRelativeTimeNsec(void);

INLINE u8 dvmGetRelativeTimeUsec(void) {
    return dvmGetRelativeTimeNsec() / 1000;
}

INLINE u4 dvmGetRelativeTimeMsec(void) {
    return (u4)(dvmGetRelativeTimeUsec() / 1000);
}

u8 dvmGetThreadCpuTimeNsec(void);

INLINE u8 dvmGetThreadCpuTimeUsec(void) {
    return dvmGetThreadCpuTimeNsec() / 1000;
}

u8 dvmGetOtherThreadCpuTimeNsec(pthread_t thread);
INLINE u8 dvmGetOtherThreadCpuTimeUsec(pthread_t thread) {
    return dvmGetOtherThreadCpuTimeNsec(thread) / 1000;
}

bool dvmIterativeSleep(int iteration, int maxTotalSleep, u8 relStartTime);

bool dvmSetCloseOnExec(int fd);

extern "C" void dvmAbort(void);
void dvmPrintNativeBackTrace(void);

#if (!HAVE_STRLCPY)
/* Implementation of strlcpy() for platforms that don't already have it. */
extern "C" size_t strlcpy(char *dst, const char *src, size_t size);
#endif

void *dvmAllocRegion(size_t size, int prot, const char *name);

struct ProcStatData {
    unsigned long utime;    /* number of jiffies scheduled in user mode */
    unsigned long stime;    /* number of jiffies scheduled in kernel mode */
    int processor;          /* number of CPU that last executed thread */
};
bool dvmGetThreadStats(ProcStatData* pData, pid_t tid);

const char* dvmPathToAbsolutePortion(const char* path);

std::string StringPrintf(const char* fmt, ...)
        __attribute__((__format__ (__printf__, 1, 2)));

void StringAppendF(std::string* dst, const char* fmt, ...)
        __attribute__((__format__ (__printf__, 2, 3)));

void StringAppendV(std::string* dst, const char* format, va_list ap);

#endif  // DALVIK_MISC_H_
