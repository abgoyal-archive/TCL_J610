
#ifndef LIBDEX_CMDUTILS_H_
#define LIBDEX_CMDUTILS_H_

/* encode the result of unzipping to a file */
enum UnzipToFileResult {
    kUTFRSuccess = 0,
    kUTFRGenericFailure,
    kUTFRBadArgs,
    kUTFRNotZip,
    kUTFRNoClassesDex,
    kUTFROutputFileProblem,
    kUTFRBadZip,
};

UnzipToFileResult dexOpenAndMap(const char* fileName, const char* tempFileName,
    MemMapping* pMap, bool quiet);

UnzipToFileResult dexUnzipToFile(const char* zipFileName,
    const char* outFileName, bool quiet);

#endif  // LIBDEX_CMDUTILS_H_
