#ifndef _CTYPE
#define _CTYPE

extern int isalnum(int c);
extern int isalpha(int c);
extern int iscntrl(int c);
extern int isdigit(int c);
extern int isgraph(int c);
extern int islower(int c);
extern int isprint(int c);
extern int ispunct(int c);
extern int isspace(int c);
extern int isupper(int c);
extern int isxdigit(int c);
extern int tolower(int c);
extern int toupper(int c);

#define _U      0001    /* Upper case */
#define _L      0002    /* Lower case */
#define _N      0004    /* Numeral (digit) */
#define _S      0010    /* Spacing character */
#define _P      0020    /* Punctuation */
#define _C      0040    /* Control character */
#define _X      0100    /* Hexadecimal */
#define _B      0200    /* Blank */

extern  unsigned short *_pctype;
extern  unsigned short _ctype__[];

#define isalnum(c)      ((_pctype+1)[c]&(_U|_L|_N))
#define isalpha(c)      ((_pctype+1)[c]&(_U|_L))
#define iscntrl(c)      ((_pctype+1)[c]&_C)
#define isdigit(c)      ((_ctype__+1)[c]&_N)
#define isgraph(c)      ((_pctype+1)[c]&(_P|_U|_L|_N))
#define islower(c)      ((_pctype+1)[c]&_L)
#define isprint(c)      ((_pctype+1)[c]&(_P|_U|_L|_N|_B))
#define ispunct(c)      ((_pctype+1)[c]&_P)
#define isspace(c)      ((_pctype+1)[c]&_S)
#define isupper(c)      ((_pctype+1)[c]&_U)
#define isxdigit(c)     ((_ctype__+1)[c]&(_N|_X))

#define isascii(c)      ((unsigned)(c)<=0177)
#define toascii(c)      ((c)&0177)

#define _toupper(c)     ((c)-'a'+'A')
#define _tolower(c)     ((c)-'A'+'a')

#endif
