#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graph.h>
#include <dos.h>
#include <process.h>
#include <errno.h>
#include <direct.h>

#define EQUIP_FLAG ((char far *)(0x410L))
#define VIDEO_AREA ((char far *)(0x449L))
/* Align the elements in the next array exactly as shown */
#pragma pack(1)
struct video_area {
	char	crt_mode;
	short	crt_cols;
	short	crt_len;
	short	crt_start;
	short	cursor_posn[8];
	short	cursor_mode;
	char	active_page;
	short	addr_6845;
	char	crt_mode_set;
	char	crt_pallette;
};
/* Revert to default packing */
#pragma pack()

#ifdef TWO
	char col_prams[30];
	char mon_prams[30];
#endif

typedef long int word;

void far EN_I();
void far DIS_I();

typedef struct PSP {
	int int20;
	int segend;
	char mscall[6];
	char prevint22[4];
	char prevctrlc[4];
	char prevcrit[4];
	char resrvd1[22];
	int  envblk;
	char resrvd2[46];
	char fcb1[16];
	char fcb2[20];
	char comtail[1];
};

#define offsetof(s,f) ((int)(&((s *)0)->f))

typedef struct MsgHdr {
	short	DataSize;
	char	ContSize;
	char	Flags;
	word	Dest;
	word	Reply;
	word	FnRc;
} MsgHdr;

typedef struct MCB {
	MsgHdr	MsgHdr;
	word	Timeout;
	word far *Control;
	char far *Data;
} MCB;

typedef union DataVec {
	long			Long;
	short			Short;
	unsigned short		UShort;
	struct	videoconfig	Videoconfig;
	struct	xycoord		Xycoord;
	struct	rccoord		Rccoord;
	char			ca[256];
	long			la[64];
} DataVec;

static fn_setvideomode(void);
static fn_setactivepage(void);
static fn_setvisualpage(void);
static fn_getvideoconfig(void);
static fn_setlogorg(void);
static fn_getlogcoord(void);
static fn_getphyscoord(void);
static fn_setcliprgn(void);
static fn_setviewport(void);
static fn_clearscreen(void);
static fn_moveto(void);
static fn_getcurrentposition(void);
static fn_lineto(void);
static fn_rectangle(void);
static fn_ellipse(void);
static fn_arc(void);
static fn_pie(void);
static fn_setpixel(void);
static fn_getpixel(void);
static fn_floodfill(void);
static fn_setcolor(void);
static fn_getcolor(void);
static fn_setlinestyle(void);
static fn_getlinestyle(void);
static fn_setfillmask(void);
static fn_getfillmask(void);
static fn_setbkcolor(void);
static fn_getbkcolor(void);
static fn_remappalette(void);
static fn_remapallpalette(void);
static fn_selectpalette(void);
static fn_settextwindow(void);
static fn_outtext(void);
static fn_wrapon(void);
static fn_displaycursor(void);
static fn_settextposition(void);
static fn_gettextposition(void);
static fn_settextcolor(void);
static fn_gettextcolor(void);
static fn_send_dummy_msg(void);
static fn_imagesize(void);
static fn_getimage(void);
static fn_putimage1(void);
static fn_putimage2(void);

typedef void (*VoidFnPtr)(void);

static VoidFnPtr far Fn[] =
{
	fn_setvideomode,
	fn_setactivepage,
	fn_setvisualpage,
	fn_getvideoconfig,
	fn_setlogorg,
	fn_getlogcoord,
	fn_getphyscoord,
	fn_setcliprgn,
	fn_setviewport,
	fn_clearscreen,
	fn_moveto,
	fn_getcurrentposition,
	fn_lineto,
	fn_rectangle,
	fn_ellipse,
	fn_arc,
	fn_pie,
	fn_setpixel,
	fn_getpixel,
	fn_floodfill,
	fn_setcolor,
	fn_getcolor,
	fn_setlinestyle,
	fn_getlinestyle,
	fn_setfillmask,
	fn_getfillmask,
	fn_setbkcolor,
	fn_getbkcolor,
	fn_remappalette,
	fn_remapallpalette,
	fn_selectpalette,
	fn_settextwindow,
	fn_outtext,
	fn_wrapon,
	fn_displaycursor,
	fn_settextposition,
	fn_gettextposition,
	fn_settextcolor,
	fn_gettextcolor,
	fn_send_dummy_msg,
	fn_imagesize,
	fn_getimage,
	fn_putimage1,
	fn_putimage2
};

static MCB far *mcb;
static DataVec far *Data;
static DataVec far *ReturnData;
char far *lmemcpy(char far *s1, char far *s2, long n);

