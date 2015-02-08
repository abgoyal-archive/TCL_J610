

#ifndef DALVIK_INDIRECTREFTABLE_H_
#define DALVIK_INDIRECTREFTABLE_H_


typedef void* IndirectRef;

/* magic failure value; must not pass dvmIsHeapAddress() */
#define kInvalidIndirectRefObject reinterpret_cast<Object*>(0xdead4321)

#define kClearedJniWeakGlobal reinterpret_cast<Object*>(0xdead1234)

enum IndirectRefKind {
    kIndirectKindInvalid    = 0,
    kIndirectKindLocal      = 1,
    kIndirectKindGlobal     = 2,
    kIndirectKindWeakGlobal = 3
};
const char* indirectRefKindToString(IndirectRefKind kind);

INLINE IndirectRefKind indirectRefKind(IndirectRef iref)
{
    return (IndirectRefKind)((u4) iref & 0x03);
}

struct IndirectRefSlot {
    Object* obj;        /* object pointer itself, NULL if the slot is unused */
    u4      serial;     /* slot serial number */
};

/* use as initial value for "cookie", and when table has only one segment */
#define IRT_FIRST_SEGMENT   0

union IRTSegmentState {
    u4          all;
    struct {
        u4      topIndex:16;            /* index of first unused entry */
        u4      numHoles:16;            /* #of holes in entire table */
    } parts;
};

class iref_iterator {
public:
    explicit iref_iterator(IndirectRefSlot* table, size_t i, size_t capacity) :
            table_(table), i_(i), capacity_(capacity) {
        skipNullsAndTombstones();
    }

    iref_iterator& operator++() {
        ++i_;
        skipNullsAndTombstones();
        return *this;
    }

    Object** operator*() {
        return &table_[i_].obj;
    }

    bool equals(const iref_iterator& rhs) const {
        return (i_ == rhs.i_ && table_ == rhs.table_);
    }

private:
    void skipNullsAndTombstones() {
        // We skip NULLs and tombstones. Clients don't want to see implementation details.
        while (i_ < capacity_ && (table_[i_].obj == NULL
                || table_[i_].obj == kClearedJniWeakGlobal)) {
            ++i_;
        }
    }

    IndirectRefSlot* table_;
    size_t i_;
    size_t capacity_;
};

bool inline operator!=(const iref_iterator& lhs, const iref_iterator& rhs) {
    return !lhs.equals(rhs);
}

struct IndirectRefTable {
public:
    typedef iref_iterator iterator;

    /* semi-public - read/write by interpreter in native call handler */
    IRTSegmentState segmentState;

    /*
     * private:
     *
     * TODO: we can't make these private as long as the interpreter
     * uses offsetof, since private member data makes us non-POD.
     */
    /* bottom of the stack */
    IndirectRefSlot* table_;
    /* bit mask, ORed into all irefs */
    IndirectRefKind kind_;
    /* #of entries we have space for */
    size_t          alloc_entries_;
    /* max #of entries allowed */
    size_t          max_entries_;

    // TODO: want hole-filling stats (#of holes filled, total entries scanned)
    //       for performance evaluation.

    /*
     * Add a new entry.  "obj" must be a valid non-NULL object reference
     * (though it's okay if it's not fully-formed, e.g. the result from
     * dvmMalloc doesn't have obj->clazz set).
     *
     * Returns NULL if the table is full (max entries reached, or alloc
     * failed during expansion).
     */
    IndirectRef add(u4 cookie, Object* obj);

    /*
     * Given an IndirectRef in the table, return the Object it refers to.
     *
     * Returns kInvalidIndirectRefObject if iref is invalid.
     */
    Object* get(IndirectRef iref) const;

    /*
     * Returns true if the table contains a reference to this object.
     */
    bool contains(const Object* obj) const;

    /*
     * Remove an existing entry.
     *
     * If the entry is not between the current top index and the bottom index
     * specified by the cookie, we don't remove anything.  This is the behavior
     * required by JNI's DeleteLocalRef function.
     *
     * Returns "false" if nothing was removed.
     */
    bool remove(u4 cookie, IndirectRef iref);

    /*
     * Initialize an IndirectRefTable.
     *
     * If "initialCount" != "maxCount", the table will expand as required.
     *
     * "kind" should be Local or Global.  The Global table may also hold
     * WeakGlobal refs.
     *
     * Returns "false" if table allocation fails.
     */
    bool init(size_t initialCount, size_t maxCount, IndirectRefKind kind);

    /*
     * Clear out the contents, freeing allocated storage.
     *
     * You must call dvmInitReferenceTable() before you can re-use this table.
     *
     * TODO: this should be a destructor.
     */
    void destroy();

    /*
     * Dump the contents of a reference table to the log file.
     *
     * The caller should lock any external sync before calling.
     *
     * TODO: we should name the table in a constructor and remove
     * the argument here.
     */
    void dump(const char* descr) const;

    /*
     * Return the #of entries in the entire table.  This includes holes, and
     * so may be larger than the actual number of "live" entries.
     */
    size_t capacity() const {
        return segmentState.parts.topIndex;
    }

    iterator begin() {
        return iterator(table_, 0, capacity());
    }

    iterator end() {
        return iterator(table_, capacity(), capacity());
    }

private:
    static inline u4 extractIndex(IndirectRef iref) {
        u4 uref = (u4) iref;
        return (uref >> 2) & 0xffff;
    }

    static inline u4 extractSerial(IndirectRef iref) {
        u4 uref = (u4) iref;
        return uref >> 20;
    }

    static inline u4 nextSerial(u4 serial) {
        return (serial + 1) & 0xfff;
    }

    static inline IndirectRef toIndirectRef(u4 index, u4 serial, IndirectRefKind kind) {
        assert(index < 65536);
        return reinterpret_cast<IndirectRef>((serial << 20) | (index << 2) | kind);
    }
};

#endif  // DALVIK_INDIRECTREFTABLE_H_
