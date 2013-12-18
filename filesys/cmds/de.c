static char rcsid[] = "$Header: /usr/perihelion/Helios/filesys/cmds/RCS/de.c,v 1.1 90/10/05 16:40:31 nick Exp $";

/* $Log:	de.c,v $
 * Revision 1.1  90/10/05  16:40:31  nick
 * Initial revision
 * 
 * Revision 1.2  90/01/24  17:19:29  chris
 * Kill (k) and Verify (a!) commands added.
 * 
 * Revision 1.1  90/01/09  13:32:43  chris
 * Initial revision
 * 
 */

#define DEBUG 0

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <syslib.h>
#include <device.h>
#include <setjmp.h>
#include <signal.h>
#include <time.h>

#include <codes.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <gsp.h>
#include <asm.h>

#include "buf.h"
#include "fs.h"
#include "inode.h"

#include "rdevinfo.c"

jmp_buf errlev;
jmp_buf deflev;

DCB *discdcb;

int	curblock = 0;
int	blocksize = 4096;
int	blockpos = 0;
bool	readonly = TRUE;
bool	written = TRUE;
bool	valid = FALSE;
bool	interrupt = FALSE;

byte	*blockbuf = NULL;

word	ncg;
word	cgoffs;
word	maxbpg;

void tidyup(int rc);

void fatal(char *str,...)
{
	va_list a;
	va_start(a,str);
	fprintf(stderr,"FATAL: ");
	vfprintf(stderr,str,a);
	fprintf(stderr,"\n");
	tidyup(1);
}


void error(char *str,...)
{
	va_list a;
	va_start(a,str);
	fprintf(stderr,"ERROR: ");
	vfprintf(stderr,str,a);
	fprintf(stderr,"\n");
	longjmp(errlev,1);
}

void warn(char *str,...)
{
	va_list a;
	va_start(a,str);
	fprintf(stderr,"WARNING: ");
	vfprintf(stderr,str,a);
	fprintf(stderr,"\n");
}

int askint(char *s, int def)
{
	int c;

	printf("%s (%d):",s,def); fflush(stdout);

	while((c=getchar()) == ' ');
	ungetc(c,stdin);
	if( c != '\n' ) 
	{
		scanf("%d",&def);
	}
	while(getchar() != '\n');
	
	return def;
}

void askstr(char *prompt, char *s, int len)
{
	char c;
	printf("%s (%s):",prompt,s); fflush(stdout);

	while((c=getchar()) == ' ');

	if( c != '\n' ) 
	{
		int i;
		ungetc(c,stdin);
		for( i=0; i < len && c != '\n' ; i++ )
			c = s[i] = getchar();
		s[i] = 0;
	}

	while(c != '\n') c=getchar();
}

void init_disc(char *name)
{
#if DEBUG
	printf("Open Disc device\n");
#else
	void *devinfo;
	InfoNode *info;
	FileSysInfo *fsi;
	VolumeInfo *vvi;
	DiscDevInfo *ddi;

	if( (devinfo = load_devinfo()) == NULL )
	{	fprintf(stderr,"Can't find devinfo file\n");
		exit(1);
	}

	if( (info = find_info(devinfo,Info_FileSys,name)) == NULL )
	{	fprintf(stderr,"Can't find devinfo entry for %s\n",name);
		exit(1);
	}
	
	fsi = (FileSysInfo *)RTOA(info->Info);
	vvi = (VolumeInfo *)RTOA(fsi->Volumes);

	info = find_info(devinfo,Info_DiscDev,RTOA(fsi->DeviceName));
	ddi = (DiscDevInfo *)RTOA(info->Info);

	init_info(fsi,ddi);
	
	discdcb = OpenDevice(RTOA(ddi->Name),ddi);
	
	if( discdcb == NULL ) fatal("cannot create device driver");
	
	ncg = vvi->CgCount;
	cgoffs = vvi->CgOffset;
	maxbpg = vvi->CgSize;

#endif
}

void tidy_disc(void)
{
#if DEBUG
	printf("Close Disc device\n");
#else
	word e;
	
	if( discdcb == NULL ) return;

	e = CloseDevice(discdcb);
	
	if( e != Err_Null ) warn("failed to close disc device: %x",e);
#endif
}