#ifdef TWO
void tocolor()
{
	char far *vm = EQUIP_FLAG;

	lmemcpy( mon_prams, VIDEO_AREA, (long)sizeof(struct video_area));
	*vm = (*vm & ~0x30) | 0x20;
	lmemcpy( VIDEO_AREA, col_prams, (long)sizeof(struct video_area));
}

void tomono()
{
	char far *vm = EQUIP_FLAG;

	lmemcpy( col_prams, VIDEO_AREA, (long)sizeof(struct video_area));
	*vm |= 0x30;
	lmemcpy( VIDEO_AREA, mon_prams, (long)sizeof(struct video_area));
}
#endif

void vwrch(char c)
{
	union REGS in,out;
	in.h.ah = 0xe;
	in.h.al = c;
	in.h.bh = 0;
	in.h.bl = 0;
	int86(0x10,&in,&out);
}

void vwrstr(char far *s)
{
	while(*s) vwrch(*s++);
}

void vwrnum1(long n)
{	char c;
	if(n==0) return;
	vwrnum1(n/16);
	switch((int)(n&0xf))
	{
	case 0:	c = '0'; break;
	case 1:	c = '1'; break;
	case 2: c = '2'; break;
    case 3: c = '3'; break;
	case 4: c = '4'; break;
	case 5: c = '5'; break;
	case 6: c = '6'; break;
	case 7: c = '7'; break;
	case 8: c = '8'; break;
	case 9: c = '9'; break;
	case 0xa: c = 'a'; break;
	case 0xb: c = 'b'; break;
	case 0xc: c = 'c'; break;
	case 0xd: c = 'd'; break;
	case 0xe: c = 'e'; break;
	case 0xf: c = 'f'; break;
	}
	vwrch(c);
}

void vwrnum(long n)
{
	if(n) vwrnum1(n);
	else vwrstr("0");
}

/* SETUP AND CONFIGURATION */

static fn_setvideomode(void)
{
	ReturnData->Short = _setvideomode( Data->la[0] );
	mcb->MsgHdr.DataSize = sizeof(short);
}

static fn_setactivepage(void)
{
	_setactivepage( Data->la[0] );
	mcb->MsgHdr.DataSize = 0;
}

static fn_setvisualpage(void)
{
	_setvisualpage( Data->la[0] );
	mcb->MsgHdr.DataSize = 0;
}
char mybuff[100];

static fn_getvideoconfig(void)
{	struct videoconfig far *x = (struct videoconfig far *)&Data->Videoconfig;
	_getvideoconfig( &ReturnData->Videoconfig );
	mcb->MsgHdr.DataSize = sizeof(struct videoconfig);
}

/* COORDINATE SYSTEMS */

static fn_setlogorg(void)
{
	ReturnData->Xycoord = _setlogorg( Data->la[0], Data->la[1] );
	mcb->MsgHdr.DataSize = sizeof(struct xycoord);
}

static fn_getlogcoord(void)
{
	ReturnData->Xycoord = _getlogcoord( Data->la[0], Data->la[1] );
	mcb->MsgHdr.DataSize = sizeof(struct xycoord);
}

static fn_getphyscoord(void)
{
	ReturnData->Xycoord = _getphyscoord( Data->la[0], Data->la[1] );
	mcb->MsgHdr.DataSize = sizeof(struct xycoord);
}

static fn_setcliprgn(void)
{
	_setcliprgn( Data->la[0], Data->la[1], Data->la[2], Data->la[3] );
	mcb->MsgHdr.DataSize = 0;
}

static fn_setviewport(void)
{
	_setviewport( Data->la[0], Data->la[1], Data->la[2], Data->la[3] );
	mcb->MsgHdr.DataSize = 0;
}

/* OUTPUT ROUTINES */

static fn_clearscreen(void)
{
	_clearscreen( Data->la[0] );
	mcb->MsgHdr.DataSize = 0;
}

static fn_moveto(void)
{
	_moveto( Data->la[0], Data->la[1] );
	mcb->MsgHdr.DataSize = 0;
}

static fn_getcurrentposition(void)
{
	ReturnData->Xycoord = _getcurrentposition();
	mcb->MsgHdr.DataSize = sizeof(struct xycoord);
}

static fn_lineto(void)
{
	_lineto( Data->la[0], Data->la[1] );
	mcb->MsgHdr.DataSize = 0;
}

static fn_rectangle(void)
{
	ReturnData->Short = _rectangle( Data->la[0], Data->la[1],
				Data->la[2], Data->la[3], Data->la[4] );
	mcb->MsgHdr.DataSize = sizeof(short);
}

static fn_ellipse(void)
{
	ReturnData->Short = _ellipse( Data->la[0], Data->la[1],
				Data->la[2], Data->la[3], Data->la[4] );
	mcb->MsgHdr.DataSize = sizeof(short);
}

