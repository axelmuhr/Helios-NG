#ifndef __FILE_H
#define __FILE_H

#include <helios.h>

#define PathSplitOK  (-1)
#define DOStoHelios  (0)
#define HeliosToDOS  (1)

extern INT PathSplit (STRING Path,
                      STRING Dir,
                      STRING Name);

#endif
