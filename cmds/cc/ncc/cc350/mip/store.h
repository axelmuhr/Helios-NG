#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * store.h, version 6
 * Copyright (C) Codemist Ltd., 1988.
 *
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:02:42 $
 * Revising $Author: nickc $
 */

/* AM memo: I would like to improve the security of the following by   */
/* discouraging the use of macros involving casts (or void *).         */

#ifndef _store_LOADED
#define _store_LOADED 1

typedef enum {
    SU_Data,
    SU_Xref,
    SU_Xsym,
    SU_Sym,
    SU_Bind,
    SU_Type,
    SU_Const,
    SU_PP,
    SU_Dbg,
    SU_Other
} StoreUse;

#define global_cons2(t,a,b) xglobal_cons2(t,(int32)(a),(int32)(b))
#define global_list3(t,a,b,c) xglobal_list3(t,(int32)(a),(int32)(b),(int32)(c))
#define global_list4(t,a,b,c,d) xglobal_list4(t,(int32)(a),(int32)(b),(int32)(c),(int32)(d))
#define global_list5(t,a,b,c,d,e) xglobal_list5(t,(int32)(a),(int32)(b),(int32)(c),(int32)(d),(int32)(e))
#define global_list6(t,a,b,c,d,e,f) xglobal_list6(t,(int32)(a),(int32)(b),(int32)(c),(int32)(d),(int32)(e),(int32)(f))

extern VoidStar xglobal_cons2(StoreUse t, int32 a, int32 b);
extern VoidStar xglobal_list3(StoreUse t, int32 a, int32 b, int32 c);
extern VoidStar xglobal_list4(StoreUse t, int32 a, int32 b, int32 c, int32 d);
extern VoidStar xglobal_list5(StoreUse t, int32 a, int32 b, int32 c, int32 d, int32 e);
extern VoidStar xglobal_list6(StoreUse t, int32 a, int32 b, int32 c, int32 d, int32 e, int32 f);

extern VoidStar GlobAlloc(StoreUse t, int32 n);
extern VoidStar BindAlloc(int32 n);
extern VoidStar SynAlloc(int32 n);

extern VoidStar discard2(VoidStar p);
#define syn_cons2(a, b) xsyn_list2((int32)(a), (int32)(b))
#define binder_cons2(a, b) xbinder_list2((int32)(a), (int32)(b))
#define binder_icons2(a, b) xbinder_list2((int32)(a), b)
#define binder_icons3(a, b, c) xbinder_list3((int32)(a), b, c)

extern VoidStar discard3(VoidStar p);

#define syn_list2(a,b) xsyn_list2((int32)(a),(int32)(b))
#define syn_list3(a,b,c) xsyn_list3((int32)(a),(int32)(b),(int32)(c))
#define syn_list4(a,b,c,d) xsyn_list4((int32)(a),(int32)(b),(int32)(c),(int32)(d))
#define syn_list5(a,b,c,d,e) xsyn_list5((int32)(a),(int32)(b),(int32)(c),(int32)(d),(int32)(e))
#define syn_list6(a,b,c,d,e,f) xsyn_list6((int32)(a),(int32)(b),(int32)(c),(int32)(d),(int32)(e),(int32)(f))
#define syn_list7(a,b,c,d,e,f,g) xsyn_list7((int32)(a),(int32)(b),(int32)(c),(int32)(d),(int32)(e),(int32)(f),(int32)(g))

extern VoidStar xsyn_list2(int32 a, int32 b);
extern VoidStar xsyn_list3(int32 a, int32 b, int32 c);
extern VoidStar xsyn_list4(int32 a, int32 b, int32 c, int32 d);
extern VoidStar xsyn_list5(int32 a, int32 b, int32 c, int32 d, int32 e);
extern VoidStar xsyn_list6(int32 a, int32 b, int32 c, int32 d, int32 e,
                           int32 f);
extern VoidStar xsyn_list7(int32 a, int32 b, int32 c, int32 d, int32 e,
                           int32 f, int32 g);

#define binder_list2(a,b) xbinder_list2((int32)(a),(int32)(b))
#define binder_list3(a,b,c) xbinder_list3((int32)(a),(int32)(b),(int32)(c))

extern VoidStar xbinder_list2(int32 a, int32 b);
extern VoidStar xbinder_list3(int32 a, int32 b, int32 c);

extern void alloc_mark(void);
extern void alloc_unmark(void);
extern void drop_local_store(void);
extern void alloc_reinit(void);

extern void alloc_init(void);
extern void alloc_noteAEstoreuse(void);
extern void show_store_use(void);
extern void alloc_dispose(void);

#endif