static fn_arc(void)
{	
	_arc( Data->la[0], Data->la[1], Data->la[2], Data->la[3],
		Data->la[4], Data->la[5], Data->la[6], Data->la[7]);
	mcb->MsgHdr.DataSize = 0;
}

static fn_pie(void)
{	
	ReturnData->Short = _pie( Data->la[0], Data->la[1], Data->la[2],
				  Data->la[3], Data->la[4], Data->la[5],
				  Data->la[6], Data->la[7], Data->la[8]);
	mcb->MsgHdr.DataSize = sizeof(short);
}

static fn_setpixel(void)
{	
	_setpixel( Data->la[0], Data->la[1] );
	mcb->MsgHdr.DataSize = 0;
}

static fn_getpixel(void)
{	
	ReturnData->Short = _getpixel( Data->la[0], Data->la[1] );
	mcb->MsgHdr.DataSize = sizeof(short);
}

static fn_floodfill(void)
{	
	ReturnData->Short = _floodfill( Data->la[0], Data->la[1], Data->la[2] );
	mcb->MsgHdr.DataSize = sizeof(short);
}

/* PEN COLOR, LINE STYLE, FILL PATTERN */

static fn_setcolor(void)
{
	_setcolor( Data->la[0] );
	mcb->MsgHdr.DataSize = 0;
}

static fn_getcolor(void)
{
	ReturnData->Short = _getcolor();
	mcb->MsgHdr.DataSize = sizeof(short);
}

static fn_setlinestyle(void)
{
	_setlinestyle( (unsigned short)Data->la[0] );
	mcb->MsgHdr.DataSize = 0;
}

static fn_getlinestyle(void)
{
	ReturnData->UShort = _getlinestyle();
	mcb->MsgHdr.DataSize = sizeof(short);
}

static fn_setfillmask(void)
{
	_setfillmask( (unsigned char far *)&Data->ca[0] );
	mcb->MsgHdr.DataSize = 0;
}

static fn_getfillmask(void)
{
	_getfillmask( (unsigned char far *)&ReturnData->ca[0]);
	mcb->MsgHdr.DataSize = 8;
}

/* COLOR SELECTION */

static fn_setbkcolor(void)
{
	ReturnData->Long = _setbkcolor( Data->la[0] );
	mcb->MsgHdr.DataSize = sizeof(long);
}

static fn_getbkcolor(void)
{
	ReturnData->Long = _getbkcolor();
	mcb->MsgHdr.DataSize = sizeof(long);
}

static fn_remappalette(void)
{
	ReturnData->Long = _remappalette( Data->la[0], Data->la[1] );
	mcb->MsgHdr.DataSize = sizeof(long);
}

static fn_remapallpalette(void)
{
	ReturnData->Long = _remapallpalette( &Data->la[0] );
	mcb->MsgHdr.DataSize = sizeof(long);
}

static fn_selectpalette(void)
{
	ReturnData->Short = _selectpalette( Data->la[0] );
	mcb->MsgHdr.DataSize = sizeof(short);
}

/* TEXT */

static fn_settextwindow(void)
{
	_settextwindow( Data->la[0], Data->la[1], Data->la[2], Data->la[3] );
	mcb->MsgHdr.DataSize = 0;
}

static fn_outtext(void)
{
	_outtext( &Data->ca[0] );
	mcb->MsgHdr.DataSize = 0;
}

static fn_wrapon(void)
{
	ReturnData->Short = _wrapon( Data->la[0] );
	mcb->MsgHdr.DataSize = sizeof(short);
}

static fn_displaycursor(void)
{
	ReturnData->Short = _displaycursor( Data->la[0] );
	mcb->MsgHdr.DataSize = sizeof(short);
}

static fn_settextposition(void)
{
	_settextposition( Data->la[0], Data->la[1] );
	mcb->MsgHdr.DataSize = 0;
}

static fn_gettextposition(void)
{
	ReturnData->Rccoord = _gettextposition();
	mcb->MsgHdr.DataSize = sizeof(struct rccoord);
}

static fn_settextcolor(void)
{
	_settextcolor( Data->la[0] );
	mcb->MsgHdr.DataSize = 0;
}

static fn_gettextcolor(void)
{
	ReturnData->Short = _gettextcolor();
	mcb->MsgHdr.DataSize = sizeof(short);
}

#if 0

static fn_send_dummy_msg(void)
{
	static int i = 0;
	static char ttt[40];

	sprintf(ttt,"Dummy Msg %d", i++);

	_settextposition(10,10);
	_outtext(ttt);

	mcb->MsgHdr.DataSize = 0;
}

#else

