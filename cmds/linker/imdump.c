/*------------------------------------------------------------------------
--                                                                      --
--				IMDUMP.C				--
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987 - 1993, Perihelion Software Ltd.      --
--                        All Rights Reserved.                          --
--                                                                      --
--	Displays human readable form of Helios image file		--
--									--
--	Author:  PAB 25/1/89						--
--	Upgrades by NJOC 27/8/93					--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: imdump.c,v 1.14 1993/12/17 12:48:14 nickc Exp $ Copyright (C) 1987, Perihelion Software Ltd.\n */
  
#ifndef __STDC__
#define void int
#define const
unsigned char *malloc();
#endif

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <stdio.h>
#include <helios.h>	/* standard header */
#include <module.h>
#include <stdlib.h>
#include <string.h>

typedef struct Image_Magics
  {
    word	ImageMagic;
    bool	invert_swap;
    bool	smtopt;
  }
Image_Magics;

int	hexopt = FALSE;

#ifdef HOSTISBIGENDIAN
int	swapopt = TRUE;	/* default to swapping */
#else
int	swapopt = FALSE;
#endif

#ifdef __SMT
word	smtopt = TRUE;
#else
word	smtopt = FALSE;
#endif

void usage()
{
	fprintf(stderr,"Usage: imdump [-h] <imagefile>\n");
	exit(1);
}

word
swapword( word it )
{
	if (!swapopt)	return (it);
	{
		byte	from[4];
		byte	to[4];

		*((word *)from) = it;

		to[0] = from[3];
		to[1] = from[2];
		to[2] = from[1];
		to[3] = from[0];

		return(*((word *)to));
	}
}


/*--------------------------------------------------------
-- LoadImage						--
--							--
-- Read an image file into memory. 			--
--							--
--------------------------------------------------------*/

unsigned char *
LoadImage(
	  ImageHdr *	Hdr,
	  char	*	name )
{
	unsigned char *	image;
	FILE *		fp;
	word		imsize;

#ifdef __STDC__
	if( (fp = fopen(name, "rb")) == NULL)
#else
	if( (fp = fopen(name, "r")) == NULL)
#endif
	{
		fprintf(stderr,"imdump: file not found\n");
		exit(1);
	}

	if (fread((char *)Hdr,1,sizeof(ImageHdr),fp) != sizeof(ImageHdr))
	{
		fprintf(stderr,"imdump: Cannot read image file header\n");
		exit(1);
	}

	  {
	    static Image_Magics Magics[] =
	      {
		/*
		 * XXX - the following have been extracted from /hsrc/include/module.h
		 *       make sure that they are kept up to date
		 */

		{ 0x12345678L, FALSE,   FALSE },	/* Transputer */
		{ 0xc4045601L, FALSE,   TRUE  },	/* TMS320C40  */
		{ 0x0a245601L, FALSE,   TRUE  },	/* ARM	      */
		{ 0x86045601L, FALSE,   TRUE  },	/* i860	      */
#ifdef HOSTISBIGENDIAN
		{ 0x01560468L, TRUE, 	FALSE },	/* 68K (on a big-endian host) */
#endif
		{ 0x68045601L, FALSE,   FALSE }		/* 68K	      */
	      };
	    word	Magic = swapword( Hdr->Magic );
	    int		i;

	    
	    for (i = sizeof (Magics) / sizeof (Magics[ 0 ] );
		 i--;)
	      {
		if (Magic == Magics[ i ].ImageMagic)
		  {
		    smtopt = Magics[ i ].smtopt;
		    
		    if (Magics[ i ].invert_swap)
		      swapopt = !swapopt;
		    
		    break;
		  }
	      }	    

	    if (i < 0)
	      {
		if (Magic == 0xC3CBC6C5)
		  {
		    fprintf( stderr, "imdump: file is an ARM Object Format file!\n" );
		  }
		else if (Magic == 0xC5C6CBC3)
		  {
		    fprintf( stderr, "imdump: file is a byte swapped ARM Object Format file!\n" );
		  }
		else if ( (Magic & 0x00FFFFFF) == 0x00014120)
		  {
		    fprintf( stderr, "imdump: file is an Helios Object Format file!\n" );
		  }
		else
		  {
		    fprintf(stderr,"imdump: file is not an image file\n");
		    fprintf(stderr, "Magic %lx is not a Helios Magic number\n", Magic);
		  }
		fclose(fp);
		exit(1);
	      }
	  }
	
	imsize = swapword(Hdr->Size);
	
	if((image = (unsigned char *)malloc((int)imsize)) == NULL )
	{
		fprintf(stderr,"imdump: out of mem\n");
		fclose(fp);
		exit(1);
	}

	if( fread(image,1,(int)imsize,fp) != imsize) 
	{
		free(image);
		fprintf(stderr,"imdump: size of image file is wrong\n");
		fclose(fp);
		exit(1);
	}
	fclose(fp);
	return image;
}


void
DisplayHex(
	   ImageHdr *		Hdr,
	   unsigned char *	image )
{
	int i,offs = 0;
	word imsize = swapword(Hdr->Size);
#	define NOPERLINE 16

	printf("HEX DISPLAY OF IMAGE HEADER:\n");
	printf("Magic = %lx\n", swapword(Hdr->Magic));
	printf("Flags = %lx\n", swapword(Hdr->Flags));
	printf("Size  = %lx\n", imsize);

	printf("\nHEX DISPLAY OF IMAGE BODY:\n");
	for(;;)
	{
		printf("%04x: ",offs);
		for (i=0; i < NOPERLINE ; i++)
		{
			if (imsize--)
				printf(" %02.2x", *image++);
			else
			{
				putchar('\n');
				exit(0);
			}
		}
		putchar('\n');
		offs += NOPERLINE;
	}
}

