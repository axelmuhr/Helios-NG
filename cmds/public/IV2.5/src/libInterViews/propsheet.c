/*
 * Property sheet management.
 */

#include <InterViews/propsheet.h>
#include <InterViews/strpool.h>
#include <InterViews/strtable.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

extern StringTable* nameTable;

PropertySheet* properties;

static StringPool* propvalues;

class PropList {
    friend class PropDir;
    friend class PropertySheet;
protected:
    PropertyName name;
    PropList* next;
    PropList* prev;

    PropList();
    PropList(PropertyName);
    virtual ~PropList();

    void Append(PropList*);
    void DeleteAll();
    PropList* Find(PropertyName);
    void Remove();
};

class AttrList : public PropList {
    friend class PropDir;
    friend class PropertySheet;

    PropertyValue value;
    PropertyType type;

    AttrList();
    AttrList(PropertyName);

    boolean FindAttr(PropertyDef&);
};

class DirList : public PropList {
    friend class PropDir;
    friend class PropertySheet;

    PropDir* info;

    DirList();
    DirList(PropertyName);
    ~DirList();

    boolean FindSubDir(PropertyName, PropDir*&);
};

class PropDir {
public:
    PropDir();
    ~PropDir();
private:
    friend class PropertySheet;

    PropDir* parent;
    AttrList* attrs;
    DirList* dirs;
    AttrList* vattrs;
    DirList* vdirs;

    PropDir* MakeDirs(const char*&);
};

static const int pathClusterSize = 20;

class PropPathElement {
    friend class PropertySheet;

    PropDir* dir;
    boolean sibling;
};

class PropPath {
    friend class PropertySheet;

    PropPathElement element[pathClusterSize];
    PropPathElement* top;
    PropPath* next;
    PropPath* prev;

    PropPath();
};

static char* Strdup (const char* str, int n) {
    char* s = new char[n+1];
    strncpy(s, str, n);
    s[n] = '\0';
    return s;
}

/*
 * class PropList
 */

PropList::PropList () {
    name = nil;
    next = this;
    prev = this;
}

PropList::PropList (PropertyName n) {
    name = n;
}

PropList::~PropList () {
    /* nothing to do */
}

void PropList::Append (PropList* p) {
    p->next = prev->next;	/* link element to old tail->next */
    p->prev = prev;		/* link element to old tail */
    prev->next = p;		/* link old tail to element */
    prev = p;			/* set tail to element */
}

void PropList::Remove () {
    next->prev = prev;
    prev->next = next;
}

PropList* PropList::Find (PropertyName n) {
    register PropList* p;

    for (p = next; p != this; p = p->next) {
	if (p->name == n) {
	    return p;
	}
    }
    return nil;
}

void PropList::DeleteAll () {
    register PropList* p, * pnext;

    for (p = next; p != this; p = pnext) {
	pnext = p->next;
	delete p;
    }
    next = this;
    prev = this;
}

/*
 * class AttrList
 */

AttrList::AttrList () {
    value = nil;
    type = nil;
}

AttrList::AttrList (PropertyName n) : (n) {
    value = nil;
    type = nil;
}

boolean AttrList::FindAttr (register PropertyDef& p) {
    register AttrList* a;

    for (a = (AttrList*)next; a != this; a = (AttrList*)a->next) {
	if (a->name == p.name) {
	    p.type = a->type;
	    p.value = a->value;
	    return true;
	}
    }
    return false;
}

/*
 * class DirList
 */

DirList::DirList () {
    info = nil;
}

DirList::DirList (PropertyName n) : (n) {
    info = new PropDir;
}

boolean DirList::FindSubDir (PropertyName n, PropDir*& subdir) {
    register DirList* d;

    for (d = (DirList*)next; d != this; d = (DirList*)d->next) {
	if (d->name == n) {
	    subdir = d->info;
	    return true;
	}
    }
    return false;
}

DirList::~DirList () {
    delete info;
}

/*
 * class PropDir
 */

PropDir::PropDir () {
    parent = nil;
    attrs = new AttrList;
    dirs = new DirList;
    vattrs = new AttrList;
    vdirs = new DirList;
}