void dev_action(DiscReq *req)
{
	Signal(&req->WaitLock);
}

void fmt_action(FormatReq *req)
{
	Signal(&req->WaitLock);
}


int readblock(void *buf, int block)
{
#if DEBUG
	printf("Read block %d size %d\n",block,blocksize);
	return 1;
#else
	{
		DiscReq req;
		
		req.DevReq.Request = FG_Read;
		req.DevReq.Action = dev_action;
		req.DevReq.Timeout = -1;
		req.DevReq.SubDevice = 0;
		req.Size = blocksize;
		req.Pos  = block * blocksize;
		req.Buf = buf;
		InitSemaphore(&req.WaitLock,0);
			
		Operate(discdcb,&req);

		Wait(&req.WaitLock);
	
		if( req.DevReq.Result != 0 || req.Actual != blocksize )
		{
			error("Read Failed: %x",req.DevReq.Result);
			return 0;
		}
		else
			return 1;
	}
#endif
}

void writeblock(void *buf, int block)
{
	if( readonly ) error("Write protect set");
	
#if DEBUG
	printf("Write block %d size %d\n",block,blocksize);
#else
	{
		DiscReq req;
		
		req.DevReq.Request = FG_Write;
		req.DevReq.Action = dev_action;
		req.DevReq.Timeout = -1;
		req.DevReq.SubDevice = 0;
		req.Size = blocksize;
		req.Pos  = block * blocksize;
		req.Buf = buf;
		InitSemaphore(&req.WaitLock,0);
		
		Operate(discdcb,&req);
	
		Wait(&req.WaitLock);

		if( req.DevReq.Result != 0 || req.Actual != blocksize )
			error("Write Failed: %x",req.DevReq.Result);		
	}
#endif
}

int discsize()
{
#if DEBUG
	printf("Disc Size\n");
	return 1024*1024;
#else

	DiscReq req;
	
	req.DevReq.Request = FG_GetSize;
	req.DevReq.Action = dev_action;
	req.DevReq.SubDevice = 0;
	InitSemaphore(&req.WaitLock,0);
		
	Operate(discdcb,&req);
	
	Wait(&req.WaitLock);

	if( req.DevReq.Result < 0 ) error("Error from disc: %x",req.DevReq.Result);
	
	return req.DevReq.Result;
#endif
}

void validate(void)
{
	DiscReq req;
	int i;
	int dsize = discsize()/blocksize;
	int done;
	int total, time;
		
	for( i = 0; i < blocksize; i++ ) blockbuf[i] = i>>8;
	
	req.DevReq.Request = FG_Write;
	req.DevReq.Action = dev_action;
	req.DevReq.Timeout = -1;
	req.DevReq.SubDevice = 0;
	req.Size = blocksize;
	req.Pos  = 0;
	req.Buf = blockbuf;
	InitSemaphore(&req.WaitLock,0);

	total = 0;
	interrupt = FALSE;	
	for(i = curblock; i < dsize ; i++ )
	{
		*(word *)blockbuf = i;
		req.Pos = i*blocksize;
		if( i % 10 == 0 ) { printf("W %6d %6d %6d\r",i,total,total/i); fflush(stdout); }
		time = ldtimer_();
		Operate(discdcb,&req);
		Wait(&req.WaitLock);
		time = diff_(time,ldtimer_());
		if( req.DevReq.Result < 0 ) 
			printf("Block %d write error %x\n",i,req.DevReq.Result);
		done = i;
		if( i==curblock ) total = time+time*i;
		else
		{
			if( time > 4*total/i ) 
			    printf("Block %d write time(%d) > 4*average(%d)\n",i,time,total/i);
			total += time;
		}
		if( interrupt ) break;
	}
	
	
	req.DevReq.Request = FG_Read;

	total = 0;
	interrupt = FALSE;	
	for(i = curblock; i < done; i++ )
	{
		int j;
		req.Pos = i*blocksize;		
		if( i % 10 == 0 ) { printf("R %6d %6d %6d\r",i,total,total/i); fflush(stdout); }
		time = ldtimer_();
		Operate(discdcb,&req);
		Wait(&req.WaitLock);
		time = diff_(time,ldtimer_());
		if( req.DevReq.Result < 0 )
		{	printf("Block %d read error %x\n",i,req.DevReq.Result);
			continue;
		}
		if( i==curblock ) total = time+time*i;
		else 
		{
			if( time > 4*total/i ) 
			    printf("Block %d read time(%d) > 4*average(%d)\n",i,time,total/i);
			total += time;
		}
		if( *(word *)blockbuf != i )
			printf("Block %d found where block %d expected\n",
				*(word *)blockbuf, i);
		for(j = 4; j < blocksize; j++ )
			if( blockbuf[j] != (j>>8) )
				printf("byte %d of block %d read %x not %x\n",
					j,i,blockbuf[j],(j>>8) );
		if( interrupt ) break;
	}	
	ungetc('\n',stdin);		
}

