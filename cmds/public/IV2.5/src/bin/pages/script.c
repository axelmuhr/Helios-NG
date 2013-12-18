/*
 * Script
 */

#include "script.h"
#include <ctype.h>
#include <string.h>

static const char BEGINQUOTE = '`';
static const char ENDQUOTE = '\'';

static const int MAXBUFFERSIZE = 1024;

StringScript::StringScript(const char* s, int l) {
    script = s;
    remaining = l;
    text = nil;
    op = NONE;
}

StringScript::StringScript(const char* s) {
    script = s;
    remaining = strlen(script);
    text = nil;
    op = NONE;
}

boolean StringScript::Next() {
    int quotelevel = 0;
    while (isspace(*script)) {
	++script;
	--remaining;
    }
    if (remaining <= 0 || *script == '\0') {
	return false;
    }
    if (isdigit(*script)) {
	op = NUMBER;
    } else if (*script == BEGINQUOTE) {
	++script;
	--remaining;
	++quotelevel;
	op = QUOTE;
    } else {
	op = *script;
    }
    text = script;
    while (
	remaining > 0 && *script != '\0'
	&& (quotelevel > 0 || ! isspace(*script))
   ) {
	if (*script == ENDQUOTE) {
	    --quotelevel;
	} else if (*script == BEGINQUOTE) {
	    ++quotelevel;
	}
	++script;
	--remaining;
    }
    return true;
}

char StringScript::Op() {
    return op;
}

int StringScript::Number() {
    const char* p = text;
    int number = 0;
    while (isdigit(*p)) {
	number *= 10;
	number += *p - '0';
	++p;
    }
    return number;
}

int StringScript::Size() {
    if (op == QUOTE) {
	return script - text - 1;
    } else {
	return script - text;
    }
}

const char* StringScript::Text() {
    return text;
}

FileScript::FileScript(const char* filename, int l) {
    char name[256];
    strncpy(name, filename, l);
    name[l] = '\0';
    file = fopen(name, "r");
    text = nil;
    op = NONE;
}

FileScript::FileScript(const char* filename) {
    file = fopen(filename, "r");
    text = nil;
    op = NONE;
}

FileScript::FileScript(FILE* f) {
    file = f;
    text = nil;
    op = NONE;
}

boolean FileScript::Next() {
    if (file == nil) {
	return false;
    }
    int quotelevel = 0;
    char c;
    while (isspace(c=getc(file))) {
	;
    }
    if (c == EOF) {
	return false;
    } else if (isdigit(c)) {
	op = NUMBER;
    } else if (c == BEGINQUOTE) {
	c = getc(file);
	++quotelevel;
	op = QUOTE;
    } else {
	op = c;
    }
    char buffer[MAXBUFFERSIZE];
    char* buffp = buffer;
    while (
	c != EOF
	&& (quotelevel > 0 || ! isspace(c))
   ) {
	if (c == ENDQUOTE) {
	    --quotelevel;
	} else if (c == BEGINQUOTE) {
	    ++quotelevel;
	}
	*buffp = c;
	++buffp;
	c = getc(file);
    }
   *buffp = '\0';
    text = new char[ strlen(buffer) + 1 ];
    strcpy((char*)text, buffer);
    return true;
}

char FileScript::Op() {
    return op;
}

int FileScript::Number() {
    const char* p = text;
    int number = 0;
    while (isdigit(*p)) {
	number *= 10;
	number += *p - '0';
	++p;
    }
    return number;
}

int FileScript::Size() {
    if (op == QUOTE) {
	return strlen(text)-1;
    } else {
	return strlen(text);
    }
}

const char* FileScript::Text() {
    return text;
}
