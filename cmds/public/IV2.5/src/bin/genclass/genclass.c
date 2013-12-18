/*
 * Copyright (c) 1987, 1988 by The Board of Trustees
 * of the Leland Stanford Junior University
 *
 *               All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of Stanford not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * Stanford makes no representations about the suitability of
 * this software for any purpose.  The Software is provided "as is"
 * without express or implied warranty.  In no event shall Stanford
 * be liable for any special, indirect or consequential damages or
 * any damages whatsoever resulting from loss of use, data or profits,
 * whether in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */

/*
 * genclass -- instantiate a parameterized class.
 */

#include <InterViews/defs.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

FILE* input;
FILE* output;

class Param {
public:
    Param(const char*, int, const char*, Param*);
    const char* name();
    int namelen();
    const char* value();
    Param* next();
private:
    const char* _name;
    int _namelen;
    const char* _value;
    Param* _next;
};

inline Param::Param (const char* n, int len, const char* v, Param* p) {
    _name = n;
    _namelen = len;
    _value = v;
    _next = p;
}

inline const char* Param::name () { return _name; }
inline int Param::namelen () { return _namelen; }
inline const char* Param::value () { return _value; }
inline Param* Param::next () { return _next; }

int badarg(const char*, const char*);
int edit(Param*);
void replace(const char*, Param*);

int main (int argc, char* argv[]) {
    register int i;
    Param* params = nil;

    input = stdin;
    output = stdout;
    for (i = 1; i < argc; i++) {
	register char* arg = argv[i];
	char* asgn = strchr(arg, '=');
	if (asgn == nil) {
	    if (input == stdin) {
		input = fopen(arg, "r");
		if (input == nil) {
		    return badarg("can't read %s", arg);
		}
	    } else if (output == stdout) {
		output = fopen(arg, "w");
		if (output == nil) {
		    return badarg("can't write %s", arg);
		}
	    } else {
		return badarg("missing assignment in %s", arg);
	    }
	} else {
	    *asgn = '\0';
	    params = new Param(arg, asgn - arg, asgn + 1, params);
	}
    }
    return edit(params);
}

int badarg (const char* msg, const char* arg) {
    fprintf(stderr, msg, arg);
    fprintf(stderr,
	"\nusage: genclass [infile] [outfile] <name>=<value> ...\n"
    );
    return 1;
}

int edit (register Param* params) {
    const int maxlength = 1024;
    char ident[maxlength];
    register int c;
    register char* p;

    while ((c = getc(input)) != EOF) {
	if (c == '\\') {
	    for (p = ident; isalnum(c = getc(input)); p++) {
		*p = c;
	    }
	    if (c == EOF) {
		return 1;
	    }
	    *p = '\0';
	    replace(ident, params);
	}
	putc(c, output);
    }
    return 0;
}

void replace (const char* name, Param* params) {
    register Param* m = nil;
    register int mlen = 0;
    for (register Param* p = params; p != nil; p = p->next()) {
	register int len = p->namelen();
	if (strncmp(name, p->name(), len) == 0 && len > mlen) {
	    m = p;
	    mlen = m->namelen();
	}
    }
    if (m == nil) {
	putc('\\', output);
	fputs(name, output);
    } else {
	fputs(m->value(), output);
	if (name[mlen] != '\0') {
	    fputs(&name[mlen], output);
	}
    }
}
