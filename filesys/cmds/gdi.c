static char rcsid[] = "$Header: /hsrc/filesys/cmds/RCS/gdi.c,v 1.1 1990/10/05 16:40:35 nick Exp $";
/* $Log: gdi.c,v $
 * Revision 1.1  1990/10/05  16:40:35  nick
 * Initial revision
 *
 * Revision 1.2  90/01/09  14:14:25  chris
 * Addition of type field in volume info
 * 
 * Revision 1.1  90/01/09  14:01:30  chris
 * Initial revision
 * 
 */
 
#include <stdio.h>
#include <syslib.h>
#include <device.h>
#include <stddef.h>
#include <string.h>
#include <setjmp.h>
#include <module.h>
#include <stdlib.h>

#define DEBUG		0

#define isalpha(c) (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
#define isalnum(c) (isalpha(c) || ('0' <= c && c <= '9'))

#define tokfirst(c) (isalpha(c) || c == '/')
#define tokchar(c) (isalnum(c) || c == '/' || c == '.')

#define s_bra		1
#define s_ket		2
#define s_keyword	3
#define s_num		4
#define s_string	5
#define s_eof		6
#define s_semi		7
#define s_file		8
#define s_error		-1

#define inoffset(field) (offsetof(InfoNode,field))
#define fsoffset(field) (offsetof(FileSysInfo,field))
#define ddoffset(field) (offsetof(DiscDevInfo,field))
#define dvoffset(field) (offsetof(DriveInfo,field))
#define vvoffset(field) (offsetof(VolumeInfo,field))
#define ppoffset(field) (offsetof(Partition,field))
#define pioffset(field) (offsetof(PartitionInfo,field))
#define ssoffset(field) (offsetof(SerialInfo,field))
#define lloffset(field) (offsetof(Line,field))
#define evoffset(field) (offsetof(EventInfo,field))

typedef struct {
	char	*name;
	int	type;
	int	argtype;
	int 	offset;
} keysym;

#define t_fs		Info_FileSys
#define t_dd		Info_DiscDev
#define t_ss		Info_Serial
#define	t_param		-1
#define t_dv		-2
#define t_vv		-3
#define t_pp		-4
#define t_pi		-5
#define t_ll		-6
#define t_ev		-7
#define t_vt		-8

#define at_none		0
#define at_number	(s_num)
#define at_name		(s_string)
#define at_file		(s_file)
#define at_keyword	(s_keyword)

word parsefs(void);
word parsedd(void);
word parsess(void);
word parsedv(void);
word parsevv(void);
word parsepi(void);
word parsell(void);
word parseev(void);

keysym toptab[] = {
	{"fileserver",	t_fs, at_name, inoffset(Name)},
	{"discdevice",	t_dd, at_name, inoffset(Name)},
	{"serialserver",t_ss, at_name, inoffset(Name)},
	{"eventserver", t_ev, at_name, inoffset(Name)},
	{0,0}
};

keysym fstab[] = {
	{"device",	t_param, at_name,   fsoffset(DeviceName)},	
	{"code",	t_param, at_file,   fsoffset(FsCode)},
	{"volume",	t_vv,    at_none,   fsoffset(Volumes)},
	{"blocksize",	t_param, at_number, fsoffset(BlockSize)},
	{"cachesize",	t_param, at_number, fsoffset(CacheSize)},
	{"syncop",	t_param, at_number, fsoffset(SyncOp)},
	{"maxinodes",	t_param, at_number, fsoffset(MaxInodes)},
	{"smallpkt",	t_param, at_number, fsoffset(SmallPkt)},
	{"mediumpkt",	t_param, at_number, fsoffset(MediumPkt)},
	{"hugepkt",	t_param, at_number, fsoffset(HugePkt)},
	{"smallcount",	t_param, at_number, fsoffset(SmallCount)},
	{"mediumcount",	t_param, at_number, fsoffset(MediumCount)},
	{"hugecount",	t_param, at_number, fsoffset(HugeCount)},
	{"possindir",	t_param, at_number, fsoffset(PossIndir)},
	{"possdir",	t_param, at_number, fsoffset(PossDir)},
	{"possinode",	t_param, at_number, fsoffset(MinGood)},
	{"bitmaperrs",	t_param, at_number, fsoffset(BitMapErrs)},
	{0,0,0,0}
};

