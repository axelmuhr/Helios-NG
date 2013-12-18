/*------------------------------------------------------------------------
--                                                                      --
--			         brom.c					--
--                     ---------------------------                      --
--                                                                      --
--             Copyright (c) 1990, Active Book Company Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--                                                                      --
-- Builds a ROM image containing multiple ROM ITEMs.			--
-- 									--
--	Author:  JGS 900227						--
--	Updated: JGS 901009		new header formats		--
--									--
-- options:								--
--	-h	Help							--
--	-v	Verbose mode on						--
--	-o	ROM image file name					--
--	-c	Create (new) empty ROM image file			--
--	-s	Specify the "system" ITEM for this ROM image file	--
--	-a	Add file(s) to the ROM image file			--
--	-d	Delete file(s) from the ROM image file (TODO)		--
--	-l	List ITEMs in the ROM image file			--
--									--
-- Notes:								--
--	Since the size is fixed at create time, the utility will	--
--	complain if there is not enough room for a particular ITEM.	--
--									--
--	The ROM image file is padded with 0xFF characters.		--
--									--
--	The "system" ITEM is a special file that contains a branch	--
--	instruction aswell as its ITEM header.				--
--									--
--	To give the greatest flexibility we store the complete ROM	--
--	image data in memory.						--
--									--
-- TODO:								--
--	There is a lot of code duplication. The program could be	--
--	simplified and shortened by boiling this down to a minimum	--
--	set of functions.						--
--									--
-- EXTENSIONS:								--
--	Allow options to manipulate the headers of individual items.	--
--	(changing the version, name, flags etc.)			--
--									--
-- ERRORS:								--
--	exit 0	OK (operation successful)				--
--	exit 1	Help given (command usage)				--
--	exit 2	Invalid HEX number given				--
--	exit 3	File not found						--
--	exit 4	Failed to open file					--
--	exit 5	Failed to seek within file				--
--	exit 6	Failed to read/write data from/to file			--
--									--
--	exit 8	Invalid ITEM header					--
--	exit 9	Operation not yet supported				--
--	exit 10	Not enough memory for operation				--
--	exit 11	ITEM will not fit into the image			--
--	exit 12	Image must be word-aligned				--
--	exit 13	Unrecognised argument					--
--	exit 14	Image file not specified				--
--	exit 15	Corrupt image file					--
------------------------------------------------------------------------*/

static char *SccsId=" %W% %G% Copyright (c) 1990, Active Book Company Ltd.\n";

#include <stdio.h>
#include <alloc.h>
#include <time.h>

#include <sys/errno.h>
extern int errno ;

/*---------------------------------------------------------------------------*/

#define	FALSE	0
#define TRUE	1

#define whoami		"brom"	    /* The name of the utility */
#define version		"0.13"      /* The version of the utiity */

#define buffsize	(0x8000)    /* 32K default buffer size (no smaller) */
#define maxstring	(256)	    /* maximum string size */

#define bell    (0x07)
#define htab    (0x09)
#define lf      (0x0A)
#define cls     (0x0C)
#define cr      (0x0D)
#define tab     (0x1F)
#define space   (0x20)
#define quote   (0x22)
#define fstop   ('.')
#define slash   ('/')

#define wcmult  ('*')
#define wcsing  ('#')

#define tolower(c)      ((c) | 0x20)
#define makelower(c)	(((c < 'A')||(c > 'Z')) ? (c) : tolower(c))

typedef unsigned int  word ;
typedef unsigned char byte ;
 
/*--------------------------------------------------------------------------*/

#include "/hsrc/include/abcARM/PCcard.h"	/* external CARD format */
#include "/hsrc/include/abcARM/ROMitems.h"	/* ITEM formats */

/*--------------------------------------------------------------------------*/
/* This is the list used to hold all the items internally */

typedef struct litem {
		      struct litem *next ;	/* next list entry */
		      struct litem *last ;	/* previous list entry */
		      char	   *data ;	/* ITEM data */
		      word          datalen ;	/* ITEMLength */
                     } litem ;

/*---------------------------------------------------------------------------*
 * compare two objects
 * in:  fptr -> full object name (NULL terminated)
 *      gptr -> wildcarded object name (NULL terminated)
 * out: boolean flag - TRUE (matched) - FALSE (failed match)
 */