void format(void)
{
	static int start = 0;
	static int end = 611;
	static int interleave = 1;
	static int trackskew = 0;
	static int cylskew = 0;
	int cyl;
	FormatReq req;
	
	start = askint("Start Cylinder",start);
	end   = askint("End Cylinder",end);
	interleave = askint("Interleave",interleave);
	trackskew = askint("Track Skew",trackskew);
	cylskew = askint("Cylinder Skew",cylskew);	

	req.DevReq.Request = FG_Format;
	req.DevReq.Action = fmt_action;
	req.DevReq.SubDevice = 0;
	InitSemaphore(&req.WaitLock,0);
	
	req.Interleave = interleave;
	req.TrackSkew = trackskew;
	req.CylSkew = cylskew;
	req.StartCyl = start;
	req.EndCyl = end;
		
	Operate(discdcb,&req);	

	Wait(&req.WaitLock);
		
	if( req.DevReq.Result < 0 ) error("Error from disc: %x",req.DevReq.Result);

#if 0
	for( cyl = start; cyl <= end; cyl++ )
	{
		req.Cylinder = cyl;
		
		printf("\rCylinder %4d",cyl); fflush(stdout);
		
		Operate(discdcb,&req);
		
		Wait(&req.WaitLock);

		if( req.DevReq.Result < 0 ) error("Error from disc: %x",req.DevReq.Result);
	}
#endif	

	ungetc('\n',stdin);
}

void rwtest(void)
{
	char *buf;
	int size;
	static int ksize = 64;
	DiscReq req;
	int start, mid, end;
	int i;
	
	ksize = askint("Buffer Size in Kbytes",ksize);
	
	size = ksize*1024;
	
	buf = malloc(size);
	
	if( buf == NULL ) error("cannot get buffer");
	
	for( i = 0; i < size; i++ ) buf[i] = i>>8;
	
	req.DevReq.Request = FG_Write;
	req.DevReq.Action = dev_action;
	req.DevReq.Timeout = -1;
	req.DevReq.SubDevice = 0;
	req.Size = size;
	req.Pos  = 0;
	req.Buf = buf;
	InitSemaphore(&req.WaitLock,0);
			
	start = _ldtimer(0);
	
	Operate(discdcb,&req);
	Wait(&req.WaitLock);
	
	mid = _ldtimer(0);
	
	req.DevReq.Request = FG_Read;

	Operate(discdcb,&req);
	Wait(&req.WaitLock);
		
	end = _ldtimer(0);
	
	printf("Write %d usec\nRead %d usec\n",mid-start,end-mid);
	
	for( i = 0; i < size; i++ ) 
		if( buf[i] != (i>>8) )
		{
			printf("Data error at %06x: %02x %02x\n",i,buf[i],i>>8);
		}
		
	free(buf);

	ungetc('\n',stdin);	
}

void makeboot()
{
	Stream *s = Open(CurrentDir,"/helios/lib/fssys",O_ReadOnly);
	byte *buf;
	int size;
	DiscReq req;
	
	if( s == NULL ) error("cannot open fssys");
	
	size = GetFileSize(s);
	
	buf = malloc(size);
	
	if( buf == NULL ) error("cannot allocate buffer");
	
	req.DevReq.Request = FG_WriteBoot;
	req.DevReq.Action = dev_action;
	req.DevReq.Timeout = -1;
	req.DevReq.SubDevice = 0;
	req.Size = size;
	req.Pos  = 0;
	req.Buf = buf;
	InitSemaphore(&req.WaitLock,0);
		
	Operate(discdcb,&req);
	
	Wait(&req.WaitLock);

	if( req.DevReq.Result != 0 || req.Actual != blocksize )
		error("Write Failed: %x",req.DevReq.Result);		
		
	free(buf);
}