static fn_send_dummy_msg(void)
{
	mcb->MsgHdr.DataSize = 0;
}

#endif
static fn_imagesize(void)
{
	ReturnData->Long = _imagesize(Data->la[0],Data->la[1],
					Data->la[2],Data->la[3]);
	mcb->MsgHdr.DataSize = sizeof(long);
}

static fn_getimage(void)
{	short ressize;
	short x1,y1,x2,y2;
	x1 = Data->la[0];
	y1 = Data->la[1];
	x2 = Data->la[2];
	y2 = Data->la[3];

	ressize = _imagesize(x1,y1,x2,y2);
	_getimage(x1,y1,x2,y2, (char far *)ReturnData);
	mcb->MsgHdr.DataSize = ressize;
}

static short x1,y1,mode;

static fn_putimage1(void)
{
	x1   = Data->la[0];
	y1   = Data->la[1];
	mode = Data->la[2];
#if 0
	vwrstr("pi1 x1 = "); vwrnum(x1); vwrstr("\r\n");
	vwrstr("pi1 y1 = "); vwrnum(y1); vwrstr("\r\n");
	vwrstr("pi1 mode = "); vwrnum(mode); vwrstr("\r\n");
#endif
	mcb->MsgHdr.DataSize = 0;
}

static fn_putimage2(void)
{
#if 0
	vwrstr("pi2 x1 = "); vwrnum(x1); vwrstr("\r\n");
	vwrstr("pi2 y1 = "); vwrnum(y1); vwrstr("\r\n");
	vwrstr("pi2 mode = "); vwrnum(mode); vwrstr("\r\n");
#endif
	_putimage(x1,y1,(char far *)Data,mode);
	mcb->MsgHdr.DataSize = 0;
}


#define	FnTabSize	(sizeof(Fn)/sizeof(VoidFnPtr))

char far *lmemcpy(char far *s1, char far *s2, long n)
{	char far *r = s1;
	while(n--) *s1++ = *s2++;
	return s1;
}


long int INTERRUPT_RTN( MCB far *a)
{
	char far *data;
	char s[40];
	int fncode;
	int far *cont;
	int i;
#ifdef TWO
	tocolor();
#endif
	mcb = a;
	i = mcb->MsgHdr.ContSize;

	cont = (int far *)mcb->Control;
	ReturnData = (DataVec far *)(data = mcb->Data);

	while (--i)
	{	fncode = *cont++;
		Data = (DataVec far *)&data[*cont++];
		Fn[fncode]();
	}

	mcb->MsgHdr.ContSize = 0;
#ifdef TWO
	tomono();
#endif
	return 0x12345678;
}

int main(int argc, char *argv[])
{
#ifdef TWO
	char far *vm = EQUIP_FLAG;

	_setvideomode(_ERESCOLOR);

	lmemcpy( (char far *)col_prams, VIDEO_AREA, (long)sizeof(struct video_area));
	*vm = (*vm & ~0x30) | 0x30;

	_setvideomode(_DEFAULTMODE);
#endif
	SET_INTERRUPT();
/*	_dos_setvect(0x60, trap60); */

#ifdef TSR
	{	struct PSP far *x;
		int top;
		FP_SEG(x) = _psp;
		FP_OFF(x) = 0;
		top = x->segend;
		_dos_keep(0,top-_psp);
	}
#else
	{	int r; 
		char *s;
		char cwd[80];
	   	s = getenv("HELIOS");
		if( s == NULL )
			s = "C:\\HELIOS";
		if( getcwd(cwd,sizeof(cwd)) == NULL )
		{
			printf("Unable to get cwd - %d\n",errno);
			exit(1);
		}

		if( chdir(s) )
		{
			printf("Unable to change dir \"%s\" - %d\n",s,errno);
			exit(1);
		}

		r = spawnvp(P_WAIT,"server",argv);
		chdir(cwd);

		if( r == -1 )
		{
			switch(errno)
			{
			case E2BIG:
				s = "Too big";
				break;
			case EACCES:
				s = "No access";
				break;
			case EMFILE:
				s = "Too many files";
				break;
			case ENOENT:
				s = "No entry";
				break;
			case ENOEXEC:
				s = "Not executable";
				break;
			case ENOMEM:
				s = "No memory";
				break;
			}
			if( s != NULL )
				printf("%s: Spawn failed: %s\n",argv[0],s);
			else
				printf("%s: Spawn failed: %d\n",argv[0],errno);
		}
	}

#ifdef TWO
	*vm = (*vm & 0xCF) | 0x20;
	lmemcpy( VIDEO_AREA, (char far *)col_prams, (long)sizeof(struct video_area));
	_setvideomode(_DEFAULTMODE);
#endif
	exit(0);
#endif
}
