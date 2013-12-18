/*------------------------------------------------------------------------
--									--
--		   Real Time Power System Simulator			--
--		   ================================			--
--									--
--		Copyright (C) 1989, University Of Bath.			--
--			All Rights Reserved.				--
--									--
--	msgbind.c							--
--									--
--	MicroSoft Graph Library Binding					--
--									--
--	Author:  K.W. Chan						--
--									--
--	Started: 12 May 89						--
--									--
------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "graph.h"

/***************						**************/
/***************     Start of Helios-PC Connection Section	**************/
/***************						**************/

#include <stdlib.h>
#include <nonansi.h>

static	MCB	txmcb;
static  MCB     rxmcb;
static	Port	Reply;
static	Stream	*pc;
static	struct	videoconfig vcfg;
static	char	cfgknown;

#define	CSIZE		128
#define	DEFAULTDSIZE	2048
#define MAXDSIZE	20480

#define usearg(x) x=x

static Semaphore TxRxReq, Mutex, asyncRxLock;

int	dsize=DEFAULTDSIZE;
static	int	DataSize;

static	word	TxControl[CSIZE];
static	byte   *Data;
static	bool	SetUpDummyMsg = FALSE;
static  bool    RxStarted = FALSE;

static	void	DoDummyMsg( void );
static int OpenMSG( void );

typedef union DataUnion {
	long			Long;
	struct	 videoconfig	Videoconfig;
	struct	 xycoord	Xycoord;
	struct	 rccoord	Rccoord;
} DataUnion;

static void RxMsg( void )
{
	int res;

	if ((res = GetMsg(&rxmcb)) != 0x12345678)
	{
		fprintf(stderr,"MsgFromPC fail :- %x (0x12345678 expected)\n",res);
		return;
	}
	if( rxmcb.MsgHdr.ContSize != 0 ) 
	{
		fprintf(stderr,"MsgFromPC fail :- 0 length control vector expected\n",res);
		return;
	}
	
	Signal( &TxRxReq );
}

static void asyncRxMsg( void )
{
	while(1)
	{
		Wait( &asyncRxLock );
		RxMsg();
	}
}

static void SendToPC( int type )
{
	int res;

	Wait( &TxRxReq );

	TxControl[txmcb.MsgHdr.ContSize++] = -1;
#if 0
	fprintf(stderr,"Control size = %d\n",txmcb.MsgHdr.ContSize);
	fprintf(stderr,"Data size = %d\n",txmcb.MsgHdr.DataSize);
#endif
	if ((res = PutMsg(&txmcb)) < 0)
	{
		fprintf(stderr,"MsgToPC fail :- %x\n",res);
		return;
	}

	if (type == WAIT) RxMsg();
	else Signal(&asyncRxLock);

	txmcb.MsgHdr.DataSize = txmcb.MsgHdr.ContSize = 0;
}

static void CallPC( int fncode, int type, char *args, int argsize, 
			void *resultbuffer)
{
	if (pc == Null(Stream)) OpenMSG();

	Wait( &Mutex );

	if ( (txmcb.MsgHdr.DataSize + argsize >= DataSize ) ||
	     (txmcb.MsgHdr.ContSize >= CSIZE - 2)	 ||
	     dsize != DataSize			           )
	  SendToPC( PASS );

	if( dsize != DataSize )
	{
		free(Data);
		Data = malloc(dsize);
		DataSize = dsize;
	}

	TxControl[txmcb.MsgHdr.ContSize++] = fncode | (txmcb.MsgHdr.DataSize << 16);
	if (argsize > 0)
	{
		memcpy( &Data[txmcb.MsgHdr.DataSize], args, argsize);
		txmcb.MsgHdr.DataSize += argsize;
	}
	
	if (type == WAIT)
	{	rxmcb.Data = resultbuffer;
		SendToPC( WAIT );
	}
	Signal( &Mutex );
}

static void flush2( void )
{
	if (txmcb.MsgHdr.ContSize > 0) SendToPC( WAIT );
	else
		Wait(&TxRxReq);	/* Wait for any pending reply */
	FreePort( Reply );
	Close( pc );
}

void FlushMSG( int type )
{
	if (txmcb.MsgHdr.ContSize > 0) SendToPC( type );
}