void info()
{
	int ds = discsize();
	printf("Disc size (bytes)        %d\n",ds);
	printf("Disc size (blocks)       %d\n",ds/blocksize);
	printf("Block size               %d\n",blocksize);
	if( valid )
	{
		printf("Current block            %d\n",curblock);
		printf("Cylinder group           %d\n",btocg(curblock));
		printf("Offset in cylinder group %d\n",btocgb(curblock));
	}
	else
		printf("No current valid block\n");
}


void putde(int offset, struct dir_elem *dir)
{
	struct inode *inode = &dir->de_inode;

	if( inode->i_mode != FREE )
	{
		char *mstr;
		char mat[40];
	
		switch( inode->i_mode )
		{
		case DATA: mstr = "f"; break;
		case DIR : mstr = "d"; break;
		case LINK: mstr = "l"; break;
		}
		DecodeMatrix(mat,inode->i_matrix,inode->i_mode);
		
		printf("%2d %s %s %4d %6d [%4d blocks] %4d %s\n",
			offset,mstr,mat,inode->i_accnt,inode->i_size,
			inode->i_blocks,inode->i_spare,dir->de_name);
	}
}

void changede(struct dir_elem *de)
{
	char mat[40];
	char mstr[10];
	struct inode *inode = &de->de_inode;

	switch( inode->i_mode )
	{
	case FREE: strcpy(mstr,"FREE"); 	break;
	case DATA: strcpy(mstr,"f");		break;
	case DIR : strcpy(mstr,"d");		break;
	case LINK: strcpy(mstr,"l");		break;
	}
	DecodeMatrix(mat,inode->i_matrix,inode->i_mode);
		
	askstr("Name",de->de_name,31);
	for(;;)
	{
		askstr("Mode",mstr,10);
		if( !strcmp(mstr,"free") ) inode->i_mode = FREE;
		elif( !strcmp(mstr,"FREE") ) inode->i_mode = FREE;
		elif( !strcmp(mstr,"f") ) inode->i_mode = DATA;
		elif( !strcmp(mstr,"d") ) inode->i_mode = DIR;
		elif( !strcmp(mstr,"l") ) inode->i_mode = LINK;
		else 
		{
			printf("invalid type\n");
			continue;
		}
		break;
	}
	
	askstr("Matrix",mat,40);
	inode->i_matrix = EncodeMatrix(mat,inode->i_mode);
	
	inode->i_accnt = askint("Account",inode->i_accnt);
	
	if( inode->i_mode != LINK )
	{
		inode->i_size  = askint("Size",inode->i_size);
		inode->i_blocks = askint("Blocks",inode->i_blocks);
		if( inode->i_mode == DIR )
			inode->i_spare = askint("Entries",inode->i_spare);
	}
}

void putdir(void *buf)
{
	struct dir_elem *dir = buf;
	int i;
	int nfree = 0;
	int nused = 0;
	for( i = 0; i < blocksize/sizeof(struct dir_elem); i++,dir++ )
	{
		if( dir->de_inode.i_mode == FREE ) nfree++;
		else { nused++; putde(i,dir); }
	}
	printf("%d entries used %d free\n",nused,nfree);
}