static int wild_card_compare(char *fptr, char *gptr)
{
 if (*fptr == NULL)
  {
   if (*gptr == wcmult)
    return(wild_card_compare(fptr,&gptr[1])) ;
   else
    return(*gptr == NULL) ;
  }

 if (*gptr == NULL)
  return(FALSE) ;

 if ((makelower(*fptr) == makelower(*gptr)) || (*gptr == wcsing))
  return(wild_card_compare(&fptr[1],&gptr[1])) ;

 if (*gptr == wcmult)
  return(wild_card_compare(fptr,&gptr[1])||wild_card_compare(&fptr[1],gptr)) ;
 else
  return(FALSE) ;
}

/*--------------------------------------------------------------------------*/

static void usage(void)
{
 fprintf(stderr,"%s builds (or updates) a ROM image (v%s %s %s)\n\n",whoami,version,__DATE__,__TIME__) ;
 fprintf(stderr," -h       : provide help on the %s utility\n",whoami) ;
 fprintf(stderr," -v       : verbose\n",whoami) ;
 fprintf(stderr," -o file  : ROM image file name\n") ;
 fprintf(stderr," -c size  : Create a new ROM image file (size in hex)\n") ;
 fprintf(stderr," -s file  : Specify the \"system\" item for this ROM image file\n") ;
 fprintf(stderr," -l       : List the items in the ROM image file\n") ;
 fprintf(stderr," -a files : Add file(s) to the ROM image file\n") ;
 fprintf(stderr," -d files : Delete file(s) from the ROM image file\n") ;
 fprintf(stderr,"\n") ;
 fprintf(stderr,"Usage: %s [-v] [-o] image [-c size] [-s file] [-l] [-a|-d file ..]\n",whoami) ;
 exit(1) ;
}

/*--------------------------------------------------------------------------*/

static word readhex(char *cptr)
{
 word value = 0x00000000 ;
 int loop ;

 if ((cptr == NULL) || (*cptr == '\0'))
  {
   fprintf(stderr,"%s: NULL hex number given\n",whoami) ;
   exit(2) ;
  }

 for (loop = 0; (loop < 8); loop++)
  {
   if (*cptr == '\0')
    break ;

   *cptr = toupper(*cptr) ;

   if (((*cptr < '0') || (*cptr > '9')) && ((*cptr < 'A') || (*cptr > 'F')))
    {
     fprintf(stderr,"%s: Invalid HEX digit given\n",whoami) ;
     exit(2) ;
    }

   value = ((value << 4) | ((*cptr <= '9') ? (*cptr - '0') : ((*cptr - 'A') + 10))) ;
   cptr++ ;
  }

 if (*cptr != '\0')
  {
   fprintf(stderr,"%s: Too many HEX digits given\n",whoami) ;
   exit(2) ;
  }

 return(value) ;
}

/*---------------------------------------------------------------------------*/

static void displaylist(litem *ilist,char *imfile,word imsize,int verbose)
{
 litem  *cptr = ilist ;	/* list of ITEMs in the image */
 char   *name ;		/* ITEM name */
 int    cnt ;		/* ITEM count */
 word 	offset = 0 ;	/* offset into the image */

 if (verbose)
  printf("List of the ITEMs held in image file \"%s\":\n",imfile) ;

 if (cptr == NULL)
  {
   printf("<No ITEMs in image file>\n") ;
   return ;
  }

 if (verbose)
  printf("T   Offset   Length  ") ;

 printf("Version Name\n") ;

 printf("------------------------------------------------------------------\n") ;
 cnt = 0 ;
 while (cptr != NULL)
  {
   ITEMstructure    *fitem = (ITEMstructure *)cptr->data ;
   ROMITEMstructure *fromitem ;
   int		     sysfound = FALSE ;

   cnt++ ;

   if (fitem->ITEMID != ITEMMagic)
    {
     fitem = (ITEMstructure *)((int)fitem + sizeof(word)) ;
     offset = sizeof(word) ;
     sysfound = TRUE ;
    }

   name = (char *)&fitem->ITEMName[0] ;

   fromitem = (ROMITEMstructure *)((int)&fitem->ITEMName[0] + fitem->ITEMNameLength) ;

   if (verbose)
    printf("%c%c %08X %08X ",((sysfound) ? 'S' : ' '),((fitem->ITEMExtensions & ITEMhdrBRANCH) ? 'B' : ' '),offset,fitem->ITEMLength) ;

   printf(" v%1X.%02X  %s\n",((fromitem->ITEMVersion & 0x00000F00) >> 8),(fromitem->ITEMVersion & 0x000000FF),name) ;

   offset += fitem->ITEMLength ;
   cptr = cptr->next ;
  }

 if (verbose)
  {
   printf("------------------------------------------------------------------\n") ;
   printf("%d ITEM%s (&%08X bytes used, &%08X bytes free)\n",cnt,((cnt == 1)?"":"s"),offset,(imsize - offset)) ;
  }

 return ;
}