static int OpenMSG( void )
{
	Object *obj;

	if (pc != NULL) return TRUE;

	if ((obj = Locate(NULL,"/pc")) == Null(Object))
	{
		fprintf(stderr,"Unable to locate /pc\n");
		fflush(stderr);
		return FALSE;
	}
	if ((pc = Open(obj,"/pc",O_ReadWrite)) == NULL)
	{
		fprintf(stderr,"Unable to open /pc\n");
		fflush(stderr);
		return FALSE;
	}

	Reply = NewPort();

	InitSemaphore( &TxRxReq, 1 );
	InitSemaphore( &Mutex, 1 );
	InitSemaphore( &asyncRxLock, 0 );

	InitMCB(&txmcb, MsgHdr_Flags_preserve,pc->Server,Reply,0x20765432);
	txmcb.Timeout = 5 * OneSec;
	txmcb.Control = TxControl;
	Data          = malloc(dsize);
	DataSize      = dsize;
	
	if( Data == NULL )
	{
		fprintf(stderr,"Failed to allocate graphics communication buffer\n");
		return FALSE;
	}
	txmcb.Data    = Data;

	InitMCB(&rxmcb, MsgHdr_Flags_preserve,Reply,NULL,0);
	rxmcb.Timeout = 5 * OneSec;
	rxmcb.Control = NULL;
	
	Close(obj);

	if (SetUpDummyMsg == FALSE)
	{
		Fork( 500, DoDummyMsg, 0);
		SetUpDummyMsg = TRUE;
	}
	
	if( RxStarted == FALSE )
	{
		Fork(500,asyncRxMsg, 0);
	}

	cfgknown = 0;

	atexit( flush2 );

	return TRUE;
}

/***************						**************/
/***************    Start of MicroSoft Graph Binding Section	**************/
/***************						**************/

#define fn_setvideomode		0
#define fn_setactivepage	1
#define fn_setvisualpage	2
#define fn_getvideoconfig	3
#define fn_setlogorg		4
#define fn_getlogcoord		5
#define fn_getphyscoord		6
#define fn_setcliprgn		7
#define fn_setviewport		8
#define fn_clearscreen		9
#define fn_moveto		10
#define fn_getcurrentposition	11
#define fn_lineto		12
#define fn_rectangle		13
#define fn_ellipse		14
#define fn_arc			15
#define fn_pie			16
#define fn_setpixel		17
#define fn_getpixel		18
#define fn_floodfill		19
#define fn_setcolor		20
#define fn_getcolor		21
#define fn_setlinestyle		22
#define fn_getlinestyle		23
#define fn_setfillmask		24
#define fn_getfillmask		25
#define fn_setbkcolor		26
#define fn_getbkcolor		27
#define fn_remappalette		28
#define fn_remapallpalette	29
#define fn_selectpalette	30
#define fn_settextwindow	31
#define fn_outtext		32
#define fn_wrapon		33
#define fn_displaycursor	34
#define fn_settextposition	35
#define fn_gettextposition	36
#define fn_settextcolor		37
#define fn_gettextcolor		38
#define fn_send_dummy_msg	39
#define fn_imagesize		40
#define fn_getimage		41
#define fn_putimage1		42
#define fn_putimage2		43

/* SETUP AND CONFIGURATION */

int _setvideomode(int a)
{	short res;
	CallPC( fn_setvideomode, WAIT, (char *)&a, sizeof(int), &res);
	cfgknown = 0;
	return (int)res;
}

void _setactivepage(int a)
{
	CallPC( fn_setactivepage, PASS, (char *)&a, sizeof(int), NULL);
}

void _setvisualpage(int a)
{
	CallPC( fn_setvisualpage, PASS, (char *)&a, sizeof(int), NULL);
}

struct videoconfig * _getvideoconfig(struct videoconfig *a)
{
	CallPC(fn_getvideoconfig,WAIT, NULL, 0, a);
	memcpy(&vcfg,a,sizeof(struct videoconfig));
	cfgknown = 1;
	return a;
}

/* COORDINATE SYSTEMS */

struct xycoord _setlogorg(int a, int b)
{	struct xycoord res;
	usearg(b);
	CallPC( fn_setlogorg, WAIT, (char *)&a, sizeof(int) * 2, &res);
	return res;
}

struct xycoord _getlogcoord(int a, int b)
{	struct xycoord res;
	usearg(b);
	CallPC( fn_getlogcoord, WAIT, (char *)&a, sizeof(int) * 2, &res);
	return res;
}

struct xycoord _getphyscoord(int a, int b)
{	struct xycoord res;
	usearg(b);
	CallPC( fn_getphyscoord, WAIT, (char *)&a, sizeof(int) * 2, &res);
	return res;
}

