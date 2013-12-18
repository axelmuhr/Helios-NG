/************************************************************************/
/*                                                                      */
/* File: debug.c                                                        */
/*                                                                      */
/*      Kernel debugger - a quick-and-dirty program to get things going */
/* and to test out some ideas. This will never be a product.            */
/*                                                                      */
/* Changes:                                                             */
/*      NHG  25-May-87  : Created                                       */
/*                                                                      */
/*                                                                      */
/* Copyright (c) 1987, Perihelion Software Ltd. All Rights Reserved.    */
/************************************************************************/

#include <osbind.h>
#include <stdio.h>
#include <fcntl.h>
#include <setjmp.h>
#include <ctype.h>
#include <ttypes.h>
#include <queue.h>
#include <debug.h>
#include <dbdecode.h>

#define New(_type) ((_type *)malloc(sizeof(_type)))

WORD current;               /* current value of interest */
WORD framesize;             /* size of display frame     */
WORD addressbase;           /* offset from which all addresses are caluclated */

WORD ch;                    /* current input character   */

UBYTE framebuf[270];        /* buffer for frame (256 + some slush)   */
UBYTE *frame;               /* pointer into start of frame in buffer */

FILE  *outfd;
WORD infd = 0;

BYTE token[128];
WORD toksize;
WORD tokval;

WORD curpos;
WORD modbase;
WORD codelim;
WORD dmode = 0;

WORD *ibuf;
WORD isize;
WORD ihdr[3];

jmp_buf def_lev;
jmp_buf *err_lev;
jmp_buf quit_lev;

extern struct List modlist;

#define issize 32
#define ismask (issize-1)
WORD istack[issize];
WORD ispos = 0;

#define BS     0x08
#define DEL    0x7f
#define ESC    0x1b

#define probe_value 0x61616161

/* minimal bootstrap */
#ifdef T414
UBYTE bootstrap[] = { 0x03, 0x27, 0x2b, 0x05 };
#endif
#ifdef T800
UBYTE bootstrap[] = { 0x03, 0x27, 0x2b, 0x05 };
#endif

/****************************************************************/
/* main                                                         */
/*                                                              */
/****************************************************************/

main()
{
        addressbase = MinInt;
        framesize = 16;
        current = 0;

        infd = 0;
        outfd = stdout;

        initsym();

        defsym("MemBase",          MinInt);

        defsym("Link0Out",         MinInt+0x00);
        defsym("Link1Out",         MinInt+0x04);
        defsym("Link2Out",         MinInt+0x08);
        defsym("Link3Out",         MinInt+0x0c);

        defsym("Link0In",          MinInt+0x10);
        defsym("Link1In",          MinInt+0x14);
        defsym("Link2In",          MinInt+0x18);
        defsym("Link3In",          MinInt+0x1c);

        defsym("Event",            MinInt+0x20);

        defsym("TPtrLoc0",         MinInt+0x24);
        defsym("TPtrLoc1",         MinInt+0x28);

        defsym("WdescIntSaveLoc",  MinInt+0x2c);
        defsym("IptrIntSaveLoc",   MinInt+0x30);
        defsym("AregIntSaveLoc",   MinInt+0x34);
        defsym("BregIntSaveLoc",   MinInt+0x38);
        defsym("CregIntSaveLoc",   MinInt+0x3c);
        defsym("STATUSIntSaveLoc", MinInt+0x40);
        defsym("EregIntSaveLoc",   MinInt+0x44);


        defsym("MemStart",         MemStart);
        defsym("LoadBase",         LoadBase);
        defsym("MemTop",           MinInt+0x40000);


        report("\nTransputer Debugger V1.01");
        report("Copyright (C)1987, Perihelion Software Ltd.");

        if( setjmp(&quit_lev) == 0 ) debug();

        printf("\nDebug finished\n");
}

defsym(name,value)
STRING name;
WORD value;
{
        struct module *m = nearsym(&modlist,value);
        struct proc *p = nearsym(&m->procsyms,value);
        newsym(&p->codesyms,name,value);
}


/****************************************************************/
/* debug                                                        */
/*                                                              */
/* Debugger main loop                                           */
/*                                                              */
/****************************************************************/