void putfile(void *buf, int offset)
{
	daddr_t *ibuf = NULL;
	struct dir_elem *de = buf;
	int i;
	int nblocks;
	
	de = &de[offset];
	
	putde(offset,de);

	nblocks = de->de_inode.i_blocks;
	
#if 0	
	if( nblocks ) 
	{
		printf("Direct blocks:\n");
		for( i = 0 ; i < NDADDR/2 && nblocks ; i++,nblocks-- ) 
			printf("%6d ",de->de_inode.i_db[i]);
		printf("\n");
		for( ; i < NDADDR && nblocks; i++,nblocks-- )
			printf("%6d ",de->de_inode.i_db[i]);
		printf("\n");
	}
#else
	printf("Direct blocks:");
	for( i = 0; i < NDADDR ; i++ )
	{
		if(  (i % 8) == 0 ) printf("\n");
		printf("%s%6d%s ",nblocks?" ":"[",de->de_inode.i_db[i],nblocks?" ":"]");
		if( nblocks ) nblocks--;
	}
	printf("\n");
#endif
	if( nblocks )
	{
		ibuf = malloc(blocksize);
		
		if( ibuf == NULL ) error("Cannot allocate block buffer");
		
		printf("First indirect: %d",de->de_inode.i_ib[0]);
		nblocks--;

		readblock(ibuf,de->de_inode.i_ib[0]);
		
		for( i = 0; i < SIADDR && nblocks; i++,nblocks-- )
		{
			if( i % 8 == 0 ) printf("\n");
			printf("%6d ",ibuf[i]);	
		}
		if( i % 8 != 0 ) printf("\n");
	}
	
	if( nblocks )
	{
		printf("Double indirect: %d\n",de->de_inode.i_ib[1]);
		printf("Triple indirect: %d\n",de->de_inode.i_ib[2]);
	}
	
	free(ibuf);
}

void putsum(struct sum *sum)
{
	printf("%3d dirs %4d blocks %3d frags\n",
		sum->s_ndir,sum->s_nbfree,sum->s_nffree);
}

void putsumblk(void *buf)
{
	int i;
	struct sum_blk *sb = buf;
	
	printf("Root Dir:\n"); putde(0,&sb->root_dir);
	
	printf("FS summary: "); putsum(&sb->fs_sum);
	
	printf("Cylinder group summaries:\n");
	
	for( i = 0; i < ncg ; i++ )
	{
		printf("cg %3d [block %6d] : ",i,cgtoib(i)); putsum(&sb->cg_sum[i]);
	}
	
	printf("Sum same flag = %s\n",sb->sum_same?"TRUE":"FALSE");
}

void putsuper(struct fs *fs)
{
	printf("File system name       : %s\n",fs->fs_name);
	printf("Created                : %s",ctime(&fs->fs_time));
	printf("Magic number           : %x\n",fs->fs_magic);

	printf("Summary block          : %d\n",fs->fs_sblknr);
	printf("Info block             : %d\n",fs->fs_iblknr);
	printf("Root directory         : %d\n",fs->fs_rblknr);

	printf("File System size       : %d blocks\n",fs->fs_size);
	printf("Data blocks            : %d\n",fs->fs_dsize);
	printf("Block size             : %d bytes\n",fs->fs_bsize);
	printf("Fragments per block    : %d\n",fs->fs_frag);
	printf("Fragment size          : %d bytes\n",fs->fs_fsize);
	printf("Min free blocks        : %d%%\n",fs->fs_minfree);
	printf("Max contiguous         : %d blocks\n",fs->fs_maxcontig);

	printf("Cylinder group size    : %d blocks\n",fs->fs_cgsize);
	printf("Cylinder group offset  : %d blocks\n",fs->fs_cgoffset);
	printf("Cylinder groups        : %d\n",fs->fs_ncg);
	
	printf("Cyl group info size    : %d bytes\n",fs->fs_szcg);
	printf("Superblock info size   : %d bytes\n",fs->fs_szfs);
}

int bitsin(char c)
{
	int bits = 0;
	int i;
	for( i = 0 ; i < 8 ; i++ ) if( c & (1<<i) ) bits++;
	return bits;
}

void putcg(void *buf)
{
	int i;
	struct cg *cg = buf;
	
	printf("Cylinder Group # :   %d\n",cg->cg_cgx);
	
	printf("Summary          : "); putsum(&cg->cg_s);
	
	printf("Last written     : %s",ctime(&cg->cg_time));
	
	printf("Data Blocks      : %d\n",cg->cg_ndblk);
	
	printf("Magic Number     : %x\n",cg->cg_magic);
	
	printf("Rotor            : %d\n",cg->cg_rotor);
	
	printf("Allocation Map   :\n");
	
	/* currently show whole blocks. When fragments are available */
	/* show those.						     */
	for( i = 0 ; i < maxbpg ; i++ )
	{
		if( i % 64 == 0 ) printf("\n%3d: ",i);
		printf("%1d",bitsin(cg->cg_free[i]));
	}
	if( i % 64 != 0 ) printf("\n");

	printf("Copy of SuperBlock:\n");
		
	putsuper((struct fs *)(cg+1));
}