void _setcliprgn(int a, int b, int c, int d)
{
	usearg(b); usearg(c); usearg(d);
	CallPC( fn_setcliprgn, PASS, (char *)&a, sizeof(int) * 4, NULL);
}

void _setviewport(int a, int b, int c, int d)
{
	usearg(b); usearg(c); usearg(d);
	CallPC( fn_setviewport, PASS, (char *)&a, sizeof(int) * 4, NULL);
}

/* OUTPUT ROUTINES */

void _clearscreen(int a)
{
	CallPC( fn_clearscreen, PASS, (char *)&a, sizeof(int), NULL);
}

void _moveto(int a, int b)
{
	usearg(b);
	CallPC( fn_moveto, PASS, (char *)&a, sizeof(int) * 2, NULL);
}

struct xycoord _getcurrentposition(void)
{	struct xycoord res;

	CallPC( fn_getcurrentposition, WAIT, NULL,  sizeof(int) * 2, &res);
	return res;
}

void _lineto(int a, int b)
{
	usearg(b);
	CallPC( fn_lineto, PASS, (char *)&a, sizeof(int) * 2, NULL);
}

int _rectangle(int a, int b, int c, int d, int e)
{	short res;
	usearg(b); usearg(c); usearg(d); usearg(e);
	CallPC( fn_rectangle, WAIT, (char *)&a, sizeof(int) * 5, &res);
	return (int)res;
}

int _ellipse(int a, int b, int c, int d, int e)
{	short res;
	usearg(b); usearg(c); usearg(d); usearg(e);
	CallPC( fn_ellipse, WAIT, (char *)&a, sizeof(int) * 5, &res);
	return (int)res;
}

void _arc(int a, int b, int c, int d, int e, int f, int g, int h)
{
	usearg(b); usearg(c); usearg(d); usearg(e);
	usearg(f); usearg(g); usearg(h);
	CallPC( fn_arc, PASS, (char *)&a, sizeof(int) * 8, NULL);
}

int _pie(int a, int b, int c, int d, int e, int f, int g, int h, int i)
{	int res;
	usearg(b); usearg(c); usearg(d); usearg(e);
	usearg(f); usearg(g); usearg(h); usearg(i);
	CallPC( fn_pie, WAIT, (char *)&a, sizeof(int) * 9, &res);
	return (int)res;
}

void _setpixel(int a, int b)
{
	usearg(b);
	CallPC( fn_setpixel, PASS, (char *)&a, sizeof(int) * 2, NULL);
}

int _getpixel(int a, int b)
{	short res;
	usearg(b);
	CallPC( fn_getpixel, WAIT, (char *)&a, sizeof(int) * 2, &res);
	return (int)res;
}

int _floodfill(int a, int b, int c)
{	short res;
	usearg(b); usearg(c);
	CallPC( fn_floodfill, WAIT, (char *)&a, sizeof(int) * 3, &res);
	return (int)res;
}

/* PEN COLOR, LINE STYLE, FILL PATTERN */

void _setcolor(int a)
{
	CallPC( fn_setcolor, PASS, (char *)&a, sizeof(int),NULL);
}

int _getcolor(void)
{	short res;
	CallPC( fn_getcolor, WAIT, NULL, 0, &res);
	return (int)res;
}

void _setlinestyle(unsigned int a)
{
	CallPC( fn_setlinestyle, PASS, (char *)&a, sizeof(int),NULL);
}

unsigned int _getlinestyle(void)
{	unsigned short res;
	CallPC( fn_getlinestyle, WAIT, NULL, 0, &res);
	return (unsigned int)res;
}

void _setfillmask(unsigned char *a)
{
	CallPC( fn_setfillmask, WAIT, (char *)a, sizeof(char) * 8, NULL);
}

unsigned char * _getfillmask(unsigned char *a)
{
	CallPC( fn_getfillmask, WAIT, NULL, 0, a);
	return a;
}

/* COLOR SELECTION */

int _setbkcolor(int a)
{	long res;
	CallPC( fn_setbkcolor, WAIT, (char *)&a, sizeof(int), &res);
	return (int)res;
}

int _getbkcolor(void)
{	long res;
	CallPC( fn_getbkcolor, WAIT, NULL, 0, &res);
	return (int)res;
}

int _remappalette(int a, int b)
{	long res;
	usearg(b);
	CallPC( fn_remappalette, WAIT, (char *)&a, sizeof(int) * 2, &res);
	return (int)res;
}