debug()
{
    WORD newval;

    ch = '0';

    if( setjmp(&def_lev) == 0 )
    {
        err_lev = &def_lev;
        toksize = 0;
    }

    for(;;)
    {
        rdch();
        ch = tolower(ch);
        if( '0' <= ch && ch <= '9' ||
            'a' <= ch && ch <= 'z' ||
	    ch == '_' ||
	    (ch == '.' && toksize > 0 ) )
        {
            token[toksize++] = ch;
            continue;
        }

        /* some simple line editing */

        if( ch == BS && toksize>0 ) {
            toksize--;
            Cconout(' ');
            Cconout(BS);
            continue;
         }
        if( ch == DEL && toksize>0 ) {
            Cconout('\r');
            while( toksize )
            { Cconout(' '); toksize--; }
            Cconout('\r');
            continue;
        }


        /* when we drop through we have a non-alpha char */

        if( toksize != 0 )
        {
            token[toksize] = '\0';
            if( issym() ) current = tokval-addressbase;
            else if( isnum() ) current = tokval;
            else if( !builtin() ) warn("Unknown symbol '%s'",&token);
            toksize = 0;
        }

        switch( ch )
        {
        default:
            /* warn("Unexpected character %02x",ch); */
        case BS:
        case DEL:
            break;

        case '\r':                     /* re-display on end of line       */
        show:
            showframe();
            break;

        case ':':                       /* set frame size                 */
            newval = readnumber();
            if( newval > 256 ) warn("Maximum frame size is #100 bytes");
            else framesize = newval;
            goto show;

        case '.':                       /* advance by one frame           */
            current += framesize;
            goto show;

        case ',':                       /* back by one frame              */
            current -= framesize;
            goto show;

        case '>':                       /* forward to next word boundary  */
            current = (current+4) & (~3);
            goto show;

        case '<':                       /* back to prev word boundary     */
            current = (current-1) & (~3);
            goto show;

        case '+':                       /* forward n bytes                */
            current += readnumber();
            goto show;

        case '-':                       /* backward n bytes               */
            current -= readnumber();
            goto show;

        case '=':                       /* alter contents of word         */
            if( (current+addressbase)&3 ) error("Not at word boundary");
            newval = dbrdint(addressbase+current);
            printf("\r%8x: %8x = ",current,newval); fflush(stdout);
            newval = readnumber();
            dbwrint(addressbase+current,newval);
            goto show;

        case '[':                       /* indirect                       */
            if( (current+addressbase)&3 ) error("Not at word boundary");
            istack[ispos++] = current;
            current = dbrdint(addressbase+current)-addressbase;
            ispos &= ismask;
            goto show;

        case ']':                       /* exdirect                       */
            ispos = (ispos-1) & ismask;
            current = istack[ispos];
            goto show;

        case '{':                       /* indirect RPTR                  */
            if( (current+addressbase)&3 ) error("Not at word boundary");
            istack[ispos++] = current;
            current += dbrdint(addressbase+current);
            ispos &= ismask;
            goto show;

        case '\'':                      /* advance by one frame & disasm  */
            current += framesize;

        case ';':
            dasm();
            break;

        }
    }
}

/****************************************************************/
/* issym                                                        */
/* isnum                                                        */
/* builtin                                                      */
/*                                                              */
/* further parts of the command interpreter                     */
/****************************************************************/

WORD isnum()
{
    WORD n = 0;
    int i;
    for( i = 0 ; token[i] ; i++ )
    {
        BYTE c = token[i];
        if('0' <= c && c <= '9' ) n = (n<<4) + c - '0';
        else if('a' <= c && c <= 'f' ) n = (n<<4) + c - 'a' + 10;
        else return FALSE;
    }
    tokval = n;
    return TRUE;
}

issym()
{
        struct module *m = (struct module *)(modlist.head);
        while( m->node.next != NULL )
        {
                struct proc *p = (struct proc *)(m->procsyms.head);
                while( p->node.next != NULL )
                {
                        struct symb *s = (struct symb *)(p->codesyms.head);
                        if( eqs(p->name,&token) )
                        {
                                printf("%s %x\n",p->name,p->value);
                                tokval = p->value;
                                return TRUE;
                        }
                        while( s->node.next != NULL )
                        {
                                if( eqs(s->name,&token) )
                                {
                                        printf("%s %x\n",s->name,s->value);
                                        tokval = s->value;
                                        return TRUE;
                                }
                                s = (struct symb *)(s->node.next);
                        }
                        p = (struct proc *)(p->node.next);
                }
                m = (struct module *)(m->node.next);
        }
        return FALSE;
}

