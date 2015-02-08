

#ifndef DALVIK_DEXPREPARE_H_
#define DALVIK_DEXPREPARE_H_

enum DexOptimizerMode {
    OPTIMIZE_MODE_UNKNOWN = 0,
    OPTIMIZE_MODE_NONE,         /* never optimize (except "essential") */
    OPTIMIZE_MODE_VERIFIED,     /* only optimize verified classes (default) */
    OPTIMIZE_MODE_ALL,          /* optimize verified & unverified (risky) */
    OPTIMIZE_MODE_FULL          /* fully opt verified classes at load time */
};

/* some additional bit flags for dexopt */
enum DexoptFlags {
    DEXOPT_OPT_ENABLED       = 1,       /* optimizations enabled? */
    DEXOPT_OPT_ALL           = 1 << 1,  /* optimize when verify fails? */
    DEXOPT_VERIFY_ENABLED    = 1 << 2,  /* verification enabled? */
    DEXOPT_VERIFY_ALL        = 1 << 3,  /* verify bootstrap classes? */
    DEXOPT_IS_BOOTSTRAP      = 1 << 4,  /* is dex in bootstrap class path? */
    DEXOPT_GEN_REGISTER_MAPS = 1 << 5,  /* generate register maps during vfy */
    DEXOPT_UNIPROCESSOR      = 1 << 6,  /* specify uniprocessor target */
    DEXOPT_SMP               = 1 << 7   /* specify SMP target */
};

enum VerifyError {
    VERIFY_ERROR_NONE = 0,      /* no error; must be zero */
    VERIFY_ERROR_GENERIC,       /* VerifyError */

    VERIFY_ERROR_NO_CLASS,      /* NoClassDefFoundError */
    VERIFY_ERROR_NO_FIELD,      /* NoSuchFieldError */
    VERIFY_ERROR_NO_METHOD,     /* NoSuchMethodError */
    VERIFY_ERROR_ACCESS_CLASS,  /* IllegalAccessError */
    VERIFY_ERROR_ACCESS_FIELD,  /* IllegalAccessError */
    VERIFY_ERROR_ACCESS_METHOD, /* IllegalAccessError */
    VERIFY_ERROR_CLASS_CHANGE,  /* IncompatibleClassChangeError */
    VERIFY_ERROR_INSTANTIATION, /* InstantiationError */
};

enum VerifyErrorRefType {
    VERIFY_ERROR_REF_CLASS  = 0,
    VERIFY_ERROR_REF_FIELD  = 1,
    VERIFY_ERROR_REF_METHOD = 2,
};

#define kVerifyErrorRefTypeShift 6

#define VERIFY_OK(_failure) ((_failure) == VERIFY_ERROR_NONE)

int dvmOpenCachedDexFile(const char* fileName, const char* cachedFile,
    u4 modWhen, u4 crc, bool isBootstrap, bool* pNewFile, bool createIfMissing);

bool dvmUnlockCachedDexFile(int fd);

bool dvmCheckOptHeaderAndDependencies(int fd, bool sourceAvail, u4 modWhen,
    u4 crc, bool expectVerify, bool expectOpt);

bool dvmOptimizeDexFile(int fd, off_t dexOffset, long dexLen,
    const char* fileName, u4 modWhen, u4 crc, bool isBootstrap);

bool dvmContinueOptimization(int fd, off_t dexOffset, long dexLength,
    const char* fileName, u4 modWhen, u4 crc, bool isBootstrap);

bool dvmPrepareDexInMemory(u1* addr, size_t len, DvmDex** ppDvmDex);

bool dvmCreateInlineSubsTable(void);
void dvmFreeInlineSubsTable(void);

#endif  // DALVIK_DEXPREPARE_H_