/*---------------------------------------------------------------------------*/
/* checkimage:
 * returns:	TRUE	image exists		(size updated)
 *		FALSE	image does NOT exist	(size preserved)
 */
static int checkimage(char *imfile,word *size)
{
 FILE *tf ;
 
 /* check if we can open the given file */
 /* updating "size" if we can */
 
 if ((tf = fopen(imfile,"r")) == NULL)
  return(FALSE) ;

 fflush(tf) ;
 if (fseek(tf,0,2/* should be SEEK_END */) != 0)
  {
   fprintf(stderr,"%s: Unable to seek to end of file \"%s\"\n",whoami,imfile) ;
   exit(5) ;
  }
 *size = ftell(tf) ;
 fclose(tf) ;

 return(TRUE) ;
}

/*---------------------------------------------------------------------------*/

static void do_create(char *imfile,word imsize,int verbose)
{
 char            *tbuff = (char *)malloc(buffsize) ; /* temporary buffer */
 int              amount = buffsize ;		     /* default packet size */
 int	          length = imsize ;		     /* total amount */
 FILE	         *imf ;				     /* image file handle */
 int		  loop ;			     /* general index */

#ifdef DEBUG
 printf("do_create: \"%s\" (size &%08X)\n",imfile,imsize) ;
#endif /* DEBUG */

 if (tbuff == NULL)
  {
   fprintf(stderr,"%s: Not enough memory for IO buffers\n",whoami) ;
   exit(10) ;
  }
   
 if (verbose)
  printf("Creating ROM image file \"%s\" of &%08X bytes\n",imfile,imsize) ;

 /* create empty (0xFF) file of the specified size */
 if ((imf = fopen(imfile,"w")) == NULL)
  {
   fprintf(stderr,"%s: Cannot create \"%s\" for writing\n",whoami,imfile) ;
   exit(4) ;
  }

 /* fill the buffer with the default data value */
 for (loop = 0; (loop < buffsize); loop++)
  tbuff[loop] = 0xFF ;

 while (length > 0)
  {
   if (length < buffsize)
    amount = length ;
   if (fwrite(tbuff,amount,1,imf) != 1)
    {
     fprintf(stderr,"%s: Unable to write data to \"%s\"\n",whoami,imfile) ;
     exit(6) ;
    }
   length -= amount ;
  }

 free(tbuff) ;
 fclose(imf) ;	/* should flush any unwritten buffers */
 return ;
}

/*---------------------------------------------------------------------------*/

