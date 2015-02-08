
#ifndef DALVIK_RAWDEXFILE_H_
#define DALVIK_RAWDEXFILE_H_

struct RawDexFile {
    char*       cacheFileName;
    DvmDex*     pDvmDex;
};

int dvmRawDexFileOpen(const char* fileName, const char* odexOutputName,
    RawDexFile** ppDexFile, bool isBootstrap);

int dvmRawDexFileOpenArray(u1* pBytes, u4 length, RawDexFile** ppDexFile);

void dvmRawDexFileFree(RawDexFile* pRawDexFile);

INLINE DvmDex* dvmGetRawDexFileDex(RawDexFile* pRawDexFile) {
    return pRawDexFile->pDvmDex;
}

/* get full path of optimized DEX file */
INLINE const char* dvmGetRawDexFileCacheFileName(RawDexFile* pRawDexFile) {
    return pRawDexFile->cacheFileName;
}

#endif  // DALVIK_RAWDEXFILE_H_
