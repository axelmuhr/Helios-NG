/* $Id: vm.h,v 1.4 1992/09/25 10:41:21 paul Exp $ */


#define VMmask		0xf
#define VMswapped	1
#define VMdirty		2
#define VMtouched	8

#define VMagebit	0x40000000

typedef struct VMpage {
	int		status;
	char		*addr;
	int		filepage;
	int		left;
	int		locks;
	int		age;
	int		swapsin;
	int		swapsout;
} VMpage;

#ifdef __STDC__

typedef struct VMRef {
	unsigned 	offset : 16;
	unsigned	page   : 16;
} VMRef;

#define offset_(x)	((x).offset)
#define page_(x)	((x).page)

#define VMSame(x,y) ((page_(x) == page_(y)) && (offset_(x) == offset_(y)))

#define MakeVMRef(r,p,o) ((r).page = p, (r).offset = o)

#else

typedef long	VMRef;

#define offset_(x)	((x)&0xffff)
#define page_(x)	((x)>>16)

#define VMSame(x,y)	((x) == (y))

#define MakeVMRef(r,p,o) ((r) = ((p)<<16) | (o))

#endif

#ifdef VM_DEBUG

#define VMAddr(type,x) ((type *)VMswap(x))

#else

#define VMAddr(type,x) \
((type *)(((VMTable[page_(x)].status|=VMtouched)&VMswapped)==VMswapped? \
VMswap(x):VMTable[page_(x)].addr+offset_(x)))

#endif
		
#define VMcheck(v) ((*(int *)&v > 0) && (page_(v) > 0) && (page_(v) < VMTabUpb) && (offset_(v) < VMPageSize))

#define VMDirty(x) {if(VMTable[page_(x)].status&VMswapped)VMswap(x);VMTable[page_(x)].status|=VMdirty;}

#define NullRef(x) (*(int *)&(x) == 0)

extern VMpage *VMTable;
extern char *VMfilename;
extern VMRef NullVMRef;
extern int VMTabUpb;
extern int VMPageSize;
extern int VMLeave;

#ifdef __STDC__
extern APTR VMswap(VMRef v);
extern VMRef VMalloc(int size, VMRef v);
extern int VMInit(char *vmfile,int pagesize);
extern VMRef VMPage(void);
extern int VMleft(VMRef v);
extern void VMlock(VMRef v);
extern void VMunlock(VMRef v);
extern VMRef VMnext(VMRef v);
extern void VMLRU(void);
extern void VMTidy(void);
extern void VMStats(void);
extern void showvm(void);
#else
extern APTR VMswap();
extern VMRef VMalloc();
extern int VMInit();
extern VMRef VMPage();
extern int VMleft();
extern void VMlock();
extern void VMunlock();
extern VMRef VMnext();
extern void VMLRU();
extern void VMTidy();
extern void VMStats();
extern void showvm();
#endif
