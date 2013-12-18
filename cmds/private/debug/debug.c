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

#include <stdio.h>
#include <fcntl.h>
#include <setjmp.h>
#include <ctype.h>
#include <helios.h>
#include <queue.h>
#include <stdarg.h>
#include <nonansi.h>
#include <attrib.h>

#include "debug.h"
#include "dbdecode.h"


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

char *iname = NULL;
WORD *ibuf = NULL;
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

#define probe_value 0xaaaaaaaa

WORD bootsize;
UBYTE *bootstrap;

WORD deftrace = MinInt;

WORD dumpdata[26];

WORD isnum(...);
WORD readnumber(void);
WORD rdch(void);
WORD Cconis(void);

bool debug_local = TRUE;

#define swap(x) (x)
#define eqs(s,t) (strcmp(s,t)==0)
#define min(x,y) (x<y?x:y)

#define Cconout(c) { char ch = c; write(1,&ch,1); }

/****************************************************************/
/* main                                                         */
/*                                                              */
/****************************************************************/

int main(argc,argv)
int argc;
char **argv;
{
	word linkno = -1;
	
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

#ifdef T414
        defsym("MemStart",         MinInt+0x48);
        defsym("LoadBase",         MinInt+0x800);
        defsym("MemTop",           MinInt+0x40000);
#endif
#ifdef T800
        defsym("MemStart",         MinInt+0x70);
        defsym("RamBase",         MinInt+0x1000);
#endif

	for( argv++; *argv; )
	{
		char *arg = *argv++;
		if( *arg = '-' )
		{
			arg++;
			switch( *arg++ )
			{
			case 'l': 
				if( *arg==0 ) arg = *argv++;
				linkno = atoi(arg);
				break;
				
			case '2':
				addressbase = 0x8000;
				break;
			}
		}
		else iname = arg;
	}

	if( iname == NULL )
	{
		ibuf = (word *)0x80001000;
		isize = *ibuf;
		defsyms();
	}
	else
	{
		loadimage(iname);
	}
	
        report("\nTransputer Debugger V1.2");
        report("Copyright (C)1987-1990, Perihelion Software Ltd.");

	ansiopen();

        if( setjmp(&quit_lev) == 0 ) 
        {
        	if( linkno != -1 )
        	{
        		report("debugging through link %d",linkno);
			debug_local = FALSE;
        		loadboot();
        		xpinit(linkno,addressbase==0x8000);
        	}
        	else report("debugging locally");

        	debug();
        	
        }

       	xptidy();

	ansiclose();
	
        printf("\nDebug finished\n");
        
       	return 0;
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
        struct module *m = (struct module *)(modlist.Head);
        while( m->node.Next != NULL )
        {
                struct proc *p = (struct proc *)(m->procsyms.Head);
                while( p->node.Next != NULL )
                {
                        struct symb *s = (struct symb *)(p->codesyms.Head);
                        if( eqs(p->name,&token) )
                        {
                                printf("%s %x\n",p->name,p->value);
                                tokval = p->value;
                                return TRUE;
                        }
                        while( s->node.Next != NULL )
                        {
                                if( eqs(s->name,&token) )
                                {
                                        printf("%s %x\n",s->name,s->value);
                                        tokval = s->value;
                                        return TRUE;
                                }
                                s = (struct symb *)(s->node.Next);
                        }
                        p = (struct proc *)(p->node.Next);
                }
                m = (struct module *)(m->node.Next);
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
    "symbols",
    "settrace",
    "xp",
    "clear",
    "info",
    "dump",
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
        addressbase += current;
        break;

    case 1:
        loadimage("/helios/lib/nucleus");
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

    case 10:
	putsyms();
	break;

    case 11:
	deftrace = addressbase+current;
	break;

    case 12:
	setxp();
	break;

    case 13:
	clear();
	break;

    case 14:
	showdump();
	break;

    case 15:
	dump();
	break;
    }
    return TRUE;
}

/****************************************************************/
/* xprun                                                        */
/*                                                              */
/* Run a program in the transputer                              */
/* Note that this is ST dependant                               */
/****************************************************************/

xprun()
{

	report("\nSending bootstrap...");

	xpwrbyte(bootsize);		/* bootstrap size */
        xpwrdata(bootstrap,bootsize);	/* bootstrap */

	xpwrbyte(4);

	report("\nSending System Image...");
	xpwrdata(ibuf,isize);		/* system image */

	report("\nSending Configuration...");
	loadconf();
	
	runhelios();

        xpanalyse();
        report("\n...Analysed");
	dump();
}

