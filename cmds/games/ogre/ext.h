#include "ogre.h"

#ifdef MAIN

UNIT unit[N_UNITS];
OGRE ogre;
int n_units;

#else

extern UNIT unit[N_UNITS];
extern OGRE ogre;
extern int n_units;

#endif