static int processimage(char *imfile,word imsize,word *imfree,litem **ilist)
{
 litem          *cptr ;			/* current ITEM list */
 word            vword ;		/* current word value */
 word            offset = 0 ;		/* offset into image */
 FILE           *imf ;			/* image file handle */
 litem          *newc = NULL ;		/* current new ITEM pointer */
 ITEMstructure  *newitem = NULL ;	/* ITEM header structure pointer */

#ifdef DEBUG
 printf("processimage: scan current image contents\n") ;
#endif /* DEBUG */

 cptr = *ilist = NULL ;			/* initialise the list */

 if ((imf = fopen(imfile,"r")) == NULL)
  {
   fprintf(stderr,"%s: Cannot read image file \"%s\"\n",whoami,imfile) ;
   exit(4) ;
  }

 if ((newitem = (ITEMstructure *)malloc(sizeof(ITEMstructure))) == NULL)
  {
   fprintf(stderr,"%s: Not enough memory for IO buffers\n",whoami) ;
   exit(10) ;
  }

 /* deal with the "system" ITEM at the front of the image */
 if (fread((char *)newitem,((int)&newitem->ITEMName[0] - (int)newitem),1,imf) != 1)
  {
   fprintf(stderr,"%s: Unable to read ITEM header from \"%s\"\n",whoami,imfile) ;
   exit(6) ;
  }

 if (newitem->ITEMID != ITEMMagic)
  {
   ITEMstructure *fitem = newitem ;
   /* If the first word is NOT "ITEMMagic" check to see if the second is */
   fitem = (ITEMstructure *)((int)newitem + sizeof(word)) ;
   if (fitem->ITEMID == ITEMMagic)
    {
     if (fitem->ITEMExtensions & ~(ITEMhdrROM | ITEMhdrRAM | ITEMhdrBRANCH))
      {
       fprintf(stderr,"%s: ITEM extension bits are non-zero (update program)\n",whoami) ;
       exit(8) ;
      }
     if (!(fitem->ITEMExtensions & ITEMhdrROM))
      {
       fprintf(stderr,"%s: ITEM header is not for a ROM ITEM\n",whoami) ;
       exit(8) ;
      }

     if ((newc = (litem *)malloc(sizeof(litem))) == NULL)
      {
       fprintf(stderr,"%s: Not enough memory for ITEM descriptor\n",whoami) ;
       exit(10) ;
      }
 
     /* allocate space large enough for the "BRANCH word" and ITEM */
     newc->datalen = sizeof(word) + fitem->ITEMLength ;
     if ((newc->data = (char *)malloc(newc->datalen)) == NULL)
      {
       fprintf(stderr,"%s: not enough memory for ITEM data\n",whoami) ;
       exit(10) ;
      }

     /* load this ITEM with the extra word at the beginning */
     fflush(imf) ;
     if (fseek(imf,0,0) != 0)
      {
       fprintf(stderr,"%s: Unable to seek to start of image\n",whoami) ;
       exit(5) ;
      }
     if (fread(newc->data,newc->datalen,1,imf) != 1)
      {
       fprintf(stderr,"%s: Unable to read data from \"%s\"\n",whoami,imfile) ;
       exit(6) ;
      }

     /* place ITEM descriptor at the front of the list */
     newc->next = NULL ;	/* reference end of list */
     newc->last = NULL ;	/* in both directions */

     cptr = *ilist = newc ;	/* this is the head of the list */

     offset = cptr->datalen ;	/* step over the loaded data */
    }
  } 

 /* "offset" = the start of the first ITEM */
 fflush(imf) ;
 if (fseek(imf,offset,0/* should be "SEEK_SET" */) != 0)
  {
   fprintf(stderr,"%s: Unable to seek to start of ITEMs\n",whoami) ;
   exit(5) ;
  }

 /* and start processing the ITEMs */
 do
  {
   if ((offset & (sizeof(int) - 1)) != 0)
    {
#ifdef DEBUG
     printf("Start of ITEM not on word boundary\n") ;
#endif /* DEBUG */
     free(newitem) ;
     fclose(imf) ;
     return(FALSE) ;
    }

#ifdef DEBUG
   printf("Offsets: real = &%08X, soft = &%08X\n",ftell(imf),offset) ;
#endif /* DEBUG */

   if ((newc = (litem *)malloc(sizeof(litem))) == NULL)
    {
     fprintf("%s: Not enough memory for ITEM descriptor\n",whoami) ;
     exit(10) ;
    }

   /* copy the ITEM descriptor from the file into RAM */
   if (fread((char *)newitem,((int)&newitem->ITEMName[0] - (int)newitem),1,imf) != 1)
    {
     fprintf(stderr,"%s: Unable to read ITEM header from \"%s\"\n",whoami,imfile) ;
     exit(6) ;
    }

   /* check the validity of the ITEM header */
   if ((vword = newitem->ITEMID) == ITEMMagic)
    {
     if (newitem->ITEMExtensions & ~(ITEMhdrROM | ITEMhdrRAM))
      {
       fprintf(stderr,"%s: ITEM extension bits are non-zero (update program)\n",whoami) ;
       exit(8) ;
      }
     if (!(newitem->ITEMExtensions & ITEMhdrROM))
      {
       fprintf(stderr,"%s: ITEM header is not for a ROM ITEM\n",whoami) ;
       exit(8) ;
      }

#ifdef DEBUG
     printf("ITEM header read OK\n") ;
#endif /* DEBUG */

     /* seek to the start of the ITEM again, and load the whole object into
      * memory.
      */
     newc->datalen = newitem->ITEMLength ;	/* complete item length */
     if ((newc->data = (char *)malloc(newc->datalen)) == NULL)
      {
       fprintf(stderr,"%s: not enough memory for ITEM data\n",whoami) ;
       exit(10) ;
      }
     fflush(imf) ;
     if (fseek(imf,offset,0/* should be "SEEK_SET" */) != 0)
      {
       fprintf(stderr,"%s: Unable to seek to start of ITEM\n",whoami) ;
       exit(5) ;
      }
     if (fread(newc->data,newc->datalen,1,imf) != 1)
      {
       fprintf(stderr,"%s: unable to read ITEM from \"%s\"\n",whoami,imfile) ;
       exit(6) ;
      }

     /* add this item into the list */
     newc->next = NULL ;	  /* end of list */
     newc->last = cptr ;	  /* previous is current */

     if (cptr != NULL)
      cptr->next = newc ;	  /* current next is new */
     else
      *ilist = newc ;		  /* this is the head of the list */
     cptr = newc ;		  /* and this is the new current ITEM */
     
#ifdef DEBUG
     printf("cptr->datalen = &%08X\n",cptr->datalen) ;
#endif /* DEBUG */

     /* and update the pointer ready for the next object */
     offset += cptr->datalen ;
     if (offset > imsize)
      {
       free(newitem) ;
       fclose(imf) ;
       return(FALSE) ;		/* ITEM steps off the end of the image */
      }
      
     fflush(imf) ;
     if (fseek(imf,offset,0) != 0)
      {
       fprintf(stderr,"%s: Unable to seek within image file\n",whoami) ;
       exit(5) ;
      }
    }
  } while ((vword == ITEMMagic) && (offset != imsize)) ;

 *imfree = offset ;		/* index of the first free word in the image */

 /* return to the start of the image file */
 fflush(imf) ;
 if (fseek(imf,0,0/* should be "SEEK_SET" */) != 0)
  {
   fprintf(stderr,"%s: Unable to seek to start of image file\n",whoami) ;
   exit(5) ;
  }

 free(newitem) ;
 fclose(imf) ;
 return(TRUE) ;		/* valid image file */
}