STRING fntab[] = {
    "base",
    "load",
    "reset",
    "analyse",
    "quit",
    "go",
    "trace",
    "bytes",
    "words",
    "cmp",
    0
};

builtin()
{
    WORD fn;

    for( fn = 0 ; fntab[fn] != 0 ; fn++ )
            if( eqs(&token,fntab[fn]) ) break;

    if( fntab[fn] == 0 ) return FALSE;

    switch( fn )
    {
    case 0:
        addressbase = current;
        break;

    case 1:
        loadimage();
        break;

    case 2:
        xpreset();
        report("Reset");
        break;

    case 3:
        xpanalyse();
        report("Analysed");
        break;

    case 4:
        printf("\nExit to system - sure? "); fflush(stdout);
        rdch();
        if( tolower(ch) == 'y' ) longjmp(&quit_lev,1);
        putchar('\n');
        break;

    case 5:
        xprun();
        break;

    case 6:
        trace();
        break;

    case 7:
        dmode = 0;
        break;

    case 8:
        dmode = 1;
        break;

    case 9:
	docmp();
	break;

    }
    return TRUE;
}

WORD eqs(s,t)
STRING s, t;
{
   int i;
   for( i = 0 ; s[i] && t[i] && i <= 31; i++ )
                if( tolower(s[i]) != tolower(t[i]) ) return FALSE;
   return tolower(s[i])==tolower(t[i]);
}

/****************************************************************/
/* xprun                                                        */
/*                                                              */
/* Run a program in the transputer                              */
/* Note that this is ST dependant                               */
/****************************************************************/

xprun()
{
        report("\nBooting...");

        xpwrdata(bootstrap,bootstrap[0]+1);     /* boot it */

        for(;;)
        {
                while ( Cconis() >= 0 )
                {
                        if( xprdrdy() ) 
				report("Byte from link: %02x",xprdbyte());
                }
                ch = ( Cconin() & 0xff ) ;
                if( ch == ESC ) break;
                if( xpwrrdy() ) xpwrbyte(ch);
        }

        xpanalyse();
        report("\n...Analysed");
}

#if 0
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
				xpwrint (0xf0f0f0f0);
				xpwrword(0x00010100);
				xpwrint (0x8000AAAA);
				printf("Info sent\n");
				break;

			case 'r':
				v=dbrdword(0x80000048);
				printf("Read value = %8lx\n",v);
				break;

			case 'w':
				dbwrword(0x80000048,probe_value);
				printf("Probe value written\n");
				break;

			case 'm':		/* send a test message */
				xpwrbyte(2);
				xpwrint (0x00010008);
				xpwrint (iocport);
				xpwrint (0x00000000);
				xpwrint (0xAAAAAAAA);
				xpwrword(0xCCCCCCCC);
				xpwrword(0xDDDDDDDD);
				xpwrword(0xDDDDDDDD);
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
				printf("WRITE: %08x %08x\n",a,v);
				break;

			case 1:			/* read command	*/
				a=xprdint();	/* address */
				xpwrword(~probe_value); /* inverted result */
				printf("READ : %08x\n",a);
				break;

			case 0xf0:		/* start of info */
				xprdbyte();
				xprdbyte();
				xprdbyte();	/* rest of sync word */
				a=xprdword();
				iocport=xprdint();
				printf("INFO : %08x %08x\n",a,iocport);
				if( ( a & 0x0000ff00 ) != 0 )
				{
					xpwrint (0xf0f0f0f0);
					xpwrword(0x00010000);
					xpwrint (0x8000AAAA);
					printf("Info sent\n");
				}
				break;
			}
		}
	}	
}
#endif

/****************************************************************/
/* showframe                                                    */
/*                                                              */
/* display a frame of memory                                    */
/*                                                              */
/****************************************************************/

#define linesize 8

