
#ifndef ART_H
#define ART_H
#include <Dalvik.h>

enum {
  ART_INTERNAL,
  ART_EXTERNAL
};

typedef struct ArtMemProof {
    size_t internal;
    size_t external;
} ArtMemProof;

typedef struct ArtDumpHeap {
    int dump;
    u4 *hash;
    const char **name;
    int *count;
    size_t *size;
} ArtDumpHeap;

struct ART {
    ArtDumpHeap dh;
    ArtMemProof mp;
    /*int dumpheap;
    size_t alloc;
    size_t external;
    u4 hash[4096];
    size_t objsize[4096];
    const char *name[4096];
    int objcount[4096];*/
};

#define ART_HEAP_DUMP_BUF_SIZE 4096

extern struct ART gArt;

void artStartup();
void artAllocLog(ClassObject* clazz, bool isArray, unsigned int size);
void artTrendLog(bool gc, const char* reason);
void artHeapLog(int type);
void artHeapDumpInit();
void artHeapDumpTrace(const Object *obj);
void artHeapDump();

size_t artGetByteAllocated(); //kirby
size_t artGetExternalByteAllocated(); //kirby
void artSetExternalLimit(int value); //kirby

#endif //ART_H
