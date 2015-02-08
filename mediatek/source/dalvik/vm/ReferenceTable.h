

#ifndef DALVIK_REFERENCETABLE_H_
#define DALVIK_REFERENCETABLE_H_

struct ReferenceTable {
    Object**        nextEntry;          /* top of the list */
    Object**        table;              /* bottom of the list */

    int             allocEntries;       /* #of entries we have space for */
    int             maxEntries;         /* max #of entries allowed */
};

bool dvmInitReferenceTable(ReferenceTable* pRef, int initialCount,
    int maxCount);

void dvmClearReferenceTable(ReferenceTable* pRef);

INLINE size_t dvmReferenceTableEntries(const ReferenceTable* pRef)
{
    return pRef->nextEntry - pRef->table;
}

INLINE size_t dvmIsReferenceTableFull(const ReferenceTable* pRef)
{
    return dvmReferenceTableEntries(pRef) == (size_t)pRef->allocEntries;
}

bool dvmAddToReferenceTable(ReferenceTable* pRef, Object* obj);

Object** dvmFindInReferenceTable(const ReferenceTable* pRef, Object** bottom,
    Object* obj);

bool dvmRemoveFromReferenceTable(ReferenceTable* pRef, Object** bottom,
    Object* obj);

void dvmDumpReferenceTable(const ReferenceTable* pRef, const char* descr);

void dvmDumpReferenceTableContents(Object* const* refs, size_t count,
    const char* descr);

#endif  // DALVIK_REFERENCETABLE_H_