showframe()
{
    int linebase = 0;
    if( !xpwrrdy() ) { warn("Transputer not ready"); return; }
    getframe();
    putchar('\n');
    while( linebase < framesize )
    {
        int todo = min(linesize,framesize-linebase);
        int j;
        printf("%8x: ",addressbase+current+linebase);

        if( dmode == 0 ) {
                for( j = 0 ; j < todo ; j++ )
                    printf("%s%02x ",j%4?"":" ",frame[linebase+j]);
        }
        else {
                WORD w = 0;
                for( j = 0 ; j < 4 ; j++ ) w |= frame[linebase+j]<<(j*8);
                printf("%08x  ",w);
                w = 0;
                for( j = 4 ; j < linesize ; j++ ) w |= frame[linebase+j]<<((j-4)*8);
                printf("%08x  ",w);
        }

        for( ; j < linesize ; j++ ) printf("%s   ",j%4?"":" ");

        for( j = 0 ; j < todo ; j++ )
        {
            UBYTE b = frame[linebase+j];
            BYTE ctl = ' ', c = b;
            if( 0 <= b && b < ' ') { ctl = '^'; c = b+'@'; }
            if( b > '~' ) c = '.';
            printf("%c%c",ctl,c);
        }
        linebase += todo;
        putchar('\n');
    }
}

/****************************************************************/
/* getframe                                                     */
/*                                                              */
/* read current frame from transputer into frame buffer         */
/*                                                              */
/****************************************************************/

getframe()
{
    int i;
    WORD lwb = current&(~3);
    WORD upb = (current+framesize+3)&(~3);
    int fsize = (upb-lwb)>>2;
    WORD *f = (WORD *)&framebuf;


    for( i = 0 ; i < fsize ; i++ )
            f[i] = dbrdword(addressbase+lwb+(i<<2));

    frame = framebuf + (current&3);
}

/****************************************************************/
/* readnumber                                                   */
/*                                                              */
/* read a hex number from the input                             */
/*                                                              */
/****************************************************************/

WORD readnumber()
{
    WORD n = 0;

    while( rdch() == ' ');

    for(;;)
    {
        if( ch == DEL ) longjmp(err_lev,1);
        if( ch == BS ) {
                Cconout(' '); Cconout(BS);
                n = (n>>4) & 0x0fffffff;
        }
        else if('0' <= ch && ch <= '9' ) n = (n<<4) + ch - '0';
             else if('a' <= ch && ch <= 'f' ) n = (n<<4) + ch - 'a' + 10;
             else break;
        rdch();
        ch = tolower(ch);
    }
    return n;
}


warn(str,a,b,c,d,e,f)
{
   printf("\nWarning : ");
   printf(str,a,b,c,d,e,f);
   putchar('\n');
}

report(str,a,b,c,d,e,f)
{
   printf(str,a,b,c,d,e,f);
   putchar('\n');
}

error(str,a,b,c,d,e,f)
{
   printf("\nError : ");
   printf(str,a,b,c,d,e,f);
   putchar('\n');

   if( infd > 0 ) { close(infd); infd = 0; }

   longjmp(err_lev,1);
}

/****************************************************************/
/* rdch                                                         */
/*                                                              */
/* single character input                                       */
/* Since this code in never likely to be moved to anything other*/
/* than an ST I dont mind about using direct system calls       */
/****************************************************************/

WORD rdch()
{
        return ch = ( Cconin() & 0xff ) ;
}

/****************************************************************/
/* sysload                                                      */
/*                                                              */
/* Load a system as described in file SYSTEM                    */
/****************************************************************/

