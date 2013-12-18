/* varargs.h.ins.cc  -- varargs for both 32ix and 64v mode C unser primos */
/* 02/22/87 Robert Larson  Cleanup  (Variable name changes to avoid conflicts) */

#ifdef __CI
/* varargs.h for 32ix mode on a Prime 9955 */
/* Modified from the Primix varargs.h by James M Synge. */
typedef char *va_list;

#define va_alist       _va_arg1
#define va_dcl	       char va_alist;
#define va_start(list) list = ((va_list)(&_va_arg1))
#define va_end(list)

#define va_arg(list,mode)	((mode *)((list) += sizeof(mode)))[-1]

#else

typedef int **va_list;
extern short *stackptr$();

#define va_alist \
	/* first argument at SB%+45 */ \
	_v0,_v1,_v2,_v3,_v4,_v5,_v6,_v7,_v8,_v9,_v10, \
	_v11,_v12,_v13,_v14,_v15,_v16,_v17,_v18,_v19,_v20, \
	_v21,_v22,_v23,_v24,_v25,_v26,_v27,_v28,_v29,_v30, \
	_v31,_v32,_v33,_v34,_v35,_v36,_v37,_v38,_v39,_v40, \
	_v41,_v42,_v43,_v44,_v45,_v46,_v47,_v48,_v49,_v50, \
	_v51,_v52,_v53,_v54,_v55,_v56,_v57,_v58,_v59,_v60, \
	_v61,_v62,_v63,_v64,_v65,_v66,_v67,_v68,_v69

#define va_dcl int *_v0;
#define va_start(list) list = ((va_list)(stackptr$() + 042))
#define va_end(list)
#define va_arg(list, mode) ( (sizeof(mode) == sizeof(char *)) ? \
			     ((**(mode **)((list)++)))	: \
			     ((mode) (**((list)++))) )
#endif
