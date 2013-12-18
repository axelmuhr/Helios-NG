/* alloc.c -- replacement malloc and friends
 *
 * author :  Sandra Loosemore
 * date   :  24 Oct 1987
 *
 * This file is a replacement for the usual definitions of malloc and free.
 * The ST Malloc routine is called to get more memory from the system.
 * Note that the memory that is Malloc'ed is never Mfree'd.
 *
 */


#include <osbind.h>
#define NULL 0L


/* This is the default size for grabbing memory from the system with Malloc.
 *	If you try to malloc a piece bigger than this, it will Malloc a
 *	piece exactly the right size.  Ideally, it should be quite a bit
 *	larger than the average size of things you are allocating (assuming
 *	you are going to allocate lots of them).
 */

static long mchunk = 16384L;           /* Size for grabbing memory */


/* Each call to malloc returns a chunk; freeing it puts the chunk on
 *	the free list.  The "next" field is only used for freed blocks.
 *      The "nbytes" field is the size of the body of the chunk, not
 *	the entire chunk.
 */

typedef struct chunk {
    long nbytes;
    union {
        struct chunk *next;
        char body[1];
        } info;
    } CHUNK;

#define Nbytes(c) (c)->nbytes
#define Next(c) (c)->info.next
#define Body(c) (c)->info.body

static CHUNK *freelist = NULL;


/* Return a block at least "size" bytes long.  Grab more memory if there
 * 	isn't anything on the free list that big.  If the block is just big
 * 	enough, remove it from the free list and return the whole thing.
 *	Otherwise, carve a piece off the front.
 * Note that the size is rounded up to make it an even number, and it must
 *	also be at least big enough to hold the next pointer when the block
 *	is freed.
 */

char *alloc (size)
    long size;
{   register CHUNK *this, *last, *new;
    long temp;
    size = (size + 1) & 0xfffe;			           /* Word alignment */
    if (size < sizeof(CHUNK *))  size = sizeof(CHUNK *);   /* Minimum size */
    this = freelist;
    last = NULL;
    while (this != NULL)  {
        if (Nbytes(this) >= size)  break;
        last = this;
        this = Next(this);
        }
    if (this == NULL)  {
        temp = ((size < mchunk) ? mchunk : size);
        this = (CHUNK *) Malloc (temp + sizeof(long));
        if (!this)  return(NULL);
        Nbytes(this) = temp;
	free(Body(this));
        this = freelist;
	last = NULL;
	while (this != NULL)  {
            if (Nbytes(this) >= size)  break;
            last = this;
            this = Next(this);
	    }
        }
    temp = Nbytes(this) - size - sizeof(long);
    if (temp <= sizeof(CHUNK))  {               /* Use the whole thing */
        if (last)
	    Next(last) = Next(this);
	else
	    freelist = Next(this);
        return(Body(this));
        }
    else  {                                         /* Grab some off end */
        new = (CHUNK *) ((char *)this + size + sizeof(long));
	Nbytes(new) = temp;
	Next(new) = Next(this);
	if (last)
	    Next(last) = new;
	else
	    freelist = new;
	Nbytes(this) = size;
	return(Body(this));
        }
    }



/* These are the user-accessible entry points */

char *malloc (size)
    unsigned size;
{   return (alloc((long)size));
    }

#if 0	/* calloc not used in mg */
char *calloc (number, size)
    unsigned number, size;
{   return (alloc ((long)number*size));
    }
#endif

char *realloc (oldptr, newsize)
    register char *oldptr;
    unsigned newsize;
{   long oldsize;
    register char *newptr;
    register unsigned i;
    CHUNK *block;
    block = (CHUNK *) (oldptr - sizeof(long));
    oldsize = Nbytes(block);
    if (newsize > oldsize)  {
        newptr = alloc((long)newsize);
        for (i=0; i<oldsize; i++)
	    newptr[i] = oldptr[i];
        free(oldptr);	    
	return(newptr);
        }
    else
        return(oldptr);
    }

    
/* Free a pointer.  The freelist is maintained in sorted order, so loop
 *	through until we find the block on either side of the one we're
 *	freeing.  Then see if we can merge this block with either one.
 */

free (ptr)
    char *ptr;
{   register CHUNK *last, *this, *block;

	/* Find where to insert the block in the free list. */

    block = (CHUNK *)(ptr - sizeof(long));
    this = freelist;
    last = NULL;
    while (this && (this < block))  {
        last = this;
	this = Next(this);
	}

	/* Can we merge it with the next block?  */

    if (this && ((ptr + Nbytes(block)) == this))  {
        Nbytes(block) = Nbytes(block) + Nbytes(this) + sizeof(long);
	Next(block) = Next(this);
	}
    else
        Next(block) = this;

	/* Can we merge it with the previous block?  */

    if (last && ((Body(last) + Nbytes(last)) == block))  {
        Nbytes(last) = Nbytes(last) + Nbytes(block) + sizeof(long);
	Next(last) = Next(block);
        }
    else if (last)
        Next(last) = block;
    else
        freelist = block;
    }


/* A debug routine to help me make sure that the freelist is compacted
 *	properly.
 */

#ifdef DEBUG

dumpfree ()
{   CHUNK *junk;
    printf ("Dump of free list:\n");
    junk = freelist;
    while (junk)  {
        printf ("    Base %ld, size %ld\n", (long)(Body(junk)), Nbytes(junk));
	junk = Next(junk);
	}
    }
#endif