/*---------------------------------------------------------------------------*/

static void do_sysadd(litem **ilist,char *sysfile)
{
 FILE           *infile ;
 litem          *newc = (litem *)malloc(sizeof(litem)) ;
 ITEMstructure  *thisitem = NULL ;

#ifdef DEBUG
 printf("Add system item \"%s\"\n",sysfile) ;
#endif /* DEBUG */

 /* The system ITEM must be placed at the start of the image, since it
  * contains the "BRANCH word" aswell as its ITEM header.
  */

 if (newc == NULL)
  {
   fprintf(stderr,"%s: not enough memory for ITEM descriptor\n",whoami) ;
   exit(10) ;
  }

 /* place the "system" image at the start of the ITEM list */
 if (!checkimage(sysfile,&newc->datalen))
  {
   fprintf(stderr,"%s: file \"%s\" not found (%d)\n",whoami,sysfile,errno) ;
   exit(3) ;
  }

#ifdef DEBUG
 printf("System item is &%08X bytes long\n",newc->datalen) ;
#endif

 if ((infile = fopen(sysfile,"r")) == NULL)
  {
   fprintf(stderr,"%s: unable to open file \"%s\"\n",whoami,sysfile) ;
   exit(4) ;
  }

 if ((newc->data = (char *)malloc(newc->datalen)) == NULL)
  {
   fprintf(stderr,"%s: not enough memory for ITEM data\n",whoami) ;
   exit(10) ;
  }

 if (fread(newc->data,newc->datalen,1,infile) != 1)
  {
   fprintf(stderr,"%s: unable to read system ITEM \"%s\"\n",whoami,sysfile) ;
   exit(6) ;
  }
 fclose(infile) ;

#ifdef DEBUG
 printf("System file read and closed\n") ;
#endif

 /* Check that the system ITEM given does contain a ITEM header */
 thisitem = (ITEMstructure *)((int)newc->data + sizeof(word)) ;
 if (thisitem->ITEMID != ITEMMagic)
  {
   fprintf(stderr,"%s: system ITEM does NOT contain a valid header\n",whoami) ;
   exit(8) ;
  }

 /* add the item onto the head of the list */
 newc->next = *ilist ;
 newc->last = NULL ;
 if (*ilist != NULL)
  (*ilist)->last = newc ;
 else
  *ilist = newc ;

 return ;
}

/*---------------------------------------------------------------------------*/

