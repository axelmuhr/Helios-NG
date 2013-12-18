#ifndef tripostypes
#define tripostypes 1

#ifdef __STDC__
#define NOARGS void
#else
#define NOARGS
#endif

typedef char  BYTE;
typedef short SHORT;
typedef long  LONG;
typedef long  APTR;
typedef long  BOOL;
typedef unsigned char  UBYTE;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;

/* All BCPL data must be LONG word aligned.  BCPL pointers are the LONG word
 *  address (i.e byte address divided by 4 (>>2)) */
typedef long  BPTR;                 /* Long word pointer */
typedef long  BSTR;                 /* Long word pointer to BCPL string  */
#define BADDR( bptr ) (bptr << 2)   /* Convert BPTR to typical C pointer */
#endif