keysym vvtab[] = {
	{"name",	t_param, at_name,   vvoffset(Name)},
	{"partition",	t_pp,    at_none,   vvoffset(Partitions)},
	{"cgsize",	t_param, at_number, vvoffset(CgSize)},
	{"ncg",		t_param, at_number, vvoffset(CgCount)},
	{"cgoffset",	t_param, at_number, vvoffset(CgOffset)},
	{"minfree",	t_param, at_number, vvoffset(MinFree)},
	{"type",	t_vt,    at_none,   vvoffset(Type)},
	{0,0,0,0}
};

keysym vvttab[] = {
	{"raw",		vvt_raw},
	{"structured",	vvt_structured},
	{0}
};

keysym pitab[] = {
	{"drive",	t_param, at_number, pioffset(Drive)},
	{"start",	t_param, at_number, pioffset(StartCyl)},
	{"end", 	t_param, at_number, pioffset(EndCyl)},
	{0,0,0,0}
};

keysym ddtab[] = {
	{"name",	t_param, at_file,   ddoffset(Name)},
	{"controller",	t_param, at_number, ddoffset(Controller)},
	{"addressing",	t_param, at_number, ddoffset(Addressing)},
	{"mode",	t_param, at_number, ddoffset(Mode)},
	{"drive",	t_dv,    at_none,   ddoffset(Drives)},
	{"partition",	t_pi,    at_none,   ddoffset(Partitions)},	
	{0,0,0,0}
};

keysym dvtab[] = {
	{"id",		t_param, at_number, dvoffset(DriveId)},
	{"type",	t_param, at_number, dvoffset(DriveType)},
	{"sectorsize",	t_param, at_number, dvoffset(SectorSize)},
	{"sectors",	t_param, at_number, dvoffset(SectorsPerTrack)},
	{"tracks",	t_param, at_number, dvoffset(TracksPerCyl)},
	{"cylinders",	t_param, at_number, dvoffset(Cylinders)},
	{0,0,0,0}
}; 

keysym sstab[] = {
	{"device",	t_param, at_file,   ssoffset(DeviceName)},
	{"server",	t_param, at_file,   ssoffset(ServerName)},
	{"name",	t_param, at_name,   ssoffset(Name)},	
	{"line",	t_ll,    at_none,   ssoffset(Lines)},
	{"address",	t_param, at_number, ssoffset(Address)},
	{0,0,0,0}
};

keysym lltab[] = {
	{"name",	t_param, at_name,   lloffset(Name)},
	{"offset",	t_param, at_number, lloffset(Offset)},
	{0,0,0,0}
};

keysym evtab[] = {
	{"device",	t_param, at_file,   evoffset(DeviceName)},
	{"server",	t_param, at_file,   evoffset(ServerName)},
	{"name",	t_param, at_name,   evoffset(Name)},
	{"address",	t_param, at_number, evoffset(Address)},
	{0,0,0,0}
};

struct {
	FileSysInfo	FileSysInfo;
	VolumeInfo	VolumeInfo;
	Partition	Partition;
	DiscDevInfo	DiscDevInfo;
	DriveInfo	DriveInfo;
	PartitionInfo	PartitionInfo;
	SerialInfo	SerialInfo;
	Line		Line;
	EventInfo	EventInfo;
} Default = 
{
	{ 0, 0, -1, 4096, 256, 0, 128, 1, -1, -1, -1, -1, -1, -1, 80, 25, 60, 80 },
	{ -1, 0, -1, -1, -1, -1, 0, vvt_structured},
	{ -1, -1 },
	{ 0, 0, 1, 0, -1, -1 },
	{ -1, 0, 0, 256, 32, 4, 612 },
	{ -1, 0, 0, -1, -1 },
	{ -1, -1, -1, -1, 0 },
	{ -1, -1, 0 },
	{ -1, -1, 0 }
};

