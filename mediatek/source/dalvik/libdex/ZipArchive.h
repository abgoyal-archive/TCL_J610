

#ifndef LIBDEX_ZIPARCHIVE_H_
#define LIBDEX_ZIPARCHIVE_H_

#include "SysUtil.h"
#include "DexFile.h"            // need DEX_INLINE

typedef void* ZipEntry;

struct ZipHashEntry {
    const char*     name;
    unsigned short  nameLen;
};

struct ZipArchive {
    /* open Zip archive */
    int         mFd;

    /* mapped central directory area */
    off_t       mDirectoryOffset;
    MemMapping  mDirectoryMap;

    /* number of entries in the Zip archive */
    int         mNumEntries;

    /*
     * We know how many entries are in the Zip archive, so we can have a
     * fixed-size hash table.  We probe on collisions.
     */
    int         mHashTableSize;
    ZipHashEntry* mHashTable;
};

/* Zip compression methods we support */
enum {
    kCompressStored     = 0,        // no compression
    kCompressDeflated   = 8,        // standard deflate
};


int dexZipOpenArchive(const char* fileName, ZipArchive* pArchive);

int dexZipPrepArchive(int fd, const char* debugFileName, ZipArchive* pArchive);

void dexZipCloseArchive(ZipArchive* pArchive);

DEX_INLINE int dexZipGetArchiveFd(const ZipArchive* pArchive) {
    return pArchive->mFd;
}

ZipEntry dexZipFindEntry(const ZipArchive* pArchive,
    const char* entryName);

int dexZipGetEntryInfo(const ZipArchive* pArchive, ZipEntry entry,
    int* pMethod, size_t* pUncompLen, size_t* pCompLen, off_t* pOffset,
    long* pModWhen, long* pCrc32);

DEX_INLINE long dexGetZipEntryOffset(const ZipArchive* pArchive,
    const ZipEntry entry)
{
    off_t val = 0;
    dexZipGetEntryInfo(pArchive, entry, NULL, NULL, NULL, &val, NULL, NULL);
    return (long) val;
}
DEX_INLINE size_t dexGetZipEntryUncompLen(const ZipArchive* pArchive,
    const ZipEntry entry)
{
    size_t val = 0;
    dexZipGetEntryInfo(pArchive, entry, NULL, &val, NULL, NULL, NULL, NULL);
    return val;
}
DEX_INLINE long dexGetZipEntryModTime(const ZipArchive* pArchive,
    const ZipEntry entry)
{
    long val = 0;
    dexZipGetEntryInfo(pArchive, entry, NULL, NULL, NULL, NULL, &val, NULL);
    return val;
}
DEX_INLINE long dexGetZipEntryCrc32(const ZipArchive* pArchive,
    const ZipEntry entry)
{
    long val = 0;
    dexZipGetEntryInfo(pArchive, entry, NULL, NULL, NULL, NULL, NULL, &val);
    return val;
}

int dexZipExtractEntryToFile(const ZipArchive* pArchive,
    const ZipEntry entry, int fd);

u4 dexInitCrc32(void);
u4 dexComputeCrc32(u4 crc, const void* buf, size_t len);

#endif  // LIBDEX_ZIPARCHIVE_H_
