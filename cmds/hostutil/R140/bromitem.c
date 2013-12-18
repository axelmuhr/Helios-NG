/*------------------------------------------------------------------------
--                                                                      --
--			       bromitem.c				--
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1989, Active Book Company Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--                                                                      --
-- Builds a ROM ITEM containing the given object.			--
-- 									--
--	Author:  JGS 891114						--
--	Updated: JGS 901008	new header formats			--
--									--
------------------------------------------------------------------------*/

static char *SccsId=" %W% %G% Copyright (c) 1989, Active Book Company Ltd.\n";

#include <stdio.h>
#include <time.h>

#define	FALSE	0
#define TRUE	1

#define whoami		"bromitem"	/* The name of the utility */
#define version		"0.03"		/* version of this program */
#define maxstring	256		/* maximum string length */

typedef unsigned char byte ;
typedef unsigned int  word ;

/*--------------------------------------------------------------------------*/

#include "/hsrc/include/abcARM/ROMitems.h" /* ITEM definitions */

/*--------------------------------------------------------------------------*/

void usage()
{
 fprintf(stderr,"%s v%s (%s at %s)\n",whoami,version,__DATE__,__TIME__) ;
 fprintf(stderr,"Usage: %s [-o outfile] [-n name] [-v version] [-f flags] [-a] infile\n",whoami) ;
 fprintf(stderr,"-o outfile : destination binary\n") ;
 fprintf(stderr,"-n name    : ROM item name\n") ;
 fprintf(stderr,"-v version : BCD item version number\n") ;
 fprintf(stderr,"-f matrix  : Helios Access matrix (hexadecimal)\n") ;
 fprintf(stderr,"-a         : pad complete item to word boundary (align)\n") ;
 exit(1) ;
}

/*--------------------------------------------------------------------------*/