void printhex(c)
char c;
{
	if (c<10) putchar(c+'0');
	else putchar(c-10+'A');
}

void display()
{
	char *buffer, *end;
	int offset = 0;
	
	if( !valid ) error("No current block");
	
	end = &blockbuf[blocksize];
	
	interrupt = FALSE;
	for ( buffer = blockbuf ; buffer < end && !interrupt; buffer += 16 )
	{
		int i;
		printf("%04x: ",offset);
		for (i = 0; i < 16; i++)
		{	
			char c = buffer[i];
			printhex(c>>4);
			printhex(c&0xf);
			putchar(' ');
		}
		printf("   ");
		for (i = 0; i < 16; i++)
		{	
			if (!isprint(buffer[i])) putchar('.');
			else putchar(buffer[i]);
		}
		putchar('\n');
		offset += 16;
	}
	interrupt = FALSE;
}
void puthelp()
{
printf(
"a              Validate disc\n"
"b <n>          Set block size in bytes\n"
"c              Interpret current block as a cylinder group\n"
"d              Display block\n"
"e <n>          Edit directory entry at slot n\n"
"f <n>          Show structure of file at dir slot n\n"
"h              Help\n"
"k <n>          Kill block\n"
"l              Interpret block as a directory block\n"
"m              Format\n"
"q              Quit\n"
"r [<n>]        Read block <n> or current\n"
"s              Interpret block as disc summary block\n"
"t              Read/Write test\n"
"v              Toggle block written flag\n"
"w [<n>]        Write block <n> or current\n"
"x              Toggle write protect\n"
"z              Zero block\n"
"?              Info\n"
"<hex>          Set block position\n"
"= <hex>        Change byte at current block position\n"	
);
}

void newbuf()
{
	if( blockbuf != NULL ) free(blockbuf);
	
	blockbuf = malloc(blocksize);
	
	if( blockbuf == NULL ) fatal("Cannot allocate buffer");
}

int killblock(int blk)
{	int cgib;
	int cgnr;
	int cgblk;
	struct info_blk *ib;
	struct sum_blk *sb;
	struct sum cgsum;

	cgnr = btocg(blk);
	cgib = cgtoib(cgnr);
	cgblk = btocgb(blk);
	cgsum = ib->cgx.cg_s;
	
	if( !readblock(blockbuf,cgib) )
	{
		printf("Failed to read cylinder group info block");
		return 0;
	}
	ib = (struct info_blk *)blockbuf;
	if( ib->cgx.cg_free[cgblk] )
	{
		printf("Block %d is allocated - not registered as bad\n",blk);
		return 0;
	}
	ib->cgx.cg_free[cgblk] = 0xff;
	ib->cgx.cg_s.s_nbfree--;
	printf("no. free is now %d\n",ib->cgx.cg_s.s_nbfree);
	writeblock(blockbuf,cgib);
	cgsum = ib->cgx.cg_s;

	if( !readblock(blockbuf,1) )
	{
		printf("Failed to read summary block");
		return 0;
	}
	sb = (struct sum_blk *)blockbuf;
	sb->fs_sum.s_nbfree--;
	sb->cg_sum[cgnr].s_nbfree--;
	
	if( sb->cg_sum[cgnr].s_nbfree != cgsum.s_nbfree ||
	    sb->cg_sum[cgnr].s_ndir    != cgsum.s_ndir )
		printf("Warning: cylinder group summaries do not match");
	
	writeblock(blockbuf,1);
	valid = 0;
	return 1;
}

void killblocks()
{	int blk;
	int r;
	int c;
	int nblks = 0;
	
	while( (c = getchar()) != '\n' )
	{
		while( isspace(c) ) c=getchar();
		ungetc(c,stdin);
		r = scanf("%d",&blk);
		if( r == 0 )
		{	printf("Block no. required\n");
			return;
		}
		if( !killblock(blk) )
			printf("Failed to kill block %d\n",blk);
		else
			nblks++;
	}
	printf("%d blocks killed\n",nblks);
	ungetc('\n',stdin);
}

