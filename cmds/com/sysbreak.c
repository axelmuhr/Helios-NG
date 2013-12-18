/*
 * Helios system image breaker
 *
 * Reads an existing nucleus and extracts some bits 
 *
 * Author : BLV 29/4/91
 */
#include <stdio.h>
#include <stdlib.h>
#include <helios.h>
#include <stddef.h>
#include "getargs.h"

#ifdef __SUN4
#define EXIT_FAILURE -1
#define EXIT_SUCCESS  0
#endif

#define eq ==
#define ne !=

int swap_bytes = FALSE;

/* macro to avoid unnecessary function call */
#define swap_(w)	((swap_bytes)?swap(w):(w))

long swap (long w)
{
    long	new_w = 0;
    
    new_w = (w >> 24) & 0xff;
    new_w |= ((w >> 16) & 0xff) << 8;
    new_w |= ((w >> 8) & 0xff) << 16;
    new_w |= (w & 0xff) << 24;

    return new_w;
}

static void usage(void);

int main(int argc,STRING *argv)
{ FILE *nucleus_source;
  FILE *output_file;
  BYTE *image;
  WORD *w_image;
  WORD size;
  int  i;
  bool no_problems = TRUE;
  int ptabsize;
  ArgStack *argstack = NULL;

  if (argc < 3) usage();

  nucleus_source = fopen(argv[1], "rb");
  if (nucleus_source eq Null(FILE))
   { fprintf(stderr, "sysbreak: unable to open nucleus %s\n", argv[1]);
     exit(EXIT_FAILURE);
   }

  if (fread((BYTE *) &size, sizeof(WORD), 1, nucleus_source) < 1)
   { fputs("sysbreak: failed to read first word of nucleus.\n", stderr);
     exit(EXIT_FAILURE);
   }

#if 0
  /* C40 nuclei with an inbuit ROMdisk can easily be over 1 meg */
    /* check the nucleus size, to ensure that it is sensible */
  if ((size <= 1024) || (size >= (1024 * 1024)))
   { fprintf(stderr,
             "sysbreak: a nucleus size of %ld bytes is silly.\n", size);
     exit(EXIT_FAILURE);
   }
#else
  /* Attempt to determine whether swapping is required */
  if (   (unsigned)size & 0xf000000
      && (unsigned)size & 0x000000f == 0)
  {
      /*
       * Top byte is set, but bottom one isn't.  Implies that the
       * size is swapped around.
       */
      swap_bytes = TRUE;
      
      fprintf (stderr, "Suspect size (%d).  Using swapping\n", size);

      size = swap (size);

      fprintf (stderr, "	(swapped size is %d (0x%lx))\n", size, size);

      if (   (unsigned)size & 0xf0000000
	  && (unsigned)size & 0x0000000f == 0)
      {
	  fprintf (stderr, "Suspect swapped size.  I give up!\n");
	  exit (EXIT_FAILURE);
      }
  }
#endif   

  image = (byte *) malloc( (int) size);
  if (image eq Null(BYTE))
   { fprintf(stderr, 
       "sysbreak: cannot allocate a nucleus buffer of %ld bytes.\n", size);
     exit(EXIT_FAILURE);
   }
   
  *((WORD *) image) = size;	/* size already swapped */

#if 0
  if (fread(&(image[sizeof(WORD)]), 1, (int)size - sizeof(WORD), nucleus_source)
             < (size - sizeof(WORD)))
   { fputs("sysbreak: failed to read all of nucleus.\n", stderr);
     exit(EXIT_FAILURE);
   }
#else
  /* read in a word at a time, swapping as we go */
  {
      int	i;
      int	n_words = size / 4;	/* size is in bytes */
      long	w;
      
      if (size % 4 != 0 && swap_bytes)
      {
	  fprintf (stderr, "Nucleus size is not a multiple of words.  I give up!\n");
	  fclose (nucleus_source);
	  exit (EXIT_FAILURE);
      }
            
      for (i = 0; i < n_words; i++)
      {
	  if (fread ((char *)(&w), 1, 4, nucleus_source) != 4)
	  {
	      fprintf (stderr, "Failed to read word %d.  I give up!\n", i);
	      fclose (nucleus_source);
	  }
	  image[i * sizeof (long)] = swap_(w);
      }
  }
#endif  

  fclose(nucleus_source);

   /* skip command name and first argument */
  argv++; argv++; argc--; argc--;
  w_image = (WORD *) image;
  
  /* Count what we think argc should be */
  {
  	int 	keepargc = argc;
	STRING 	*keepargv = argv;
	int	cnt = 0;

	/* Count the real arguments */
	while ( (*argv != NULL) || (argstack != NULL) ) {
		if (*argv == NULL) {
			popargs(&argstack, &argc, &argv);
			continue;
		}

		if ( **argv == '@') {
			char **argfile;

			argfile = getargs((*argv)+1, &argc);
			if (argfile == NULL) { 
				fprintf(stderr, "Unable to open or create arguments from %s\n", (*argv)+1);
				exit(EXIT_FAILURE);
			} else {
				/* argv+1 is pushed so we return to the next argv */
				pushargs(&argstack, argc-1, argv+1);
				argv = argfile;
			}

			continue;
		}

		argv++;
		cnt++;
	}

	/* Restore settings */
	argc = keepargc;
	argv = keepargv;

	/* Free memory */
	freeargs();

	/* How many */
	ptabsize = cnt;
  }

  /* Now break up */  
  for (i = 0; i < ptabsize; i++)
   { WORD *base, *nextbase;
     BYTE *addr, *nextaddr;
     WORD component_size;
     char *imfile;

     /* argfile ? */
     while ( (*argv == NULL) || (**argv == '@') ) {
       if (*argv == NULL) {
         popargs(&argstack, &argc, &argv);
         continue;
       }

       if ( **argv == '@') {
         char **argfile;

         argfile = getargs((*argv)+1, &argc);
         if (argfile == NULL) { 
           fprintf(stderr, "Unable to open or create arguments from %s\n", (*argv)+1);
           exit(EXIT_FAILURE);
         } else {
           /* argv+1 is pushed so we return to the next argv */
           pushargs(&argstack, argc-1, argv+1);
           argv = argfile;
         }

         continue;
       }
     }

     /* The image */
     imfile = *argv++;

     /* Here we go ... */
     base = &(w_image[i + 1]);
     if (*base eq 0)
      { fprintf(stderr, "sysbreak: end of nucleus reached before producing %s\n",
                imfile);
        exit(EXIT_FAILURE);
      }
     addr = (BYTE *) base + *base;

     nextbase = &(base[1]);
     
     if (*nextbase eq 0)
      component_size = &(image[size]) - addr;
     else
      { nextaddr = (BYTE *) nextbase + *nextbase;
        component_size = nextaddr - addr;
      }
      
/*
     printf("Component %d: base %x, size is %d\n", i+1, addr, component_size);
     printf("            : starts with %x\n", *((word *) addr));
*/
     output_file = fopen(imfile, "wb");
     if (output_file eq Null(FILE))
      { fprintf(stderr, "sysbreak: failed to open output file %s\n",
      		imfile);
      	no_problems = FALSE;
        continue;
      }
      
     { WORD junk[3];
       junk[0] = 0x12345678;
       junk[1] = 0;
       junk[2] = component_size;
       if (fwrite((BYTE *) junk, sizeof(WORD), 3, output_file) ne 3)
        { fprintf(stderr, "sysbreak: failed to write program header to %s\n",
                  imfile);
          fclose(output_file);
          no_problems = FALSE;
          continue;
        }
     }
     
    if (fwrite(addr, 1, (int) component_size, output_file) ne component_size)
     { fprintf(stderr, "sysbreak: failed to write all of %s\n", imfile);
       fclose(output_file);
       no_problems = FALSE;
       continue;
     }
    
    fclose(output_file);
  }
  
  return(no_problems ? EXIT_SUCCESS : EXIT_FAILURE);
}

static void usage(void)
{ fputs("sysbreak: usage, sysbreak <nucleus> <outputfiles>\n", stderr);
  exit(EXIT_FAILURE);
}