static void do_add(char *ifile,litem **ilist)
{
 litem         *newc = (litem *)malloc(sizeof(litem)) ;
 litem	       *cptr = *ilist ;
 FILE	       *infile ;
 ITEMstructure *fitem = NULL ;

 if (newc == NULL)
  {
   fprintf(stderr,"%s: not enough memory for ITEM descriptor\n",whoami) ;
   exit(10) ;
  }
   
 /* New ITEMs are added at the end of an image */
 if (!checkimage(ifile,&newc->datalen))
  {
   fprintf(stderr,"%s: file \"%s\" not found (%d)\n",whoami,ifile,errno) ;
   exit(3) ;
  }

#ifdef DEBUG
 printf("ITEM filesize = &%08X\n",newc->datalen) ;
#endif /* DEBUG */

 if (newc->datalen & (sizeof(int) - 1))
  {
   fprintf(stderr,"%s: ITEM \"%s\" is NOT word-aligned\n",whoami,ifile) ;
   exit(12) ;
  }

 if (newc->datalen == 0) /* NULL files are NOT ROM ITEMs */
  {
   fprintf(stderr,"%s: Zero length ITEM \"%s\"\n",whoami,ifile) ;
   exit(12) ;
  }

 if ((infile = fopen(ifile,"r")) == NULL)
  {
   fprintf(stderr,"%s: unable to open file \"%s\"\n",whoami,ifile) ;
   exit(4) ;
  }

 if ((newc->data = (char *)malloc(newc->datalen)) == NULL)
  {
   fprintf(stderr,"%s: Not enough memory for ITEM \"%s\"\n",whoami,ifile) ;
   exit(10) ;
  }

 if (fread(newc->data,newc->datalen,1,infile) != 1)
  {
   fprintf(stderr,"%s: unable to read data from ITEM \"%s\"\n",whoami,ifile);
   exit(6) ;
  }
 fclose(infile) ;

 fitem = (ITEMstructure *)newc->data ;
 /* If the specified file does NOT have a ROM item header then
  * we must generate a suitable error.
  */
 if (fitem->ITEMID != ITEMMagic)
  {
   fprintf(stderr,"%s: file \"%s\" is not a ROM ITEM\n",whoami,ifile) ;
   exit(8) ;
  }

#ifdef DEBUG
 printf("File has ITEM magic word\n") ;
 printf("ITEM to be added onto internal list structure\n") ;
 printf("*ilist = &%08X, newc = &%08X\n",*ilist,newc) ;
#endif /* DEBUG */
 /* find the end of the current ITEM list */
 for (cptr=*ilist; ((cptr != NULL) && (cptr->next != NULL)); cptr=cptr->next) ;

 /* add the ITEM onto the end of the list */
 newc->next = NULL ;
 newc->last = cptr ;
 if (cptr != NULL)
  cptr->next = newc ;
 else
  *ilist = newc ;

 return ;
}

/*---------------------------------------------------------------------------*/

static void do_delete(char *ifile,litem **ilist)
{
 litem          *cptr = *ilist ;
 ITEMstructure  *fitem ;
 char           *iname ;

#ifdef DEBUG
 printf("do_delete: remove \"%s\" from the list &%08X\n",ifile,*ilist) ;
#endif /* DEBUG */

 while (cptr != NULL)
  {
   /* deal with ITEMs with that may have "BRANCH word" headers */
   fitem = (ITEMstructure *)cptr->data ;
   if (fitem->ITEMID != ITEMMagic)
    fitem = (ITEMstructure *)((int)cptr->data + sizeof(word)) ;

   iname = (char *)&fitem->ITEMName[0] ;
   if (wild_card_compare(iname,ifile))
    {
     litem *oldptr = cptr ;
     if (oldptr->last != NULL)
      oldptr->last->next = oldptr->next ;
     if (oldptr->next != NULL)
      oldptr->next->last = oldptr->last ;
     cptr = oldptr->next ;
     if (oldptr == *ilist)
      *ilist = cptr ;
     free(oldptr) ;
    }
   else
    cptr = cptr->next ;
  }

 return ;
}

/*---------------------------------------------------------------------------*/