#define ctrl(x) ((x)-'a'+1)

runhelios()
{
	WORD iocport = 0;
	WORD uch = -1;
	WORD scport = 0;
	for(;;)
	{

		if( Cconis() || uch > 0 )
		{
			WORD v;
			BYTE ch = uch;

			if( ch == -1 ) ch = rdch() & 0xff;
			else uch = -1;

			switch ( ch )
			{
			case ESC: return 0;

			case ctrl('i'):
				xpwrint (0xf0f0f0f0);
				xpwrword(0x00010100);
				xpwrint (0x8000AAAA);
				printf("Info sent\n");
				break;

			case ctrl('r'):
				v=dbrdword(MemStart);
				printf("Read value = %8lx\n",v);
				break;

			case ctrl('w'):
				dbwrword(MemStart,probe_value);
				printf("Probe value written\n");
				break;

#if 0
			case ctrl('m'):		/* send a test message */
				xpwrbyte(2);
				xpwrint (0x00010008);
				xpwrint (iocport);
				xpwrint (0x00000000);
				xpwrint (0xAAAAAAAA);
				xpwrword(0xCCCCCCCC);
				xpwrword(0xDDDDDDDD);
				xpwrword(0xDDDDDDDD);
				break;

#endif
			default:
				if( scport != 0 )
				{
					xpwrbyte(2);
					xpwrint(1);
					xpwrint(scport);
					xpwrint(0);
					xpwrint(0);
					xpwrbyte(ch);
					scport = 0;
				}
				else uch = ch;
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

			case 2:
			{
				WORD i,h,d,r,f;
				WORD csize, dsize;
				h = xprdint();
				d = xprdint();
				r = xprdint();
				f = xprdint();


				/* special single character message */
				if( (h&0xffff)==1 && f == 0x22222222 )
				{
					char c = xprdbyte();
					putchar(c);
					break;
				}

				if( f == 0x44444444 )
				{
					scport = r;
					break;
				}

				if( f == 0x60002010 )
				{
					printf("Search: %08x %08x %08x %08x\n",
						h,d,r,f);
					xprdint();
					xprdint();
					if( (dsize=(h & 0x0000ffff)) != 0 )
					{
						printf("For : ");
						for( i = 0 ; i < dsize-1 ; i++ )
							printf("%c",xprdbyte());
						xprdbyte();
						putchar('\n');
					}
					xpwrbyte(2);
					xpwrint (0x00000000);
					xpwrint (r);
					xpwrint (0x8000BBBB);
					xpwrint (0x00000000);
					break;
				}

				printf("Message       : %08x %08x %08x %08x\n",
					h,d,r,f);
				if( (csize=((h & 0x00ff0000)>>16)) != 0 )
				{
					printf("Control vector: ");
					for( i = 0 ; i < csize ; i++ )
						printf("%08x ",xprdint());
					putchar('\n');
				}
				if( (dsize=(h & 0x0000ffff)) != 0 )
				{
					printf("Data vector   : ");
					for( i = 0 ; i < dsize ; i++ )
						printf("%02x ",xprdbyte());
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
				printf("INFO : %08x %08x\n",a,iocport);
				if( ( a & 0x0000ff00 ) != 0 )
				{
					xpwrint (0xf0f0f0f0);
					xpwrword(0x00010000);
					xpwrint (0x8000AAAA);
					printf("Info sent\n");
				}
				break;

			default:
				printf("Unexpected %x",b);
				break;
			}
		}
	}
}

setxp()
{
	while( rdch() == ' ' );

	while ( ('0' <= ch) && (ch <= '3') )
	{
		xpwrbyte(bootsize);		/* bootstrap size */
	        xpwrdata(bootstrap,bootsize);	/* bootstrap */

		xpwrbyte(ch - '0');

		rdch();
	}
}

clear()
{
	report("\nSending bootstrap...");

	xpwrbyte(bootsize);		/* bootstrap size */
        xpwrdata(bootstrap,bootsize);	/* bootstrap */

	xpwrbyte(5);

	xpwrint(addressbase+current);

	xprdbyte();

	report("\nDone");

	xpanalyse();
}

WORD savearea[100];

dump()
{
	int i;

	if( debug_local ) 
	{
		word *nbootinfo = (word *)(0x80000188);
		for( i = 0 ; i < 26 ; i++ ) dumpdata[i] = nbootinfo[i];
		return;
	}
#if 0
	report("\nSaving low memory...");

	for(i = 0 ; i < 100 ; i++ ) savearea[i] = dbrdword(0x80000000+(i<<2));
#endif
	xpanalyse();
	
	report("\nSending bootstrap...");

	xpwrbyte(bootsize);		/* bootstrap size */
        xpwrdata(bootstrap,bootsize);	/* bootstrap */

IOdebug("sending command");
	xpwrbyte(6);
IOdebug("reading data");
	for( i = 0 ; i < 26 ; i++ ) dumpdata[i] = xprdint();
IOdebug("data read");
	xpanalyse();
#if 0
	report("\nRestoring low memory...");

	for(i = 0 ; i < 100 ; i++ ) dbwrword(0x80000000+(i<<2),savearea[i]);
#endif
}

showdump()
{
report("Iptr         : %8x",dumpdata[0]);
report("Wptr         : %8x",dumpdata[1]);
report("BootLink     : %8x",dumpdata[2]);
report("%s",dumpdata[3]?"Analysed":"Reset");
report("Output links : %8x %8x %8x %8x",dumpdata[4],dumpdata[5],dumpdata[6],dumpdata[7]);
report("Input  links : %8x %8x %8x %8x",dumpdata[8],dumpdata[9],dumpdata[10],dumpdata[11]);
report("Event channel: %8x",dumpdata[12]);
report("Timer Queues : hi %8x lo %8x",dumpdata[13],dumpdata[14]);
report("Save Area    : W %8x I %8x A %8x B %8x C %8x\n               S %8x E %8x",
	dumpdata[15],dumpdata[16],dumpdata[17],dumpdata[18],dumpdata[19],
	dumpdata[20],dumpdata[21]);
report("Hi Pri Queue : head %8x tail %8x",dumpdata[22],dumpdata[23]);
report("Lo Pri Queue : head %8x tail %8x",dumpdata[24],dumpdata[25]);
}

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
    if( !xpwrrdy() ) { warn("Transputer not ready"); return 0; }
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


warn(str,...)
char *str;
{
   va_list a;
   va_start(a,str);
   printf("\nWarning : ");
   vprintf(str,a);
   putchar('\n');
}

report(str,...)
char *str;
{
   va_list a;
   va_start(a,str);
   vprintf(str,a);
   putchar('\n');
}

error(str,...)
char *str;
{
   va_list a;
   va_start(a,str);
   printf("\nError : ");
   vprintf(str,a);
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

int pendch = -1;

WORD rdch()
{
	if( pendch != -1 )
	{
		ch = pendch;
		pendch = -1;
	}
	else
	{
		ch = 0;
		read(0,&ch,1);
	}
	return ch;
}

WORD Cconis( void )
{
	int c,e;

	if( pendch != -1 ) return TRUE;

	c = 0;
	e = Read(fdstream(0),&c,1,0);
	if ( e == 1 ) pendch = c;
	return (e==1);
}

/****************************************************************/
/* sysload                                                      */
/*                                                              */
/* Load a system as described in file SYSTEM                    */
/****************************************************************/

loadimage(fname)
char *fname;
{
        infd = open(fname,O_RDONLY);

        if( infd <= 0 ) { warn("Cannot open '%s' for input",fname); goto done1; }

        if( read(infd,&isize,4) != 4) {
                warn("Cannot read image header");
                goto done;
        }

        isize = swap(isize);

        report("System size = %d bytes",isize);

        ibuf = malloc(isize);

        if( ibuf == NULL ) {
                warn("Cannot get image buffer");
                goto done;
        }

        if( read(infd,ibuf+1,isize-4) != isize-4 ) {
                warn("Image too small");
                free(ibuf); ibuf = NULL;
                goto done;
        }

	ibuf[0] = swap(isize); /* place size back in image */

        defsyms();

    done:
        close( infd );
    done1:
        infd = 0;
}

loadconf()
{
	xpwrint(9*4);		/* 6 words in conf vector */
	xpwrint(64);		/* size of port table */
	xpwrint(1);		/* incarnation number */
	xpwrint(LoadBase);
	xpwrint(isize);
	xpwrint(4);		/* all 4 links */

	xpwrint(0x00000240);	/* link conf structures */
	xpwrint(0x01000000);
	xpwrint(0x02000000);
	xpwrint(0x03000000);
}

loadboot()
{
	WORD ihdr[3];
        infd = open("/helios/lib/nboot.i",O_RDONLY);

        if( infd <= 0 ) { warn("Cannot open 'nboot.i' for input"); goto done1; }

        if( read(infd,&ihdr,12) != 12) {
                warn("Cannot read boot header");
                goto done;
        }

        if( swap(ihdr[0]) != 0x12345678 )
        {
                warn("First word of file not magic number");
                goto done;
        }

        bootsize = swap(ihdr[2]);

        report("Boot size = %d bytes",bootsize);

        bootstrap = malloc(bootsize);

        if( bootstrap == NULL ) {
                warn("Cannot get image buffer");
                goto done;
        }

        if( read(infd,bootstrap,bootsize) != bootsize ) {
                warn("Image too small");
        }

    done:
        close( infd );
    done1:
        infd = 0;
	
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

#if 0
WORD swap(x)
WORD x;
{
        WORD r = 0;
        r |= ((x>>24)&0xff)<< 0;
        r |= ((x>>16)&0xff)<< 8;
        r |= ((x>> 8)&0xff)<<16;
        r |= ((x>> 0)&0xff)<<24;
}
#endif

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

WORD dbvec;
WORD dbpos;

#define TVSIZE 4096

WORD nextint()
{
	word r;
	if( dbpos >= TVSIZE ) dbpos = 4;
	r = dbrdint(dbvec+dbpos);
	dbpos+=4;
	return r;
}

trace()
{
        WORD todo;
        WORD i;

	if( deftrace != MinInt ) dbvec = deftrace;
	else {
		dbvec = dbrdint(0x80001000L);
		dbvec = dbrdint(dbvec+0x80001000L+0x54);
	}

	todo = dbrdint(dbvec) & 0xfffffffc;
	
	dbpos = todo & (TVSIZE-4);
	
	if( todo < TVSIZE ) dbpos = 4;
	else todo = TVSIZE+4;
	
	todo /= 4;
	todo--;

        for ( i = 0; i < todo; i++ )
        {
        	word t,w,ii,a,b,c;
                switch( t=nextint() )
                {
                case 0x11111111:
                	t = nextint();
                	w = nextint();
                	ii = nextint();
                        printf("Regs: T= %08x W= %08x I= %08x ",t,w,ii);
                        codeopd(0, ii );
                	a = nextint();                        
                	b = nextint();                        
                	c = nextint();                        
                        printf("\n      A= %08x B= %08x C= %08x\n",a,b,c);
			i += 6;
                        break;

                case 0x22222222:
                	t = nextint();
                	w = nextint();
                	ii = nextint();
                        printf("Mark: T= %08x W= %08x I= %08x ",t,w,ii);
                        codeopd(0, ii );
                        putchar('\n');
                        i += 3;
                        break;

                default:
                        printf("????: %08x\n",dbrdint(t) );
                        break;
                }

		if( Cconis() )
		{
			rdch();
			if( ch == ESC || ch == 0x03 ) return;
		}
        }
}

putsyms()
{
        struct module *m = (struct module *)(modlist.Head);
        while( m->node.Next != NULL )
        {
                struct proc *p = (struct proc *)(m->procsyms.Head);
                printf("Module:\t%s %x %x\n",m->name,m->base,m->size);
                while( p->node.Next != NULL )
                {
                        struct symb *s = (struct symb *)(p->codesyms.Head);
                        printf("Proc:\t\t%s %x\n",p->name,p->value);
                        while( s->node.Next != NULL )
                        {
                                printf("Code:\t\t\t%s %x\n",s->name,s->value);
                                s = (struct symb *)(s->node.Next);
                        }
                        p = (struct proc *)(p->node.Next);
                }
                m = (struct module *)(m->node.Next);
        }
}




Attributes ostate, nstate;

ansiopen()
{
	GetAttributes(Heliosno(stdin),&ostate);
	nstate = ostate;
	AddAttribute(&nstate,ConsoleRawInput);
/*	AddAttribute(&nstate, ConsoleRawOutput); */
/*	RemoveAttribute(&nstate, ConsolePause); */
	RemoveAttribute(&nstate, ConsoleIgnoreBreak);
	RemoveAttribute(&nstate, ConsoleBreakInterrupt);
/*	RemoveAttribute(&nstate, ConsoleEcho);*/
	AddAttribute(&nstate, ConsoleEcho);
	SetAttributes(Heliosno(stdin),&nstate);
	
}

ansiclose()
{
	SetAttributes(Heliosno(stdin),&ostate);
}

/*  -- End of debug.c -- */
