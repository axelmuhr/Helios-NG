/*
 * Hello Mic:
 *
 *   After mailing the first message to you about the free() function I
 * decide on a solution similar to the one you sugested in your reply. There
 * is one disadvantage with this solution it requires an extra 4 bytes for
 * the link information beyond the original 8 bytes already required by each
 * block.  If we can find a clean way of deallocating stuff within EMACS such
 * as killing each buffer and all the other structures we will save 8 bytes
 * per allocation (leaving only the 4 bytes for the block size). Anyways
 * here is my doubly linked list version of free() which works very fast
 * but takes up a bit more memory.
 *
 *      Later,
 *         Leon
 *
 */

/* name: malloc.c
 * desc: An improved malloc() and free() function for Aztec c
 * date: 04/02/87 LF
 * note: This takes over the Aztec _cln() function that is called by
 *	 the startup code once main() returns.
 */

#define NULL 0L

struct mem {
	struct mem *next, *prev;
	long size;
};

static struct mem *Free;

void *_AllocMem();

static
cleanup()
{
	register struct mem *mp, *xp;

	for (mp=Free;mp;mp=xp) {
		xp = mp->next;
		_FreeMem(mp, mp->size+sizeof(struct mem));
	}
	Free = 0;
}

char *
lmalloc(size)
unsigned long size;
{
	register struct mem *ptr;
	extern int (*_cln)();

	_cln = cleanup;
	if ((ptr = _AllocMem(size+sizeof(struct mem), 0L)) == 0)
		return(0);
	ptr->next = Free;
        if (Free != NULL)
        {
          Free->prev = ptr;
        }
        ptr->prev = NULL;
	ptr->size = size;
	Free = ptr;
	return((char *)ptr + sizeof(struct mem));
}

char *
malloc(size)
unsigned size;
{
	return(lmalloc((unsigned long)size));
}

free(blk)
char *blk;
{
	register struct mem *mp, *xp, *xn;

        mp = (struct mem*)(blk - sizeof(struct mem));
        xp = mp->prev;
        xn = mp->next;

        if (xn != NULL)
        {
          xn->prev = xp;
        }

        if (xp == NULL)
        {
          Free = xn;
        }
        else
        {
          xp->next = xn;
        }

	_FreeMem(mp, mp->size+sizeof(struct mem));
	return(0);
}
