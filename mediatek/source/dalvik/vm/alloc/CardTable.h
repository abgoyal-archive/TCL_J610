


#ifndef DALVIK_ALLOC_CARDTABLE_H_
#define DALVIK_ALLOC_CARDTABLE_H_

#define GC_CARD_SHIFT 7
#define GC_CARD_SIZE (1 << GC_CARD_SHIFT)
#define GC_CARD_CLEAN 0
#define GC_CARD_DIRTY 0x70

bool dvmCardTableStartup(size_t heapMaximumSize, size_t growthLimit);

void dvmCardTableShutdown(void);

void dvmClearCardTable(void);

u1 *dvmCardFromAddr(const void *addr);

void *dvmAddrFromCard(const u1 *card);

bool dvmIsValidCard(const u1 *card);

void dvmMarkCard(const void *addr);

void dvmVerifyCardTable(void);

#endif  // DALVIK_ALLOC_CARDTABLE_H_
