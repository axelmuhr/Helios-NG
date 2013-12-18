// $Header: mapkey.h,v 1.6 88/09/24 15:07:18 interran Exp $
// declares class MapKey.

#ifndef mapkey_h
#define mapkey_h

#include <InterViews/defs.h>

// Declare imported types.

class Interactor;

// A MapKey maps characters to Interactors.

static const int MAXCHAR = 127;	// Maximum value of any character.

class MapKey {
public:

    MapKey();

    void Enter(Interactor*, char);
    Interactor* LookUp(char);
    const char* ToStr(char);

protected:

    Interactor* array[MAXCHAR + 1]; // stores Interactors by character

};

#endif