char *error_msgs[] = 
{
	NULL,	
	NULL,
	"digit too large for base",
	"unexpected character",
	"wrong argument type",
	"internal error",
	"keyword expected",
	"unknown keyword",
	"could not open input file",
	"could not allocate memory",
	"could not read input file",
	"could not open output file",
	"expected character not found",
	"keyword value not recognised"
};

int symb;
keysym *toksym;
union {
	int	i;
	char	*s;
} tokval;
char token[128];
int lineno = 1;

char *devinfo = NULL;
int dsize = 0;
int dpos = 0;

word infovec[1000];
int infosize = 0;

jmp_buf err_buf;

void error(int code)
{
	longjmp(err_buf,code);
}

int extend(int size)
{
	int oldsize = infosize;
	infosize += ((size+3) & ~3)/sizeof(word);
	return oldsize;
}

int rdch(void)
{
	if( dpos == dsize ) return EOF;
	else return devinfo[dpos++];
}

void unrdch(int c)
{
	dpos--;
	devinfo[dpos] = c;
}

int newstring(char *s)
{
	int off = extend(strlen(s)+1);
	char *t = (char *)(infovec+off);
	strcpy(t,s);
	return off;
}
	
int ctoi(int c)
{
	int val = 0;
	int d;
	int base = 10;
	int sign = 1;
		
	if( c == '0' )
	{
		c = rdch();
		if( c == 'x' ) 
		{
			c = rdch();
			base = 16;
		}
		else 
		{
			unrdch(c);
			base = 8;
		}	
	}
	elif( c == '-' )
	{
		c = rdch();
		sign = -1;
	}
	
	for(;;)
	{
		if( '0' <= c && c <= '7' ) d = c - '0';
		elif( base == 10 && (c == '8' || c == '9') ) d = c - '0';
		elif( base == 16 && 'a' <= c && c <= 'f' ) d = c - 'a' + 10;
		elif( base == 16 && 'A' <= c && c <= 'A' ) d = c - 'A' + 10;
		else break;
		if( d >= base ) error(2);
		val = val*base + d;
		c = rdch();
	}
	unrdch(c);
	return val*sign;
}

static void nextsym(keysym *keytab)
{
	char *t;
	int c;
	int toksize;
	
again:
	t  = token;
	
	do { 
		c = rdch();
		if( c == '\n' ) lineno++;
	} while( c == ' ' || c == '\t' || c == '\n' || c == '\r' );
	
	toksize = 0;
	
	if( tokfirst(c) )
	{
		keysym *k;
		int type = c=='/'?s_file:s_string;
				
		do {
			*t++ = c;
			c = rdch();
			if( c == '/' ) type = s_file;
		} while( tokchar(c) );
		
		unrdch(c);
		
		*t = 0;
		
		for(k = keytab;  k!= NULL && k->name != 0; k++ )
		{
			if( strcmp(token,k->name) == 0 )
			{
				symb = s_keyword;
				tokval.i = k->type;
				toksym = k;
#if DEBUG
IOdebug("keyword %s",token);
#endif
				return;
			}
		}
		
		symb = type;
		tokval.s = token;
		toksym = NULL;
#if DEBUG
IOdebug("string %s",token);
#endif
		return;
	}
	
	switch( c )
	{
	case EOF: symb = s_eof;	break;
	case '{': symb = s_bra; break;
	case '}': symb = s_ket; break;
	case ';': symb = s_semi; break;
	case '1': case '2': case '3':
	case '4': case '5': case '6':
	case '7': case '8': case '9':
	case '0': case '-':
		tokval.i = ctoi(c);
		symb = s_num;
#if DEBUG
IOdebug("number %d",tokval.i);
#endif
		return;
	case '#':
		while( c != '\n' ) c = rdch();
	case '\n': lineno++;
	case '\r':	
		goto again;
	default:
		error(3);
		break;
	}
#if DEBUG
IOdebug("symb %c",c);
#endif
	return;
}

void check(int sym)
{
	nextsym(NULL);
	if( symb != sym ) error(12);
}