int _remapallpalette(long *a)
{	short res;
	CallPC( fn_remapallpalette, WAIT, (char *)a, sizeof(long) * 16, &res);
	return (int)res;
}

int _selectpalette(int a)
{	short res;
	CallPC( fn_selectpalette, WAIT, (char *)&a, sizeof(int), &res);
	return (int)res;
}

/* TEXT */

void _settextwindow(int a, int b, int c, int d)
{
	usearg(b); usearg(c); usearg(d);
	CallPC( fn_settextwindow, WAIT, (char *)&a, sizeof(int) * 4, NULL);
}

void _outtext(char *a)
{
	extern int strlen( char * );
	CallPC( fn_outtext, PASS, (char *)a, strlen(a) + 1, NULL);
}

int _wrapon(int a)
{	short res;
	CallPC( fn_wrapon, WAIT, (char *)&a, sizeof(int), &res);
	return (int)res;
}

int _displaycursor(int a)
{	short res;
	CallPC( fn_displaycursor, WAIT, (char *)&a, sizeof(int), &res);
	return (int)res;
}

void _settextposition(int a, int b)
{
	usearg(b);
	CallPC( fn_settextposition, PASS, (char *)&a, sizeof(int) * 2, NULL);
}

struct rccoord _gettextposition(void)
{	struct rccoord res;
	CallPC( fn_gettextposition, WAIT, NULL, 0, &res);
	return res;
}

void _settextcolor(int a)
{
	CallPC( fn_settextcolor, PASS, (char *)&a, sizeof(int), NULL);
}

int _gettextcolor(void)
{	short res;
	CallPC( fn_gettextcolor, WAIT, NULL, 0, &res);
	return (int)res;
}

/* SCREEN IMAGES */
int _imagesize(int a, int b, int c, int d)
{	long res;
	usearg(b); usearg(c); usearg(d);
	CallPC( fn_imagesize, WAIT, (char *)&a, sizeof(int)*4, &res);
	return (int)res;
}

void _getimage(int a, int b, int c, int d, char *e)
{	usearg(b); usearg(c); usearg(d);
	CallPC( fn_getimage, WAIT, (char *)&a, sizeof(int)*4, e);
}

void _putimage(int x, int y, char *e, int mode)
{	short *thisrow;
	int bitsperrow;
	int bytesperrow;
	int nrows;
	int rowsperbuff;
	int v[3];
	int planes;
	
	if( !cfgknown )
		_getvideoconfig(&vcfg);

	switch( vcfg.mode )
	{
	case _MRES4COLOR: case _MRESNOCOLOR: case _HRESBW:
	case _VRES2COLOR: case _MRES256COLOR:
		planes = 1;
		break;
	case _ERESNOCOLOR:
		planes = 2;
		break;
	case _ERESCOLOR: case _MRES16COLOR: case _HRES16COLOR:
	case _VRES16COLOR:
		planes = 4;
		break;
	default:
		return;
	}

	thisrow= (short *)e;
	bitsperrow = thisrow[0];
	bytesperrow = (bitsperrow+7)/8*planes;
	nrows = thisrow[1];
/* The 4 in the next line is to account for the bitsperrow & nrows	*
 *  that go on the front of each request				*/
	rowsperbuff = (dsize-4)/bytesperrow;

#ifdef DEBUG
	fprintf(stderr,"size of image = %d\n",bytesperrow*nrows+4);
#endif
	v[0] = x;
	v[2] = mode;

	while(  nrows )
	{	int thisrows = nrows>rowsperbuff ? rowsperbuff: nrows;
		int thisxfersize = thisrows*bytesperrow;
		int j1,j2;

		v[1] = y;
		j1 = thisrow[1];
		j2 = thisrow[0];
		thisrow[1] = thisrows;
		thisrow[0] = bitsperrow;

		CallPC( fn_putimage1, PASS, (char *)v, sizeof(int)*3, NULL);
		CallPC( fn_putimage2, PASS, (char *)thisrow,thisxfersize+4, NULL);

		thisrow[1] = j1;
		thisrow[0] = j2;
		y += thisrows;
		thisrow = (short*)((char *)thisrow+thisxfersize);
		nrows -= thisrows;
	}
}

static void DoDummyMsg( void )
{
	forever
	{
		Delay( 60 * OneSec );

		CallPC( fn_send_dummy_msg, WAIT, NULL, 0,NULL);
	}
}

/***************						**************/
/***************     End of MicroSoft Graph Binding Section	**************/
/***************						**************/

/***  end of msgbind.c  ***/