PropDir::~PropDir () {
    dirs->DeleteAll();
    delete dirs;
    vdirs->DeleteAll();
    delete vdirs;
    attrs->DeleteAll();
    delete attrs;
    vattrs->DeleteAll();
    delete vattrs;
}

PropDir* PropDir::MakeDirs (const char*& name) {
    register const char* p;
    PropDir* dir;
    DirList* cur, * sub;
    PropertyName n;
    int c;

    dir = this;
    c = '.';
    for (p = name; *p != '\0'; p++) {
	if (*p == '.' || *p == '*') {
	    if (p > name) {
		n = nameTable->Id(name, p - name);
		cur = (c == '*') ? dir->vdirs : dir->dirs;
		if (!cur->FindSubDir(n, dir)) {
		    sub = new DirList(n);
		    sub->info->parent = dir;
		    cur->Append(sub);
		    dir = sub->info;
		}
	    }
	    c = *p;
	    name = p + 1;
	}
    }
    return dir;
}

/*
 * class PropPath
 */

PropPath::PropPath () {
    top = &element[0];
    next = nil;
    prev = nil;
}

/*
 * class PropertySheet
 */

PropertySheet::PropertySheet () {
    cur = new PropDir;
    head = new PropPath;
    tail = head;
    head->element[0].dir = cur;
    head->element[0].sibling = false;
    if (propvalues == nil) {
	propvalues = new StringPool;
    }
}

PropertySheet::~PropertySheet () {
    register PropPath* p, * pnext;

    delete cur;
    for (p = head; p != nil; p = pnext) {
	pnext = p->next;
	delete p;
    }
}

boolean PropertySheet::Get (PropertyDef& prop) {
    register PropPath* p;
    register PropPathElement* i;

    /* look for instance.attr */
    p = tail;
    i = tail->top;
    if (i->dir->attrs->FindAttr(prop)) {
	return true;
    }
    /* look for class.attr (if prev path element is class for instance) */
    if (i->sibling) {
	/* prev path element is class -- look for class.attr */
	--i;
	if (i < &p->element[0]) {
	    i = p->prev->top;
	}
	if (i->dir->attrs->FindAttr(prop)) {
	    return true;
	}
    }
    /* look for path*attr */
    for (p = tail; p != nil; p = p->prev) {
	for (i = p->top; i >= &p->element[0]; i--) {
	    if (i->dir->vattrs->FindAttr(prop)) {
		return true;
	    }
	}
    }
    prop.value = nil;
    return false;
}

/*
 * Special lookup under current directory for .attr  or *attr.
 */

boolean PropertySheet::GetLocal (PropDir* dir, PropertyDef& prop) {
    if (dir->attrs->FindAttr(prop) || dir->vattrs->FindAttr(prop)) {
	return true;
    }
    prop.value = nil;
    return false;
}

/*
 * Add an empty directory to the property sheet.
 */

PropDir* PropertySheet::MakeDir (const char* path) {
    const char* name = path;
    PropDir* dir = cur;
    DirList* d = (name > path && *(name-1) == '*') ? dir->vdirs : dir->dirs;
    PropertyName n = nameTable->Id(name);
    if (!d->FindSubDir(n, dir)) {
	DirList* sub = new DirList(n);
	sub->info->parent = dir;
	d->Append(sub);
	dir = sub->info;
    }
    return dir;
}

/*
 * Put an attribute in the property sheet, starting the path search
 * at a given directory (typically the root).  If the name is already
 * defined and the "override" parameter is false, then do nothing.
 */

void PropertySheet::DoPut (
    PropDir* root, const char* path, const char* value, const char* type,
    boolean override
) {
    const char* name = path;
    boolean newvalue = override;
    PropDir* dir = root->MakeDirs(name);
    AttrList* alist =
	(name > path && *(name-1) == '*') ? dir->vattrs : dir->attrs;
    PropertyName n = nameTable->Id(name);
    register AttrList* a = (AttrList*)alist->Find(n);
    if (a == nil) {
	a = new AttrList(n);
	alist->Append(a);
	newvalue = true;
    }
    if (newvalue) {
	register const char* v;

	for (v = value; isspace(*v); v++);
	a->value = propvalues->Append(v, strlen(v) + 1);
	a->type = type;
    }
}

