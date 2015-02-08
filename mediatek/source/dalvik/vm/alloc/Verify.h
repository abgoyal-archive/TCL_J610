

#ifndef DALVIK_ALLOC_VERIFY_H_
#define DALVIK_ALLOC_VERIFY_H_

void dvmVerifyObject(const Object *obj);

void dvmVerifyBitmap(const HeapBitmap *bitmap);

void dvmVerifyRoots(void);

#endif  // DALVIK_ALLOC_VERIFY_H_
