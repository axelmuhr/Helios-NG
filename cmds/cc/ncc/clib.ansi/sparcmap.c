/* sparcmap.c:  Copyright (C) Codemist Ltd         */
/* Very SPARC specific routines.		     */

/* version 1 */

#include "hostsys.h"                            /* things like _initio() */
#include <stdlib.h>                             /* for exit()            */
#include <string.h>                             /* for strlen()          */
#include <ctype.h>                              /* for isprint()         */

typedef union count_position
{
    int i;
    struct s
    {   unsigned int posn:12,
                     line:16,
                     file:4;
    } s;
} count_position;

#define _codebase 0x2000
#define HEADER_WORD	0x000fbace      /* Magic number: function base */

extern void __mcount(void);
extern void mapstore(void), write_profile(void);

#define file_name_map_start 0xfff12340  /* Magic number */
#define file_name_map_end   0x31415926  /* Magic number */

#define word_roundup(s) ((char *)(((int)s + 3) & (~3)))

char *find_file_map(int p)
{
    int i, w;
    char *s;
    while (((w = *(int *)p) & 0xfffffff0) != file_name_map_start)
    {   if (p >= (int)__mcount) return "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
        p += 4;
    }
    s = (char *)(p + 4);
    for ( i = 0; i<=(w & 0xf); i++)
    {   s += 1 + strlen(s);
        s = word_roundup(s);
    }
    if (*(int *)s != file_name_map_end) return find_file_map((int)s);
    return (char *)(p + 4);
}

void _mapstore()
{
    int count = ((int)__mcount)>>2;  /* address of the function as an int */
    int p, onthisline = 0, w1 = 0;
    count_position posn;
    fprintf(stderr, "\nCounts from %4x to %p\n", _codebase, __mcount);
    for (p = (int)_codebase; p<(int)__mcount; p += 4)
    {   int w = *(int *)p;
	if (w == (0x40000000 + (unsigned int)count - (p>>2)))
				/* CALL mcount */
	  {   int i;
	      char *s;
	      fprintf(stderr, "%.6x: %-10u ", p, *(int *)(*(int *)(p + 8)));
	      posn.i = *(int *)(p + 12);
	      fprintf(stderr, "posn %d line %d file %d ",
		      posn.s.posn, posn.s.line, posn.s.file);
	      s = find_file_map(p+12);
	      for (i = 0; i<posn.s.file; i++)
		{ s += 1 + strlen(s);
		  s = word_roundup(s);
		}
	      fprintf(stderr, "(%s) ", s);
	      if (onthisline == 1) onthisline = 0, fputc('\n', stderr);
/*	      if (onthisline == 3) onthisline = 0, fputc('\n', stderr);   */
	      else onthisline++, fputc(' ', stderr);
          }
        if (w == HEADER_WORD)
	  {   char *name = (char *)(p - 4 - (w1 & 0xffff));
            int len = 0;
            char *temp = name;
            /* I do not use strlen() here to keep the library more modular */
            while (*temp++ != 0) len++;
            if (len >= 10 && onthisline == 3)
            {   onthisline = 0;
                fputc('\n', stderr);
            }
            len = fprintf(stderr, "%.6x: %s", p - 4, name);
            len = 19 - len;
            while (len < 0) len += 20, onthisline++;
            while (len > 0) len--, fputc(' ', stderr);
            if (onthisline >= 3) onthisline = 0, fputc('\n', stderr);
            else onthisline++, fputc(' ', stderr);
        }
        w1 = w;
    }
    if (onthisline != 0) fputc('\n', stderr);
}

void _write_profile(char *filename)
{
/* Create a (binary) file containing execution profile information for     */
/* the current program. The format is eccentric, and must be kept in step  */
/* with (a) parts of gen.c that generate code that collects statistics     */
/* and (b) code in misc.c that reads in the binary file created here and   */
/* displays the counts attached to a source listing of the original code.  */
    int count = ((int)__mcount)>>2;
    int p, w1 = 0, w2 = 0, pass, nfiles = 0, namebytes = 0, ncounts = 0;
    int global_name_offset[256];
    char *global_file_map[256]; /* Limits total number of files allowed */
    FILE *map_file = fopen(filename, "wb");
    char *file_map;
    if (map_file == NULL)
    {   fprintf(stderr, "\nUnable to open %s for execution profile log\n",
                        filename);
        return;
    }
    for (pass = 1; pass <=2; pass++)
    {
        if (pass == 2)
        /* Write file header indicating size of sub-parts */
        {   fwrite("\xff*COUNTFILE*", 4, 3, map_file);
            fwrite(&namebytes, 4, 1, map_file);
            fwrite(&nfiles,    4, 1, map_file);
            fwrite(&ncounts,   4, 1, map_file);
            for (p = 0; p < nfiles; p++)
            {   char *ss = global_file_map[p];
                int len = 1 + strlen(ss);
                len = ((len + 3) & (~3)) / 4;
                fwrite(ss, 4, len, map_file);
            }
            for (p = 0; p < nfiles; p++)
                fwrite(&global_name_offset[p], 4, 1, map_file);
        }
        file_map = NULL;
        for (p = (int)_codebase; p<(int)__mcount; p += 4)
        {   int w = *(int *)p;
	    if (w == (0x40000000 + (unsigned int)count - (p>>2)))
				/* CALL mcount */
            {   count_position k;
		int i;
		char *s;
		if (file_map == NULL ||
		    (int)file_map <= p) file_map = find_file_map(p+12);
		s = file_map;
		k.i = *(int *)(p + 8);
		for (i = 0; i<k.s.file; i++)
		  {   s += 1 + strlen(s);
		      s = word_roundup(s);
                  }
		if (pass == 1) {
		  int i;
		  for (i = 0;;i++) {
		    if (i >= nfiles) {
		      global_name_offset[nfiles] = namebytes;
		      global_file_map[nfiles++] = s;
		      namebytes += 1 + strlen(s);
		      namebytes = (namebytes + 3) & (~3);
		      break;
		    }
		    else if (strcmp(s,global_file_map[i]) ==0) break;
		  }
		  ncounts++;
		}
		else {
		  int i;
		  for (i = 0; strcmp(s, global_file_map[i]) !=0; i++);
		  fwrite((int *)(p + 4), 4, 1, map_file);
		  i = (k.s.line & 0xffff) | (i << 16);
		  fwrite(&i, 4, 1, map_file);
		}
		p += 8;
	      }
	    w2 = w1;
	    w1 = w;
	  }
      }
    fwrite("\xff*ENDCOUNT*\n", 4, 3, map_file); /* Trailer data */
    fclose(map_file);
    fprintf(stderr, "\nProfile information written to %s\n", filename);
}

/* end of sparcmap.c */
