/* 
 * Base class for all objects/symbols of the linker 
 */

#include "base.h"
#include "hash.h"
#include "program.h"
#include "system.h"

Base::Base (What id) {
    this->id = id;
    BaseInit();
}

Base::~Base () {
    // do nothing
}

/* 
 * similar to Link.  Called by the object wanting to be unlinked.
 * typical use : minor->Unlink(major.listHead); 
 */
void Base::Unlink (Base*& head) {
    if (nn == this) {
	head = nil;
    } else {
	if (head == this) {
	    head = nn;
	}
	pp->nn = nn;
	nn->pp = pp;
    }
    nn = nil;
    pp = nil;
}

/* Delete inferior */
void Base::DeleteInf (Base* dirt) {
    dirt->Unlink(this->inf);
    dirt->owner = nil;
}

/*
 * BaseName class. 
 */

BaseName::BaseName (What w, const char* s) : (w) {
    name = (s == nil) ? -2 : strTab->AddString(s);
}

BaseName::BaseName (What w, STindex i) : (w) {
    CellPointer cp = symTab->Ins2(i, this);
    name = i;
}

BaseName::~BaseName () {
    const char* theName = GetName();
    Data dummy;
    if (symTab->Get(theName, dummy)) {
        symTab->Delete(theName);
    }
}

void BaseName::Enter () {
    symTab->Ins2(name, this);
}

/*
 * InputFile class
 */

InputFile::InputFile (What id, const char* s, FILE* f)
: (id, s) {
    file = f;
    if (currProg->tinclink == false || id == LIBRARY) {
        updateTime = UpdateTime(s);
    }
}

InputFile::~InputFile () {
    const char* theName = GetName();
    Data dummy;
    if (units->Get(theName, dummy)) {
        units->Delete(theName);
    }
}

void InputFile::WriteText (FILE*) {
    Panic("InputFile::WriteText");
}

void InputFile::WriteData (FILE*) {
    Panic("InputFile::WriteData");
}

void InputFile::WriteSyms (FILE*) {
    Panic("InputFile::WriteSyms");
}

void InputFile::WriteStrTab (FILE*) {
    Panic("InputFile::WriteStrTab");
}

void InputFile::Update () {
    time_t newTime = UpdateTime(GetName());
    if (newTime > updateTime) {
        Reread();
        updateTime = newTime;
    }
}

void InputFile::Reread () {
    Panic("InputFile::Reread");
}

void InputFile::OpenFile () {
    file = fopen(GetName(), "r");
}

void InputFile::CloseFile () {
    if (file != nil) {
        fclose(file);
        file = nil;
    }
}
