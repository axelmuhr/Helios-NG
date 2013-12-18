/* 
 * Unix system interface
 */
 
#ifndef system_h
#define system_h

#include "types.h"

extern void ReadBuff(FILE*, Address, int, const char* =nil, int exitStatus =2);
extern void WriteBuff(FILE*, Address, int);

extern time_t UpdateTime (const char*);
extern const char* SegTypeToStr(SegmentType st);
extern const char* SearchPath(const char* name, const char* path);
extern const char* StripPath(const char*);
extern FileType GetFileType(FILE*);
extern unsigned short GetMode(FILE*);
extern char* FullPath(char* const);

inline int SegToNType (SegmentType st) { return (int)st; }

inline int RoundUp (int i, int mod) { return ((i + mod - 1) / mod) * mod; }

/* 
 * return next multiple of alignSize greater than or equal to i 
 * Assumes alignSize is a multiple of 2!!
 */
inline int AlignTo (int alignSize, int i) {
    return (i + alignSize - 1) & ~(alignSize - 1);
}

extern const char* arMagicString;
static const int arMagicLen = 8;    /* strlen(arMagicString) */

#endif