/*
 * Look up the path for a subdirectory matching the given name.
 */

PropDir* PropertySheet::Find (PropertyName name) {
    register PropPath* p;
    register PropPathElement* e;
    PropDir* dir;

    dir = nil;
    /* look for path.name */
    if (tail->top->dir->dirs->FindSubDir(name, dir)) {
	return dir;
    }
    /* look for path*name */
    for (p = tail; p != nil; p = p->prev) {
	for (e = p->top; e >= &p->element[0]; e--) {
	    if (e->dir->vdirs->FindSubDir(name, dir)) {
		return dir;
	    }
	}
    }
    return nil;
}

void PropertySheet::Push (PropDir* dir, boolean b) {
    register PropPath* p;
    register PropPathElement* e;

    p = tail;
    e = p->top + 1;
    if (e >= &p->element[pathClusterSize]) {
	p = new PropPath;
	p->prev = tail;
	tail->next = p;
	tail = p;
	e = p->top;
    } else {
	p->top = e;
    }
    e->dir = dir;
    e->sibling = b;
}

void PropertySheet::Pop () {
    register PropPath* p = tail;
    if (p == nil) {
	/* ignore underflow */
    } else if (p->top > &p->element[0]) {
	p->top -= 1;
    } else {
	tail = p->prev;
	tail->next = nil;
	delete p;
    }
}

PropDir* PropertySheet::Root () {
    return cur;
}

static int line;	/* for error handling */

/*
 * Load a single property from a string containing
 * the name and value, terminated by either a newline or null.
 */

void PropertySheet::LoadProperty (const char* s) {
    register const char* src;
    register char* dst;
    char buf[4096];
    const char* value;

    for (src = s; *src == ' ' || *src == '\t'; src++);
    if (*src == '#' || *src == '\n' || *src == '\0') {
	return;
    }
    dst = buf;
    value = nil;
    for (; *src != '\n' && *src != '\0'; src++) {
	if (*src == '\\') {
	    ++src;
	    if (*src == 'n') {
		*dst++ = '\n';
	    } else if (*src == '\n') {
		++line;
	    } else {
		*dst++ = *src;
	    }
	} else if (value == nil && *src == ':') {
	    if (dst == buf) {
		fprintf(stderr, "%d: empty path\n", line);
		return;
	    }
	    *dst++ = '\0';
	    value = dst;
	} else {
	    *dst++ = *src;
	}
    }
    *dst = '\0';
    if (value == nil) {
	fprintf(stderr, "%d: missing value for %s\n", line, buf);
	return;
    }
    PutLower(buf, value);
}

/*
 * Parse property sheet information from a string.
 */

void PropertySheet::LoadList (const char* data) {
    register const char* p, * start;

    start = data;
    for (p = strchr(data, '\n'); p != nil; p = strchr(p+1, '\n')) {
	if (p > start && *(p-1) != '\\') {
	    LoadProperty(start);
	}
	start = p+1;
    }
}

/*
 * Read a property sheet from a file (e.g., Xdefaults).
 */

boolean PropertySheet::LoadFile (const char* filename) {
    FILE* f;
    char buf[4096];
    register int i;

    if (filename == nil) {
	f = stdin;
    } else {
	f = fopen(filename, "r");
	if (f == nil) {
	    return false;
	}
    }
    line = 0;
    i = 0;
    while (fgets(&buf[i], sizeof(buf) - i, f) != nil) {
	i = strlen(buf);
	if (buf[i-1] == '\n' && buf[i-2] != '\\') {
	    LoadProperty(buf);
	    i = 0;
	} else {
	    if (i == sizeof(buf)) {
		fprintf(stderr, "%s: %d: line too long\n", filename, line);
		return true;
	    }
	}
    }
    return true;
}
