/*
 * Property sheet of user-customizable variables.
 */

#ifndef propsheet_h
#define propsheet_h

#include <InterViews/defs.h>

typedef class StringId* PropertyName;
typedef const char* PropertyType;
typedef const char* PropertyValue;

class PropDir;
class PropPath;

/*
 * A property is defined by a name, optional type, and value.
 */

class PropertyDef {
public:
    PropertyName name;
    PropertyType type;
    PropertyValue value;

    PropertyDef () {}
    PropertyDef (PropertyName n) { name = n; value = nil; }
};

class PropertySheet {
public:
    PropertySheet();
    ~PropertySheet();

    /* individual property operations */
    boolean Get(PropertyDef&);
    boolean GetLocal(PropDir*, PropertyDef&);
    PropDir* MakeDir(const char* path);
    void Put(const char* path, const char* value, const char* type = nil);
    void PutLower(const char* path, const char* value, const char* type = nil);
    void PutLocal(
	PropDir*, const char* path, const char* value, const char* type = nil
    );
    void PutLocalLower(
	PropDir*, const char* path, const char* value, const char* type = nil
    );

    /* property tree navigation */
    PropDir* Find(PropertyName name);
    void Pop();
    void Push(PropDir*, boolean sibling);
    PropDir* Root();

    /* external interface */
    void LoadProperty(const char*);
    void LoadList(const char*);
    boolean LoadFile(const char* filename);
private:
    PropDir* cur;
    PropPath* head;
    PropPath* tail;

    void DoPut(
	PropDir* dir, const char* path, const char* value, const char* type,
	boolean override
    );
};

extern PropertySheet* properties;

inline void PropertySheet::Put (
    const char* path, const char* value, const char* type
) {
    DoPut(cur, path, value, type, true);
}

inline void PropertySheet::PutLower (
    const char* path, const char* value, const char* type
) {
    DoPut(cur, path, value, type, false);
}

inline void PropertySheet::PutLocal (
    PropDir* dir, const char* path, const char* value, const char* type
) {
    DoPut(dir, path, value, type, true);
}

inline void PropertySheet::PutLocalLower (
    PropDir* dir, const char* path, const char* value, const char* type
) {
    DoPut(dir, path, value, type, false);
}

#endif
