/******************************************************************************
**
**
**	FILE		:	X_Open/search.h
**
**
**	DESCRIPTION	:	X-Open header file : <search.h>
**
**
******************************************************************************/


/* definitions for hsearch() */

typedef struct entry {
	char	*key;
	char	*data;
} ENTRY;


typedef enum {
	FIND,
	ENTER
} ACTION;


/* definition for tsearch() */

typedef enum {
	preorder,
	postorder,
	endorder,
	leaf
} VISIT;

extern	void *lsearch(void *key, void *base, size_t *nelp, size_t width, int (*compar)());
extern	void *lfind(void *key, void *base, size_t *nelp, size_t width, int (*compar)());

extern	int hcreate(unsigned nel);
extern	void hdestroy(void);
extern	ENTRY *hsearch(ENTRY item, ACTION act);

extern	void *tdelete(void *key, void **rootp, int (*compar)());
extern	void *tfind(void *key, void **rootp, int (*compar)());
extern	void *tsearch(void *key, void **rootp, int (*compar)());
extern	void twalk(void *root, void (*action)());