void parsearg(int stroff)
{
	int offset = toksym->offset/4;
	int argtype = toksym->argtype;

	nextsym(NULL);
	
	if( argtype == symb || (argtype == s_file && symb == s_string) )
	{
		switch( symb )
		{
		case s_num:
			infovec[stroff+offset] = tokval.i;
			break;
		case s_string:
		case s_file:
			infovec[stroff+offset] = (word)(infovec+newstring(tokval.s));
			infovec[stroff+offset] = ATOR(infovec[stroff+offset]);
			break;
		default:
			error(5);
		}
	}
	else  error(4);
}

void parsesub(int stroff, word (*parse)(void))
{
	int offset = toksym->offset/4;
	int last = stroff+offset;
	int x = parse();

	while( infovec[last] != -1 ) last = last + infovec[last]/4;

	infovec[last] = (x-last)*4;
}

word parseinfo(void)
{
	InfoNode *i;
	int stroff;
	word info;
	word type;
	
	nextsym(toptab);
	
	if( symb == s_eof ) return EOF;
	
	if( symb != s_keyword ) error(6);
	
	stroff = extend(sizeof(InfoNode));
	type = tokval.i;
	
	parsearg(stroff);
	
	switch( type )
	{
	case t_fs: info = (word)parsefs(); break;
	case t_dd: info = (word)parsedd(); break;
	case t_ss: info = (word)parsess(); break;
	case t_ev: info = (word)parseev(); break;	
	default: error(7);
	}
	
	i = ((InfoNode *)&infovec[stroff]);
	
	i->Type = type;
	i->Info = (word)(infovec+info);
	i->Info = ATOR(i->Info);
	
	return stroff;
}

word parsefs(void)
{
	word stroff = extend(sizeof(FileSysInfo));

	check(s_bra);
		
	*(FileSysInfo *)(infovec+stroff) = Default.FileSysInfo;
	
	forever
	{
		nextsym(fstab);
		
		if( symb == s_ket ) break;
	
		if( symb == s_keyword && tokval.i == t_vv ) 
			parsesub(stroff,parsevv);
		elif( symb == s_keyword ) parsearg(stroff);
		else error(6);
	}	

	return stroff;	
}

word parsevv(void)
{
	word stroff = extend(sizeof(VolumeInfo));

	check(s_bra);

	*(VolumeInfo *)(infovec+stroff) = Default.VolumeInfo;
	
	forever
	{
		nextsym(vvtab);
		
		if( symb == s_ket ) break;
		
		if( symb == s_keyword)
		{
			switch( tokval.i )
			{
			case t_pp:
			{
				word ppoff = extend(sizeof(Partition));
				int last = stroff+toksym->offset/4;
				Partition *p = (Partition *)(infovec+ppoff);
				*p = Default.Partition;
				nextsym(NULL);
				if( symb != s_num ) error(4);
				p->Partition = tokval.i;
				while( infovec[last] != -1 ) 
					last = last + infovec[last]/4;
				infovec[last] = (ppoff-last)*4;
				break;
			}
			case t_vt:
			{	word vvtoff = stroff+toksym->offset/4;
				nextsym(vvttab);
				if( symb != s_keyword ) error(13);
				infovec[vvtoff] = tokval.i;
				break;
			}
			case t_param:
				parsearg(stroff); break;
			}
		}
		else error(6);
	}
	
	return stroff;
}

word parsedd(void)
{
	word stroff = extend(sizeof(DiscDevInfo));

	check(s_bra);

	*(DiscDevInfo *)(infovec+stroff) = Default.DiscDevInfo;
	
	forever
	{
		nextsym(ddtab);
		
		if( symb == s_ket ) break;
		
		if( symb == s_keyword && tokval.i == t_dv )
			parsesub(stroff,parsedv);
		elif( symb == s_keyword && tokval.i == t_pi )
			parsesub(stroff,parsepi);
		elif( symb == s_keyword ) parsearg(stroff);
		else error(6);
	}
		
	return stroff;
}

