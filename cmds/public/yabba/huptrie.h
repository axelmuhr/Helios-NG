/* Placed into the public domain by Daniel J. Bernstein. */

#ifndef HUPTRIE_H
#define HUPTRIE_H

/* Ever seen an efficient data structure library? You're about to. */

/* Yes, folks, I have some documentation for this; I'm just not sure */
/* that the library is ready for general release yet. If you're */
/* desperate for huptries in an application and *need* to use this */
/* library *now*, let me know. */

/*#defines: PTRS, TYPE, HASHTYPE, ZEROFILLED, BZERO, MEMZERO, OPTCANT{1,2,5}.*/
/* HASHPTRS. */

#ifndef HASHTYPE
#define HASHTYPE TYPE
#endif

typedef unsigned TYPE pos;

#ifdef PTRS
typedef struct nod { struct nod *p; struct nod *n; } *node;
typedef struct nod *next;
typedef int * /*XXX: irrelevant*/ parent;
#else
typedef pos node;
typedef node *next;
typedef node *parent;
#endif

#ifdef HASHPTRS
typedef struct hod { node n; struct hod *h; } htt;
typedef struct hod *hash;
#define HASHP 255
#define HANO(h,sh) ((sh)->n)
#else
typedef node htt;
typedef unsigned HASHTYPE hash;
#define HASHP 0
#define HANO(h,sh) ((h)[sh])
#endif

typedef htt *hashtable;
typedef pos ipos;

/* A huptrie consists of:
   next n; parent p; hashtable h;
   ipos m; pos sm; pos l1; hash h1; 
*/

#define TOPHASH 5381

#ifdef HASHPTRS
#define hh(sh,sch,h1) (please do not use this yet, XXX)
#define hc(sh,c,h1) (sh[(unsigned char) c].h)
#else
#define hh(sh,sch,h1) (((sch) - (((sh) << 5) + (sh))) & (h1))
#define hc(sh,c,h1) ((((sh) << 5) + (sh) + ((hash) (unsigned char) (c))) & (h1))
#endif

#ifdef PTRS
#define NEXT(ne,no) ((no)->n)
#define PARENT(pa,no) ((no)->p)
#define AVAIL(ne) ((ne)[0].n)
#define NODEBYN(ne,po) ((ne) + (po))
#else
#define NEXT(ne,no) ((ne)[no])
#define PARENT(pa,no) ((pa)[no])
#define AVAIL(ne) ((ne)[0])
#define NODEBYN(ne,po) (po)
#endif

/* INIT, HUP_DELETE, DOWN are block statements. */

/* n,p,h,m,sm are by address */
#ifdef PTRS
#define INIT(n,p,h,m,sm,l1,h1) \
{ (n) = malloc(sizeof(struct nod) * ((l1) + 1)); AVAIL(n) = 0; \
 (h) = (hashtable) malloc(sizeof(htt) * ((h1) + HASHP + 1)); \
 CLEARHASH(h,h1); FIRSTHASH(h,h1); (m) = (l1) + 1; (sm) = -1; }
#define STATICDECLARE(n,p,h,l1,h1) \
  struct nod (n)[(l1) + 1]; parent (p); htt (h)[(h1) + HASHP + 1];
#else
#define INIT(n,p,h,m,sm,l1,h1) \
{ (n) = (next) malloc(sizeof(node) * ((l1) + 1)); AVAIL(n) = 0; \
 (p) = (parent) malloc(sizeof(node) * ((l1) + 1)); \
 (h) = (hashtable) malloc(sizeof(htt) * ((h1) + HASHP + 1)); \
 CLEARHASH(h,h1); FIRSTHASH(h,h1); (m) = (l1) + 1; (sm) = -1; }
#define STATICDECLARE(n,p,h,l1,h1) \
  node (n)[(l1) + 1]; node (p)[(l1) + 1]; htt (h)[(h1) + HASHP + 1];
#endif

#define STATICINIT(n,p,h,m,sm,l1,h1) \
{ INITHASH(h,h1); AVAIL(n) = 0; (m) = (l1) + 1; (sm) = -1; }
/* FIRSTHASH(h,h1)  XXX*/

#ifdef HASHPTRS
#define FIRSTHASH(h,h1) { hash ha; ha = (h) + ((h1) + HASHP + 1); \
do { --ha; ha->h = (h) + ((((ha - h) << 5) + (ha - h)) & (h1)); } \
while (ha != (h)); }
#else
#define FIRSTHASH(h,h1) ;
#endif

#ifdef ZEROFILLED
#define INITHASH(h,h1) ;
#else
#define INITHASH(h,h1) CLEARHASH(h,h1);
#endif

#ifdef HASHPTRS
#define CLEARHASH(h,h1) { hash ha; ha = (h) + ((h1) + 1); do HANO(h,--ha) = 0; while (ha != (h)); }
/* XXX: if -UZEROFILLED, this is unnecessarily inefficient on the first CLEARHASH */
#else
#ifdef BZERO
#define CLEARHASH(h,h1) (void) bzero((char *) (h),sizeof(node) * ((h1) + 1));
#else
#ifdef MEMZERO
#define CLEARHASH(h,h1) (void) memset((char *) (h),0,sizeof(node) * ((h1) + 1));
#else
#define CLEARHASH(h,h1) { hash ha; ha = (h1) + 1; do HANO(h,--ha) = 0; while (ha); }
#endif
#endif
#endif


/* sm and newnode are by address */
#define ADDAVAIL(n,p,h,sm,sp,sch,newnode) \
( --(sm), (newnode = AVAIL(n)), (AVAIL(n) = NEXT(n,newnode)), \
 (PARENT(p,newnode) = (sp)), \
 (NEXT(n,newnode) = HANO(h,sch)), (HANO(h,sch) = (newnode)) )

