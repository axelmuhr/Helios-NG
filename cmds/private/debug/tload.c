#include <osbind.h>
#include <stdio.h>
#include <ttypes.h>
#include <debug.h>

#define ESC	0x1B
#define probe_value 0x61616161L

WORD bootsize;
UBYTE *bootstrap;
WORD *ibuf;
WORD isize;

FILE *infd;

WORD *lmalloc();
WORD swap();

WORD xprdint();
WORD xprdword();
WORD xprdbyte();
WORD dbrdword();
WORD dbrdint();

main()
{
	tload("boot.i","addon");
}

report(str,a,b,c,d,e,f)
STRING str;
WORD a,b,c,d,e,f;
{
	printf("Report: ");
	printf(str,a,b,c,d,e,f);
	putchar('\n');
}

tload(bootfile,sysfile)
STRING bootfile,sysfile;
{
	infd = NULL;
	loadboot(bootfile);
	loadimage(sysfile);
	xprun();
}

loadimage(fname)
STRING fname;
{
        infd = fopen(fname,"rb");

        if( infd == NULL ) { report("Cannot open '%s' for input",&fname); goto done1; }

        if( fread(&isize,1,4,infd) != 4) {
                report("Cannot read image header");
                goto done;
        }

        isize = swap(isize);

        report("System size = %ld bytes",isize);

        ibuf = lmalloc(isize);

        if( ibuf == NULL ) {
                report("Cannot get image buffer");
                goto done;
        }

        if( fread(ibuf+1,1,(int)(isize-4),infd) != isize-4 ) {
                report("Image too small");
        }

	ibuf[0] = swap(isize); /* place size back in image */

    done:
        fclose( infd );
    done1:
        infd = NULL;
}

loadconf()
{
	xpwrint(24L);		/* 6 words in conf vector */
	xpwrint(64L);		/* size of port table */
	xpwrint(1L);		/* incarnation number */
	xpwrint(LoadBase);
	xpwrint(isize);
	xpwrint(1L);		/* just 1 link for now */
	xpwrword(0x40020000L);	/* link conf structure */
}

loadboot(bootfile)
STRING bootfile;
{
	int s;
	WORD ihdr[3];

        infd = fopen(bootfile,"rb");

        if( infd == NULL ) { report("Cannot open '%s' for input",bootfile); goto done1; }

        if( (s=fread(ihdr,1,12,infd)) != 12) {
                report("Cannot read boot header %d %d",s,ferror(infd));
                goto done;
        }

        if( swap(ihdr[0]) != 0x12345678L )
        {
                report("First word of file not magic number");
                goto done;
        }

        bootsize = swap(ihdr[2]);

        report("Boot size = %ld bytes",bootsize);

        bootstrap = lmalloc(bootsize);

        if( bootstrap == NULL ) {
                report("Cannot get image buffer");
                goto done;
        }

        if( (s=fread(bootstrap,1,(int)bootsize,infd)) != bootsize ) {
                report("Image too small %d %d",s,ferror(infd));
        }

    done:
        fclose( infd );
    done1:
        infd = NULL;
	
}

WORD swap(x)
WORD x;
{
        WORD r = 0;
        r |= ((x>>24)&0xff)<< 0;
        r |= ((x>>16)&0xff)<< 8;
        r |= ((x>> 8)&0xff)<<16;
        r |= ((x>> 0)&0xff)<<24;
	return r;
}

xprun()
{
	xpreset();

        report("Sending Bootstrap...");

	xpwrbyte(bootsize);		/* bootstrap size */
        xpwrdata(bootstrap,bootsize);	/* bootstrap */

	report("Sending System Image...");
	xpwrdata(ibuf,isize);		/* system image */

	report("Sending Configuration...");
	loadconf();
	
	helios();

        xpanalyse();
        report("...Analysed");
}

helios()
{
	WORD iocport = 0;
	for(;;)
	{
		if( Cconis() )
		{
			WORD v;
			UBYTE ch = Cconin() &0xff;
			switch ( ch )
			{
			case ESC: return;

			case 'i':
				xpwrint (0xf0f0f0f0L);
				xpwrword(0x00010100L);
				xpwrint (0x8000AAAAL);
				printf("Info sent\n");
				break;

			case 'r':
				v=dbrdword(MemStart);
				printf("Read value = %8lx\n",v);
				break;

			case 'w':
				dbwrword(MemStart,probe_value);
				printf("Probe value written\n");
				break;

			case 'm':		/* send a test message */
				xpwrbyte(2);
				xpwrint (0x00010008L);
				xpwrint (iocport);
				xpwrint (0x00000000L);
				xpwrint (0xAAAAAAAAL);
				xpwrword(0xCCCCCCCCL);
				xpwrword(0xDDDDDDDDL);
				xpwrword(0xDDDDDDDDL);
				break;
			}
		}
		if( xprdrdy() )
		{
			UBYTE b = xprdbyte();
			WORD a, v;
			switch( b )
			{
			case 0:			/* write (part of probe */
				a=xprdint();
				v=xprdint();
				printf("WRITE: %08lx %08lx\n",a,v);
				break;

			case 1:			/* read command	*/
				a=xprdint();	/* address */
				xpwrword(~probe_value); /* inverted result */
				printf("READ : %08lx\n",a);
				break;

			case 2:
			{
				WORD i,h,d,r,f;
				WORD csize, dsize;
				h = xprdint();
				d = xprdint();
				r = xprdint();
				f = xprdint();

				printf("Message       : %08lx %08lx %08lx %08lx\n",h,d,r,f);
				if( (csize=((h & 0x00ff0000L)>>16)) != 0 )
				{
					printf("Control vector: ");
					for( i = 0 ; i < csize ; i++ )
						printf("%08lx ",xprdint());
					putchar('\n');
				}
				if( (dsize=(h & 0x0000ffffL)) != 0 )
				{
					printf("Data vector   : ");
					for( i = 0 ; i < dsize ; i++ )
						printf("%02lx ",xprdbyte());
					putchar('\n');
				}
				break;
			}

			case 0xf0:		/* start of info */
				xprdbyte();
				xprdbyte();
				xprdbyte();	/* rest of sync word */
				a=xprdword();
				iocport=xprdint();
				printf("INFO : %08lx %08lx\n",a,iocport);
				if( ( a & 0x0000ff00L ) != 0 )
				{
					xpwrint (0xf0f0f0f0L);
					xpwrword(0x00010000L);
					xpwrint (0x8000AAAAL);
					printf("Info sent\n");
				}
				break;
			}
		}
	}
}