void
DisplayHdr( ImageHdr * Hdr )
{
	printf("IMAGE HEADER:\n");
	printf("Magic = %lx\n", swapword(Hdr->Magic));
	printf("Flags = %lx\n", swapword(Hdr->Flags));
	printf("Size  = %#lx (%ld)\n", swapword(Hdr->Size),swapword(Hdr->Size));
}

void
DisplayMods(
	    ImageHdr *		Hdr,
	    unsigned char *	image )
{
	Module *mod = (Module *)image;
	word	swap, type, size, imsize;

	printf("\nIMAGE BODY:\n");
	imsize = swapword(Hdr->Size);

	while ((unsigned char *)mod < image + imsize)
	{
	putchar('\n');
	type = swapword(mod->Type);

	switch(type)
		{
		case T_Program:
			printf("Type      = Program\n");
			goto bypass;
		case T_Module:
			printf("Type      = Code Module\n");
		bypass:
			size = swapword(mod->Size);
			printf("Size      = %#lx (%ld)\n",size,size);

			printf("Name      = \"%s\"\n",mod->Name);

			swap = swapword(mod->Id);
			printf("Slot      = %#lx (%ld)\n",swap,swap);

			swap = swapword(mod->Version);
			printf("Version   = %#lx (%ld)\n",swap,swap);

			swap = swapword(mod->MaxData);
			printf("MaxData   = %#lx (%ld)\n",swap,swap);

			swap = swapword(mod->Init);
			printf("Init RPTR = %#lx (%ld)\n",swap,swap);

			if (smtopt) {
				/* Cheat slightly, so we can have one version */
				/* of imdump even if the Module struct has no */
				/* MaxCodeP field. */
				swap = swapword(*(((word *)(&mod->Init)) + 1));
				printf("MaxCodeP  = %#lx (%ld)\n",swap,swap);
			}

			if(type == T_Module)
				break;

			swap = swapword(((Program *)mod)->Stacksize);
			printf("Stacksize = %#lx (%ld)\n",swap,swap);

			swap = swapword(((Program *)mod)->Heapsize);
			printf("Heapsize  = %#lx (%ld)\n",swap,swap);

			swap = swapword(((Program *)mod)->Main);
			printf("Main RPTR = %#lx (%ld)\n",swap,swap);
			break;

		case T_ResRef:
			printf("Type      = ResRef\n");

			size = swapword(((ResRef *)mod)->Size);
			printf("Size      = %#lx (%ld)\n",size,size);

			printf("Require   = \"%s\"\n",((ResRef *)mod)->Name);

			swap = swapword(((ResRef *)mod)->Id);
			printf("Slot      = %#lx (%ld)\n",swap,swap);

			swap = swapword(((ResRef *)mod)->Version);
			printf("Version   = %#lx (%ld)\n",swap,swap);

			break;

 	        case T_Device:
			/* This relies on the "Module" structure and
			 * "Device" structure identity for the most part.
			 * (Since <device.h> has too many Helios definitions
			 * for a HOSTed version of this program)
			 */
                        printf("Type      = Device\n") ;

			size = swapword(mod->Size);
			printf("Size      = %#lx (%ld)\n",size,size) ;

			printf("Name      = \"%s\"\n",mod->Name) ;

			swap = swapword(mod->Version);
	                printf("Version   = %#lx (%ld)\n",swap,swap) ;

                	swap = swapword(mod->MaxData);
        	        printf("DevOpen   = %#lx (%ld)\n",swap,swap) ;
	                break ;

		case 0L:
			/* module 0 = last module */
			return;

		default:
			fprintf(stderr, "WARNING: Unknown module type %#lx - skipping\n",type);
			size = mod->Size; /* a definite maybe */
 			if (size <= 0) size = 4;
			break;
		}

		mod = (Module *)((char *)mod + size);
	}
	fprintf(stderr, "imdump exit - no last module indication\n");
}


int
main(
     int 	argc,
     char **	argv )
{
	ImageHdr Hdr;
	unsigned char	*image;
	char	*imname = NULL;

	switch (argc)
	{
	case 2:
		imname = argv[1];
		break;
	case 3:
		if (strcmp(argv[1], "-h") == 0)
			hexopt = TRUE;
		else if (strcmp(argv[1], "-nsmt") == 0)
			smtopt = FALSE;
		else if (strcmp(argv[1], "-smt") == 0)
			smtopt = TRUE;
		else	usage();
		imname = argv[2];
		break;
	case 4:
		if (strcmp(argv[1], "-h") == 0)
			hexopt = TRUE;
		else if (strcmp(argv[1], "-nsmt") == 0)
			smtopt = FALSE;
		else if (strcmp(argv[1], "-smt") == 0)
			smtopt = TRUE;
		else	usage();

		if (strcmp(argv[2], "-h") == 0)
			hexopt = TRUE;
		else if (strcmp(argv[2], "-nsmt") == 0)
			smtopt = FALSE;
		else	usage();

		imname = argv[3];
		break;
	default:
		usage();
	}

	if ((image = LoadImage(&Hdr,imname)) != NULL)
	{
		if(hexopt)
			DisplayHex(&Hdr, image);
		else
		{
			DisplayHdr(&Hdr);
			DisplayMods(&Hdr,image);
		}
		free(image);
	}
}


/* -- End of imdump.c */