/* m, sm, newnode are by address */
#define WASTEMAX(n,p,h,m,sm,newnode) \
( (++(sm)), (newnode = NODEBYN(n,--(m))), (PARENT(p,newnode) = 0), \
 (NEXT(n,newnode) = AVAIL(n)), (AVAIL(n) = (newnode)) )

/* m and newnode are by address */
#define ADDMAX(n,p,h,m,sp,sch,newnode) \
( (newnode = NODEBYN(n,--(m))), (PARENT(p,newnode) = (sp)), \
 (NEXT(n,newnode) = HANO(h,sch)), (HANO(h,sch) = (newnode)) )

/* sm and noptr are by address */
#define HUP_DELETE(n,p,h,sm,sp,sh,noptr) \
{ (noptr) = &(HANO(h,sh)); while (*(noptr) != (sp)) (noptr) = &(NEXT(n,*(noptr))); \
 *(noptr) = NEXT(n,sp); PARENT(p,sp) = 0; NEXT(n,sp) = AVAIL(n); \
 AVAIL(n) = (sp); ++(sm); }

/* no is by address, FOUND and NOTFOUND are formal */
/* XXX: Reorganize hash lists? */
#define DOWN2(n,p,h,sp,sch,no,FOUND,NOTFOUND) { \
 if ((no) = HANO(h,sch)) { if (PARENT(p,no) != (sp)) { \
 if ((no) = NEXT(n,no))  { if (PARENT(p,no) != (sp)) { \
 if ((no) = NEXT(n,no))  { if (PARENT(p,no) != (sp)) { \
   do (no) = NEXT(n,no); while ((no) && (PARENT(p,no) != (sp))); \
   if (no) { FOUND } else { NOTFOUND } \
 } else { FOUND } } else { NOTFOUND } \
 } else { FOUND } } else { NOTFOUND } \
 } else { FOUND } } else { NOTFOUND } \
}

#define DOWN1(n,p,h,sp,sch,no,FOUND,NOTFOUND) { \
 if ((no) = HANO(h,sch)) { if (PARENT(p,no) != (sp)) { \
 if ((no) = NEXT(n,no))  { if (PARENT(p,no) != (sp)) { \
   do (no) = NEXT(n,no); while ((no) && (PARENT(p,no) != (sp))); \
   if (no) { FOUND } else { NOTFOUND } \
 } else { FOUND } } else { NOTFOUND } \
 } else { FOUND } } else { NOTFOUND } \
}

#define DOWN0(n,p,h,sp,sch,no,FOUND,NOTFOUND) { \
 if ((no) = HANO(h,sch)) { if (PARENT(p,no) != (sp)) { \
   do (no) = NEXT(n,no); while ((no) && (PARENT(p,no) != (sp))); \
   if (no) { FOUND } else { NOTFOUND } \
 } else { FOUND } } else { NOTFOUND } \
}

#define DOWN5(n,p,h,sp,sch,no,FOUND,NOTFOUND) { \
 if ((no) = HANO(h,sch)) { if (PARENT(p,no) != (sp)) { \
 if ((no) = NEXT(n,no))  { if (PARENT(p,no) != (sp)) { \
 if ((no) = NEXT(n,no))  { if (PARENT(p,no) != (sp)) { \
 if ((no) = NEXT(n,no))  { if (PARENT(p,no) != (sp)) { \
 if ((no) = NEXT(n,no))  { if (PARENT(p,no) != (sp)) { \
 if ((no) = NEXT(n,no))  { if (PARENT(p,no) != (sp)) { \
   do (no) = NEXT(n,no); while ((no) && (PARENT(p,no) != (sp))); \
   if (no) { FOUND } else { NOTFOUND } \
 } else { FOUND } } else { NOTFOUND } \
 } else { FOUND } } else { NOTFOUND } \
 } else { FOUND } } else { NOTFOUND } \
 } else { FOUND } } else { NOTFOUND } \
 } else { FOUND } } else { NOTFOUND } \
 } else { FOUND } } else { NOTFOUND } \
}

#ifdef OPTCANT1
#define DOWN DOWN0
#define DOWNI DOWN0
#else
#ifdef OPTCANT2
#define DOWN DOWN1
#define DOWNI DOWN1
#else
#ifdef OPTCANT5
#define DOWN DOWN2
#define DOWNI DOWN2
#else
#define DOWN DOWN5
#define DOWNI DOWN2
#endif
#endif
#endif

#define hup_parent(p,no) PARENT(p,no)
#define setparent(p,scp,sp) (PARENT(p,scp) = (sp))
#ifdef HASHPTRS
#define tophash(h,h1) ((h) + (TOPHASH & (h1)))
#else
#define tophash(h,h1) (TOPHASH & (h1))
#endif
#define topnode(n,l1) (NODEBYN(n,(l1) + 1))
#define Hmax(m,l1) ((l1) - (m))
#define size(m,sm,l1) (((l1) - (m)) - (sm))
#define limitm1(l1) (l1)
#define ipos2pos(n,ip,l1) ((l1) - (ip))
#define pos2ipos(n,po,l1) ((l1) - (po))

#ifdef PTRS
#define node2pos(n,no,l1) ((l1) - ((no) - (n)))
#define node2ipos(n,no,l1) ((no) - (n))
#else
#define node2pos(n,no,l1) ((l1) - (no))
#define node2ipos(n,no,l1) (no)
#endif

#define ipos2node(n,ip,l1) (NODEBYN(n,ip))
#define pos2node(n,po,l1) (NODEBYN(n,(l1) - (po)))

#endif
