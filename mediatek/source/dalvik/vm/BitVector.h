

#ifndef DALVIK_BITVECTOR_H_
#define DALVIK_BITVECTOR_H_

struct BitVector {
    bool    expandable;     /* expand bitmap if we run out? */
    u4      storageSize;    /* current size, in 32-bit words */
    u4*     storage;
};

/* Handy iterator to walk through the bit positions set to 1 */
struct BitVectorIterator {
    BitVector *pBits;
    u4 idx;
    u4 bitSize;
};

/* allocate a bit vector with enough space to hold "startBits" bits */
BitVector* dvmAllocBitVector(unsigned int startBits, bool expandable);
void dvmFreeBitVector(BitVector* pBits);

int dvmAllocBit(BitVector* pBits);
void dvmSetBit(BitVector* pBits, unsigned int num);
void dvmClearBit(BitVector* pBits, unsigned int num);
void dvmClearAllBits(BitVector* pBits);
void dvmSetInitialBits(BitVector* pBits, unsigned int numBits);
bool dvmIsBitSet(const BitVector* pBits, unsigned int num);

/* count the number of bits that have been set */
int dvmCountSetBits(const BitVector* pBits);

/* copy one vector to another of equal size */
void dvmCopyBitVector(BitVector *dest, const BitVector *src);

bool dvmIntersectBitVectors(BitVector *dest, const BitVector *src1,
                            const BitVector *src2);

bool dvmUnifyBitVectors(BitVector *dest, const BitVector *src1,
                        const BitVector *src2);

bool dvmCheckMergeBitVectors(BitVector* dst, const BitVector* src);

bool dvmCompareBitVectors(const BitVector *src1, const BitVector *src2);

/* Initialize the iterator structure */
void dvmBitVectorIteratorInit(BitVector* pBits, BitVectorIterator* iterator);

/* Return the next position set to 1. -1 means end-of-vector reached */
int dvmBitVectorIteratorNext(BitVectorIterator* iterator);

#endif  // DALVIK_BITVECTOR_H_