word parsedv(void)
{
	word stroff = extend(sizeof(DriveInfo));

	check(s_bra);
	
	*(DriveInfo *)(infovec+stroff) = Default.DriveInfo;

	forever
	{
		nextsym(dvtab);
		
		if( symb == s_ket ) break;
		
		if( symb == s_keyword ) parsearg(stroff);
		else error(6);
	}	
	
	return stroff;
}

word parsepi(void)
{
	word stroff = extend(sizeof(PartitionInfo));

	check(s_bra);
	
	*(PartitionInfo *)(infovec+stroff) = Default.PartitionInfo;

	forever
	{
		nextsym(pitab);
		
		if( symb == s_ket ) break;
		
		if( symb == s_keyword ) parsearg(stroff);
		else error(6);
	}	
	
	return stroff;
}

word parsess(void)
{
	word stroff = extend(sizeof(SerialInfo));

	check(s_bra);
	
	*(SerialInfo *)(infovec+stroff) = Default.SerialInfo;
	
	forever
	{
		nextsym(sstab);
		
		if( symb == s_ket ) break;

		if( symb == s_keyword && tokval.i == t_ll ) 
			parsesub(stroff,parsell);
		elif( symb == s_keyword ) parsearg(stroff);
		else error(6);
	}
	
	return stroff;
}

word parsell(void)
{
	word stroff = extend(sizeof(Line));

	check(s_bra);
	
	*(Line *)(infovec+stroff) = Default.Line;

	forever
	{
		nextsym(lltab);
		
		if( symb == s_ket ) break;
		
		if( symb == s_keyword ) parsearg(stroff);
		else error(6);
	}	
	
	return stroff;
}

word parseev(void)
{
	word stroff = extend(sizeof(EventInfo));

	check(s_bra);
	
	*(EventInfo *)(infovec+stroff) = Default.EventInfo;

	forever
	{
		nextsym(evtab);
		
		if( symb == s_ket ) break;
		
		if( symb == s_keyword ) parsearg(stroff);
		else error(6);
	}	
	
	return stroff;
}

void parsedevinfo(char *file)
{
	Stream *s = NULL;
	word info = NULL;
	word last = -1;
			
	s = Open(CurrentDir, file, O_ReadOnly);
	
	if( s == NULL ) error(8);

	dsize = GetFileSize(s);

	devinfo = Malloc(dsize);

	if( devinfo == NULL ) error(9);

	if( Read(s,devinfo,dsize,-1) != dsize ) error(10);

	while((info = parseinfo()) != EOF )
	{
		if( last != -1 )
		{
			infovec[last] = (word)(infovec+info);
			infovec[last] = ATOR(infovec[last]);
		}
		last = info;
	}
}

void writedevinfo(char *file)
{
	Stream *s = NULL;
	ImageHdr hdr;
	Module module;
	int size = infosize*sizeof(word);	
	int zero = 0;
					
	s = Open(CurrentDir, file, O_WriteOnly|O_Create);
	
	if( s == NULL ) error(11);

	hdr.Magic = Image_Magic;
	hdr.Flags = 0;
	hdr.Size = size + sizeof(Module) + sizeof(zero);
	
	Write(s,(byte *)&hdr,sizeof(hdr),-1);
	
	module.Type = T_DevInfo;
	module.Size = sizeof(Module)+size;
	strcpy(module.Name,"DevInfo");
	module.Id = -1;
	module.Version = 1000;
	module.MaxData = 0;
	module.Init = 0;

	Write(s,(byte *)&module,sizeof(Module),-1);
	
	Write(s,(byte *)infovec,size,-1);

	Write(s,(byte *)&zero,sizeof(zero),-1);
		
	Close(s);
}

int main(int argc, char **argv)
{
	word e;
	
	if( argc != 3 )
	{
		printf("usage: %s source dest\n",argv[0]);
		exit(1);
	}

	if( (e=setjmp(err_buf)) == 0 )
	{
		parsedevinfo(argv[1]);
		
		writedevinfo(argv[2]);
	}

	if( devinfo != NULL ) Free(devinfo);
	
	if( e != 0 ) 
	{
		char *msg = error_msgs[e];
		printf("Error on line %d : %s\n",lineno,msg);
	}
}