void disced(void)
{
	int c;
	jmp_buf reclev;
	jmp_buf oldlev;
	
	newbuf();
	
	memcpy(oldlev,errlev,sizeof(jmp_buf));
	
	if( setjmp(reclev) )
	{
		while( getchar() != '\n' );
	}
	else memcpy(errlev,reclev,sizeof(jmp_buf));

	for( c = '\n'; ; c = getchar() )
	{
		switch( c )
		{
		default: error("illegal command: %c",c);
		
		case '\n':
		case '\r':
			printf("de: "); 
			fflush(stdout);
			break;
			
		case 'q':
			if( !written ) error("Current block not written"); 
			goto done;

		case 'h': puthelp(); break;

		case 'r': 
			while((c=getchar()) == ' ');
			ungetc(c,stdin);
			if( c != '\n' ) scanf("%d",&curblock);

			if( !written ) error("Current block not up-to-date on disc");
			valid = FALSE;		
			valid = readblock(blockbuf,curblock); 
			written = TRUE;
			break;
		
		case 'w': 
			if( !valid ) error("No block to write!!");

			while((c=getchar()) == ' ');
			ungetc(c,stdin);
			if( c != '\n' ) 
			{
				scanf("%d",&curblock);
				written = FALSE;
			}

			writeblock(blockbuf,curblock); 
			written = TRUE;	
			break;
		
		case 'x':
			readonly = !readonly;
			printf("Write protect %s\n",readonly?"ON":"OFF");
			break;

		case 'b':
			scanf("%d",&blocksize);
			newbuf();
			break;
		
		case 'k':	/* Kill block(s) */
			killblocks();
			break;

		case 'l':
			putdir(blockbuf);
			break;

		case 'f':
		{
			int offset;
			scanf("%d",&offset);
			putfile(blockbuf,offset);
			break;
		}

		case 'e':
		{
			int offset;
			scanf("%d",&offset);
			while(getchar() != '\n');
			changede(&((struct dir_elem *)blockbuf)[offset]);
			break;
		}

		case 's':
			putsumblk(blockbuf);
			break;

		case 'c':
			putcg(blockbuf);
			break;

		case '?': info(); break;
		
		case 'd': display(); break;
		
		case '1': case '2': case '3':
		case '4': case '5': case '6':
		case '7': case '8': case '9':
		case '0':
			ungetc(c,stdin);
			scanf("%x",&blockpos);
			blockpos %= blocksize;
			break;
			
		case '=':
		{
			int v = 0;
			scanf("%x",&v);
			if( !valid ) error("No current block");
			blockbuf[blockpos] = v;
			written = FALSE;
			break;
		}	
		
		case 'v':
			if( !valid ) error("No current block");
			written = !written;
			printf("Written flag %s\n",written?"set":"cleared");
			break;
			
		case 'z':
		{
			int i;
			for( i = 0; i < blocksize; i++) blockbuf[i] = 0;
			written = FALSE;
			break;
		}
		case 'm':
			while(getchar() != '\n');
			format();
			break;
		case 't':
			while(getchar() != '\n');
			rwtest();
			break;
		case 'a':
			while(getchar() != '\n');
			validate();
			break;

		}
	}
done:
	memcpy(errlev,oldlev,sizeof(jmp_buf));
}

void ctrlc(int sig)
{
	interrupt = TRUE;
	signal(sig, ctrlc);
}

int main(int argc, char **argv)
{
	if( argc != 2 )
	{
		printf("usage: %s fsname\n",argv[0]);
		exit(1);
	}

	init_disc(argv[1]);

	printf("Helios Disc Editor V1.0\n");
	printf("(C) Copyright 1989 Perihelion Software Ltd.\n");
	printf("All Rights Reserved\n\n");
	
	if( !setjmp(deflev) )
	{
		memcpy(errlev,deflev,sizeof(jmp_buf));		
		signal(SIGINT, ctrlc);
		disced();
	}
	
	tidyup(0);
	
	return 0;
}

void tidyup(int rc)
{
	tidy_disc();
	
	exit(rc);	
}