unsigned int readhex(char *cptr)
{
 unsigned int value = 0x00000000 ;
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
     fprintf(stderr,"%s: invalid HEX digit given\n",whoami) ;
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

int readword(FILE *inf,unsigned int *value)
{
 return((fread((char *)value,sizeof(unsigned int),1,inf) == 1) ? 0 : -1) ;
}

/*---------------------------------------------------------------------------*/

int main(int argc,char **argv)
{
 FILE		  *outf = stdout ;
 FILE		  *inf = stdin ;
 int		   i ;
 char		   iname[maxstring] ;
 ITEMstructure     item ;	/* ITEM header */
 ROMITEMstructure  romitem ;	/* ROM ITEM specific header */
 char		  *cp ;
 int		   loop ;
 unsigned int	   imagelen = 0 ;
 char		  *basename = NULL ;
 char		   cval ;
 int		   alignsource = FALSE ;
 int 		   alignpadding = 0 ;
 unsigned int	   buildtime = time(NULL) ;
 int 		   hdrsize ;
 int		   excess ;

 item.ITEMID = ITEMMagic ;		/* magic identifier */
 item.ITEMAccess = defaultITEMaccess ;	/* default access matrix */
 item.ITEMDate[0] = 0x00000000 ;	/* useconds not attainable */
 item.ITEMDate[1] = buildtime ;
 item.ITEMExtensions = ITEMhdrROM ; /* ROM header by default */

 sprintf(iname,"<<UNDEFINED>>") ;	/* default name */
 /* NOTE: ITEMNameLength not initialised here */

 romitem.OBJECTInit = 0x00000000 ;	/* no function */
 romitem.ITEMVersion = 0x00000000 ;	/* default version */

 /* structure size upto the variable length name field */
 hdrsize = ((int)&item.ITEMName[0] - (int)&item) ;

 for (i=1; (i < argc); i++)
  {
   if (argv[i][0] == '-')
    {
     switch(argv[i][1])
      {
       case 'o' : /* output file */
		  if (argv[i][2] == '\0')
		   cp = argv[++i] ;
		  else
		   cp = &argv[i][2] ;
#ifdef __STDC__ /* ANSI */
		  if ((outf = fopen(cp,"wb")) == NULL)
#else
		  if ((outf = fopen(cp,"w")) == NULL)
#endif
		   {
		    fprintf(stderr,"%s: Cannot open output file\n",whoami) ;
		    exit(2) ;
		   }
		  break ;

       case 'n' : /* item name */
		  if (argv[i][2] == '\0')
		   cp = argv[++i] ;
		  else
		   cp = &argv[i][2] ;
		  /* copy the NULL or space terminated name */
		  if (strlen(cp) >= maxstring)
		   {
		    fprintf(stderr,"%s: item name too long\n",whoami) ;
		    exit(3) ;
		   }
		  for (loop=0; (*cp && (*cp != ' ')); cp++)
		   iname[loop++] = *cp ;
		  iname[loop] = '\0' ;
		  break ;

       case 'v' : /* item version number */
		  if (argv[i][2] == '\0')
		   cp = argv[++i] ;
		  else
		   cp = &argv[i][2] ;
		  romitem.ITEMVersion = readhex(cp) ;
		  break ;

       case 'f' : /* item flags */
		  if (argv[i][2] == '\0')
		   cp = argv[++i] ;
		  else
		   cp = &argv[i][2] ;
		  item.ITEMAccess = readhex(cp) ;
		  break ;

       case 'a' : /* align the source data to a word-boundary */
                  alignsource = TRUE ;
                  break ;
			
       default  : /* unrecognised option */
		  usage() ;

      }
    }
   else
    {
     if ((inf = fopen(argv[i],"r")) == NULL)
      {
       fprintf(stderr,"%s: cannot open input file\n",whoami) ;
       exit(3) ;
      }
     basename = argv[i] ;
    }
  }
 /* end of arg processing */

 if (basename == NULL)
  {
   fprintf(stderr,"%s: image file not given\n",whoami) ;
   exit(3) ;
  }

#ifdef DEBUG
 printf("Generating ROM item for file \"%s\"\n",basename) ;
 printf("(Created %s)\n",ctime(&buildtime)) ;
#endif /* DEBUG */
 
 /* calculate the input item size */
 if (fseek(inf,0,2 /* should be "SEEK_END" */) != 0)
  {
   fprintf(stderr,"%s: unable to seek to end of file\n",whoami) ;
   exit(3) ;
  }
 imagelen = ftell(inf) ;
 if (fseek(inf,0,0 /* should be "SEEK_SET" */) != 0)
  {
   fprintf(stderr,"%s: unable to seek to start of file\n",whoami) ;
   exit(3) ;
  }

 if (((imagelen & (sizeof(int) - 1)) != 0) && !alignsource)
  {
   fprintf(stderr,"%s: infile image length is not word aligned\n",whoami) ;
   exit(4) ;
  }

#ifdef DEBUG
 printf("Image size = &%08X\n",imagelen) ;
#endif /* DEBUG */

 if (alignsource)
  {
   alignpadding = sizeof(int) - (imagelen & (sizeof(int) - 1)) ;
   if (alignpadding == sizeof(int))
    alignpadding = 0 ;
  }

#ifdef DEBUG
 printf("Image size = &%08X (after word-aligning)\n",imagelen) ;
#endif /* DEBUG */

 /* calculate the header size */
 excess = (hdrsize + strlen(iname) + 1) ;
 item.OBJECTOffset = ((excess + (sizeof(int) - 1)) & ~(sizeof(int) - 1)) ;
 excess = item.OBJECTOffset - excess ;
 item.OBJECTOffset += sizeof(ROMITEMstructure) ;
 item.ITEMLength = item.OBJECTOffset + imagelen + alignpadding ;
 item.OBJECTLength = imagelen ;
 item.ITEMNameLength = (strlen(iname) + 1 + excess) ;

#ifdef DEBUG
 printf("ItemLength = &%08X\n",item.ITEMLength) ;
#endif /* DEBUG */

 /* write the header to the outfile (echoing to the user) */
 if (fwrite((char *)&item,hdrsize,1,outf) != 1)
  {
   fprintf(stderr,"%s: unable to write ITEM header to image\n",whoami) ;
   exit(5) ;
  }
 /* write the name (and padding NULLs) to the image */
#ifdef DEBUG
 printf("Image name : \"%s\"\n",iname) ;
#endif /* DEBUG */
 if (fwrite(&(iname[0]),(strlen(iname) + 1),1,outf) != 1)
  {
   fprintf(stderr,"%s: unable to write item name to image\n",whoami) ;
   exit(5) ;
  }
 if (excess != sizeof(int))
  {
   cval = 0x00 ;
   for (loop = 0; (loop < excess); loop++)
    if (fwrite(&cval,sizeof(char),1,outf) != 1)
     {
      fprintf(stderr,"%s: unable to write name padding to image\n",whoami) ;
      exit(5) ;
     }
  }

 /* write the ROM specific header */
 if (fwrite((char *)&romitem,sizeof(ROMITEMstructure),1,outf) != 1)
  {
   fprintf(stderr,"%s: unable to write ROM ITEM header to image\n",whoami) ;
   exit(5) ;
  }
  
 /* copy the data to the outfile */
 while (fread(&cval,sizeof(char),1,inf) == 1)
  if (fwrite(&cval,sizeof(char),1,outf) != 1)
   {
    fprintf(stderr,"%s: unable to write data to image\n",whoami) ;
    exit(5) ;
   }

 cval = 0x00 ;	/* NULL padding character */
 while (alignpadding != 0)
  {
   if (fwrite(&cval,sizeof(char),1,outf) != 1)
    {
     fprintf(stderr,"%s: unable to write data to image\n",whoami) ;
     exit(5) ;
    }
   alignpadding-- ;
  }

 /* and exit cleanly */
 fclose(inf) ;
 fclose(outf) ;
 exit(0) ;
 SccsId = SccsId ; /* remove warning */
}

/*--------------------------------------------------------------------------*/
/*> EOF bromitem.c <*/