void update_image(litem *ilist,char *imfile,word imsize)
{
 FILE           *imf ;	/* image file handle */
 litem          *cptr = ilist ;
 ITEMstructure  *fitem = NULL ;
 word		 used = 0x00000000 ;
 word		 tword = 0x00000000 ;

#ifdef DEBUG
 printf("update_image: \"%s\" (cptr = &%08X)\n",imfile,(word)cptr) ;
#endif /* DEBUG */

 if ((imf = fopen(imfile,"r+")) == NULL)
  {
   fprintf(stderr,"%s: Cannot update image file \"%s\"\n",whoami,imfile) ;
   exit(4) ;
  }

 if (cptr != NULL)
  {
   /* If the ITEM at the root of the "ilist" chain is actually a system ITEM,
    * we should overwrite the ITEM at the start of the image file.
    */
   fitem = (ITEMstructure *)cptr->data ;
   if (fitem->ITEMID != ITEMMagic)
    {
#ifdef DEBUG
     printf("First ITEM in list has invalid ITEMMagic\n") ;
#endif /* DEBUG */
     /* assume the chain has not been corrupted and treat as the system ITEM */
     if ((used + cptr->datalen) > imsize)
      {
       fprintf(stderr,"%s: ITEM will not fit into the image\n",whoami) ;
       exit(11) ;
      }

     if (fwrite(cptr->data,cptr->datalen,1,imf) != 1)
      {
       fprintf(stderr,"%s: Unable to write data to \"%s\"\n",whoami,imfile) ;
       exit(6) ;
      }
     used += cptr->datalen ;
     cptr = cptr->next ;
    }
  }

 /* normal processing */
 while (cptr != NULL)
  {
   if ((used + cptr->datalen) > imsize)
    {
     fprintf("%s: ITEM will not fit into the image\n",whoami) ;
     exit(11) ;
    }

#ifdef DEBUG
   printf("Writing &%08X bytes to index &%08X\n",cptr->datalen,used) ;
#endif /* DEBUG */

   /* write "cptr->data" to the given image file */
   if (fwrite(cptr->data,cptr->datalen,1,imf) != 1)
    {
     fprintf(stderr,"%s: Unable to write data to \"%s\"\n",whoami,imfile) ;
     exit(6) ;
    }
   used += cptr->datalen ;
   cptr = cptr->next ;
  } 

 /* The ITEM list is NULL terminated. The image is padded with 0xFF chars */
 if (used < imsize)
  if (fwrite((char *)&tword,sizeof(word),1,imf) != 1)
   {
    fprintf(stderr,"%s: Unable to write date to \"%s\"\n",whoami,imfile) ;
    exit(6) ;
   }

 used += sizeof(word) ;	/* count NULL word */

#ifdef DEBUG
 printf("&%08X bytes used in image file (size &%08X)\n",used,imsize) ;
#endif

 tword = 0xFFFFFFFF ;
 for (; (used < imsize); used += sizeof(word))
  {
   if (fwrite((char *)&tword,sizeof(word),1,imf) != 1)
    {
     fprintf(stderr,"%s: Unable to write padding to \"%s\"\n",whoami,imfile) ;
     exit(6) ;
    }
  }

 fclose(imf) ;
 return ;
}

/*---------------------------------------------------------------------------*/

