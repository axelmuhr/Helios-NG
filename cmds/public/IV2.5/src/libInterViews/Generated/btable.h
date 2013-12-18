/*
 * Object association table.
 */

#ifndef BitmapTable_h
#define BitmapTable_h

#include "table2.h"

class BitmapTableEntry;

class BitmapTable : public Table2 {
public:
    BitmapTable(int);
    void Insert(int, int, class Bitmap*);
    boolean Find(class Bitmap*&, int, int);
    void Remove(int, int);
};

inline BitmapTable::BitmapTable (int n) : (n) {}

inline void BitmapTable::Insert (int k1, int k2, class Bitmap* v) {
    Table2::Insert((void*)k1, (void*)k2, (void*)v);
}

inline boolean BitmapTable::Find (class Bitmap*& v, int k1, int k2) {
    void* vv;

    boolean b = Table2::Find(vv, (void*)k1, (void*)k2);
    if (b) {
	v = (class Bitmap*)vv;
    }
    return b;
}

inline void BitmapTable::Remove (int k1, int k2) {
    Table2::Remove((void*)k1, (void*)k2);
}

#endif
