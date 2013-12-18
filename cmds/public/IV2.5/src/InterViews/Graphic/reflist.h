/* RefList class */

#ifndef reflist_h
#define reflist_h

#include <InterViews/defs.h>
#include <InterViews/Graphic/ref.h>

class RefList : public Ref {
public:
    RefList();
    RefList(UID);
    RefList(Persistent*);
    ~RefList();

    void SetRef(Ref r);

    boolean IsEmpty();
    void Append(RefList*);
    void Prepend(RefList*);
    void Remove(RefList*);
    void Delete(Ref);
    RefList* Find(Ref);
    RefList* First();
    RefList* Last();
    RefList* End();
    RefList* Next();
    RefList* Prev();

    boolean Write(PFile*);
    boolean Read(PFile*);
    boolean WriteObjects(PFile*);
    boolean ReadObjects(PFile*);

    RefList* operator[](int count);
protected:
    RefList* next;
    RefList* prev;
};

/*
 * inlines
 */

inline RefList::RefList () : () { next = this; prev = this; }
inline RefList::RefList (UID u) : (u) { next = this; prev = this; }
inline RefList::RefList (Persistent* p) : (p) { next = this; prev = this; }

inline RefList::~RefList () {
    uid(INVALIDUID);
    next = (RefList*) -2;
    prev = (RefList*) -3;
}

inline boolean RefList::IsEmpty () { return next == this; }
inline void RefList::SetRef (Ref r) { uid(r.uid()); }

inline void RefList::Append (RefList* e) {
    prev->next = e;
    e->prev = prev;
    e->next = this;
    prev = e;
}

inline void RefList::Prepend (RefList* e) {
    next->prev = e;
    e->prev = this;
    e->next = next;
    next = e;
}

inline void RefList::Remove (RefList* e) {
    e->prev->next = e->next;
    e->next->prev = e->prev;
    e->prev = (RefList*) -1;
    e->next = (RefList*) -1;
}

inline void RefList::Delete (Ref p) {
    register RefList* e;

    e = Find(p);
    if (e != nil) {
	Remove(e);
	delete e;
    }
}

inline RefList* RefList::First () { return next; }
inline RefList* RefList::Last () { return prev; }
inline RefList* RefList::End () { return this; }
inline RefList* RefList::Next () { return next; }
inline RefList* RefList::Prev () { return prev; }

#endif