int main(int argc,char **argv)
{
 int    index ;			/* used in argument processing */
 int    loop ;			/* used in argument processing */
 char  *cp ;			/* used in argument processing */
 char   imfile[maxstring] ;	/* image file name */
 char   sysfile[maxstring] ;	/* system ITEM name */
 int    imexists = FALSE ;	/* does the image file exist */
 int    create = FALSE ;	/* are we creating a new image */
 word   imfree = 0 ;		/* offset of first free space */
 word   imsize = 0 ;		/* size of the ROM image file */
 int    listflag = FALSE ;	/* do not list the ROM items */
 int    filelist = 0 ;		/* index of first file name in list */
 int    sysadd = FALSE ;	/* are we adding a system item */
 int    adding = TRUE ;		/* wether adding or deleting files */
 int    verbose = FALSE ;	/* wether we print lots of stats */
 int    update = FALSE ;	/* are we updating the image */
 litem *ilist ;			/* list of ITEMs in current image */

 imfile[0] = '\0' ;		/* no image file */
 sysfile[0] = '\0' ;		/* no system file */

 for (index=1; (index < argc); index++)
  {
   if (argv[index][0] == '-')
    {
     switch(argv[index][1])
      {
       case 'o' : /* output (image) file */
		  if (argv[index][2] == '\0')
		   cp = argv[++index] ;
		  else
		   cp = &argv[index][2] ;

                  for (loop=0; (*cp && (*cp != ' ')); cp++)
		   imfile[loop++] = *cp ;
		  imfile[loop] = '\0' ;
		   
		  /* check if the file exists */
		  imexists = checkimage(imfile,&imsize) ;
		  break ;

       case 'c' : /* create item of specified size */
		  if (argv[index][2] == '\0')
		   cp = argv[++index] ;
		  else
		   cp = &argv[index][2] ;
		  imsize = readhex(cp) ;
		  if (imsize & 3)
		   {
		    fprintf(stderr,"%s: ROM image size must be word multiple\n",whoami) ;
		    exit(12) ;
		   }
	
		  /* if the item already exists we want to delete it and
		   * create a new empty file
		   */
		  create = TRUE ;
		  break ;

       case 'l' : /* list the items in the ROM image */
		  listflag = TRUE ;	
		  break ;

       case 's' : /* specify the system ROM item file */
		  if (argv[index][2] == '\0')
		   cp = argv[++index] ;
		  else
		   cp = &argv[index][2] ;

		  /* copy the name from "cp" */
                  for (loop=0; (*cp && (*cp != ' ')); cp++)
		   sysfile[loop++] = *cp ;
		  sysfile[loop] = '\0' ;
		  sysadd = TRUE ;
		  update = TRUE ;
		  filelist = ++index ;
		  break ;

       /* NOTE: these options are mutually exclusive */
       case 'a' : /* add the remaining arguments as files */
       		  adding = TRUE ;
       		  update = TRUE ;
       		  filelist = ++index ;
       		  index = argc ;	/* terminate the argument processing */
       		  break ;

       case 'd' : /* delete the remaining arguments from the image */
		  adding = FALSE ;
		  update = TRUE ;
                  filelist = ++index ;
                  index = argc ;	/* stop argument processing */
		  break ;	

       /* and the following are simple modifiers */
       case 'v' : /* verbose mode enable */
		  verbose = TRUE ;
       		  break ;

       case 'h' : /* help requested */
       default  : /* unrecognised option */
		  usage() ;
      }
    }
   else
    {
     /* default output (image) file */
     if (imfile[0] == '\0')
      {
       cp = argv[index] ;
       for (loop=0; (*cp && (*cp != ' ')); cp++)
        imfile[loop++] = *cp ;
       imfile[loop] = '\0' ;
		   
       /* check if the file exists */
       imexists = checkimage(imfile,&imsize) ;
      }
     else
      {
       fprintf(stderr,"%s: unrecognised arg \"%s\"\n",whoami,argv[index]) ;
       exit(13) ;
      }
    }
  }
 /* end of arg processing:
  * imfile	= image filename or NULL
  * create	= TRUE then creating new empty image file
  * imexists    = TRUE then image exists
  * imsize	= length of image if "imexists" or "create" TRUE
  * update	= TRUE then performing add or delete operation
  * sysadd	= TRUE then adding "system" ITEM
  * sysfile	= system ITEM filename if "sysadd" TRUE
  * adding      = TRUE then adding given files, otherwise deleting given files
  * verbose	= TRUE then verbose operation, else quiet operation
  * filelist	= argv index of start of ITEM files
  * listflag	= TRUE if ITEM listing required
  */

 if (imfile[0] == '\0')
  {
   fprintf(stderr,"%s: image file not given\n",whoami) ;
   exit(14) ;
  }

#ifdef DEBUG
 if (create && imexists)
  printf("File already exists, it will be overwritten\n") ;
#endif /* DEBUG */

 if (create)
  do_create(imfile,imsize,verbose) ;
 else
  {
   if (!imexists)
    {
     fprintf(stderr,"%s: Image file \"%s\" does NOT exist\n",whoami,imfile) ;
     exit(3) ;
    }
   if (verbose && update)
    printf("Updating ROM image file \"%s\"\n",imfile) ;
  }

 /* "imfile" should exist (being "imsize" bytes long) */
#ifdef DEBUG
 printf("Image file \"%s\" (size = &%08X)\n",imfile,imsize) ;
#endif /* DEBUG */

 /* open and scan the image file */
 if (update)
  {
   litem *cptr ;
   
   /* create structure describing the items held in this image */
   if (processimage(imfile,imsize,&imfree,&ilist) != TRUE)
    {
     fprintf(stderr,"%s: Corrupt image file \"%s\"\n",whoami,imfile) ;
     exit(15) ;
    }

   /* find the end of the current ITEM list */
   for (cptr = ilist; ((cptr != NULL) && (cptr->next != NULL)); cptr = cptr->next) ;

   /* deal with the system item (if any) */
   if (sysadd)
    do_sysadd(&ilist,sysfile) ;

#ifdef DEBUG
   printf("About to start processing file arguments\n") ;
#endif

   /* actually perform the updating */
   for (loop = filelist; (loop < argc); loop++)
    {
#ifdef DEBUG
     printf("%s = \"%s\"\n",(adding?"Add":"Delete"),argv[loop]) ;
#endif /* DEBUG */

     if (adding)
      do_add(argv[loop],&ilist) ;
     else /* deleting */
      do_delete(argv[loop],&ilist) ;
    }

   if (listflag)
    displaylist(ilist,imfile,imsize,verbose) ;

   /* and write the modified list back to the image */
   update_image(ilist,imfile,imsize) ;
  }
 else
  {
   if (listflag)
    {
     /* create structure describing the items held in this image */
     if (processimage(imfile,imsize,&imfree,&ilist) != TRUE)
      {
       fprintf(stderr,"%s: Corrupt image file \"%s\"\n",whoami,imfile) ;
       exit(15) ;
      }

     displaylist(ilist,imfile,imsize,verbose) ;
    }
  }

 exit(0) ;
 SccsId = SccsId ; /* remove warning */
}

/*--------------------------------------------------------------------------*/
/*> EOF brom.c <*/
