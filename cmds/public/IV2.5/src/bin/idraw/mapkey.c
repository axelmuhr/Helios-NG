// $Header: mapkey.c,v 1.6 88/09/24 15:07:15 interran Exp $
// implements class MapKey.

#include "mapkey.h"
#include <ctype.h>
#include <stdio.h>

// MapKey clears MapKey's array to all nils.

MapKey::MapKey () {
    for (int i = 0; i <= MAXCHAR; i++) {
	array[i] = nil;
    }
}

// Enter enters an Interactor into a slot in MapKey's array.  If the
// slot doesn't exist or another Interactor already occupies it,
// Enter prints a warning message.

void MapKey::Enter (Interactor* i, char c) {
    if (c >= 0 && c <= MAXCHAR) {
	if (array[c] == nil) {
	    array[c] = i;
	} else {
	    fprintf(stderr, "MapKey: slot %d already occupied!\n", c);
	}
    } else {
	fprintf(stderr, "MapKey: slot %d not in array!\n", c);
    }
}

// LookUp returns the Interactor associated with the given character
// or nil if there's no Interactor or the character's out of bounds.

Interactor* MapKey::LookUp (char c) {
    if (c >= 0 && c <= MAXCHAR) {
	return array[c];
    } else {
	fprintf(stderr, "MapKey: slot %d not in array!\n", c);
	return nil;
    }
}

// ToStr returns a printable string representing the given character.
// The caller must copy the returned string because the next call of
// ToStr will change it.

const char* MapKey::ToStr (char c) {
    static char key[3];
    if (!isprint(c)) {
	c = toascii(c + 0100);
	key[0] = '^';
    } else {
	key[0] = ' ';
    }
    key[1] = c;
    key[2] = 0;
    return key;
}