loadimage()
{
        BYTE fname[128];
        WORD i = 0;

        while( ch == ' ' ) rdch();

        if( ch == '\r' )
        {
                printf("\nImage file: "); fflush(stdout);
                while( rdch() == ' ' );
        }

	for(;;) {
        	/* some simple line editing */

        	if( ch == BS && i>0 ) {
			i--;
                        Cconout(' ');
                        Cconout(BS);
                        rdch(); continue;
                }
                if( ch == DEL && i>0 ) {
            		while( i )
            		{ Cconout(' '); Cconout(BS); i--; }
            		rdch(); continue;
        	}

		if( ch == '\r' ) break;
		fname[i++] = ch;
		rdch();
	}

        fname[i] = '\0';

        infd = open(&fname,O_RDONLY|O_RAW);

        if( infd <= 0 ) { warn("Cannot open '%s' for input",&fname); goto done1; }

        putchar('\n');

        if( read(infd,&ihdr,12) != 12) {
                warn("Cannot read image header");
                goto done;
        }

        if( swap(ihdr[0]) != 0x12345678 )
        {
                warn("First word of file not magic number");
                goto done;
        }

        isize = swap(ihdr[2]);

        report("Image size = %d bytes",isize);

        ibuf = malloc(isize);

        if( ibuf == NULL ) {
                warn("Cannot get image buffer");
                goto done;
        }

        if( read(infd,ibuf,isize) != isize ) {
                warn("Image too small");
      /*          goto done; */
        }

        for( i = 0 ; i < isize ; i+=4 ) dbwrword(LoadBase+i,ibuf[i>>2]);

        defsyms();

	loadconf();

    done:
        close( infd );
    done1:
        infd = 0;
}

loadconf()
{
	WORD conf = MinInt + 0x100;

	dbwrint(conf,64);		/* size of port table */
	dbwrint(conf+4,1);		/* incarnation number */
	dbwrint(conf+8,LoadBase);
	dbwrint(conf+12,isize);
	dbwrint(conf+16,1);		/* just 1 link for now */
	dbwrword(conf+20,0x40020000);	/* link conf structure */
}

docmp()
{
	INT i;
	for( i = 0 ; i < isize ; i+=4 )
	{
		WORD xpw, bfw;
		if( (xpw=dbrdword(LoadBase+i)) != (bfw=ibuf[i>>2]) )
			report("%8x: %8x != %8x",LoadBase+i,xpw,bfw);
	}
	report("Comparison finished");
}


WORD swap(x)
WORD x;
{
        WORD r = 0;
        r |= ((x>>24)&0xff)<< 0;
        r |= ((x>>16)&0xff)<< 8;
        r |= ((x>> 8)&0xff)<<16;
        r |= ((x>> 0)&0xff)<<24;
}

/****************************************************************/
/* dasm                                                         */
/*                                                              */
/* Disassemble current frame                                    */
/****************************************************************/

WORD fpos;
jmp_buf end_lev;

dasm()
{

        framesize+=8;         /* make sure we complete the last instruction */
        getframe();
        framesize-=8;

        putchar('\n');

        fpos = 0;
        curpos = addressbase+current;

        if( setjmp(&end_lev) == 0 )
        {
                while( fpos < framesize ) dodisasm();
        }

}

UBYTE gbyte()
{
/*        if( fpos >= framesize ) longjmp(&end_lev,1); */
        curpos++;
        return frame[fpos++];
}

void ungbyte()
{
        curpos--;
        fpos--;
}

/****************************************************************/
/* trace                                                        */
/*                                                              */
/* Print trace vector                                           */
/****************************************************************/

#define dbvec 0x80030000

trace()
{
        WORD upb = dbrdint(dbvec);
        WORD i;

        for ( i = 4; i < upb; i+=4)
        {
                switch( dbrdint(dbvec+i) )
                {
                case 0x11111111:
                        printf("Regs: T= %08x W= %08x I= %08x ",
                                dbrdint(dbvec+i+4),
                                dbrdint(dbvec+i+8),
                                dbrdint(dbvec+i+12) );
                        codeopd(0, dbrdint(dbvec+i+12) );
                        printf("\n      A= %08x B= %08x C= %08x\n",
                                dbrdint(dbvec+i+16),
                                dbrdint(dbvec+i+20),
                                dbrdint(dbvec+i+24) );
                        i += 24;
                        break;

                case 0x22222222:
                        printf("Mark: T= %08x W= %08x I= %08x ",
                                dbrdint(dbvec+i+4),
                                dbrdint(dbvec+i+8),
                                dbrdint(dbvec+i+12) );
                        codeopd(0, dbrdint(dbvec+i+12) );
                        putchar('\n');
                        i += 12;
                        break;

                default:
                        printf("????: %08x\n",dbrdint(dbvec+i) );
                        break;
                }
                if( Cconis() < 0 ) {
                        WORD ch = Cconin() & 0xff;
                        if( ch == ESC ) break;
                }
        }

}

/*  -- End of debug.c -- */
