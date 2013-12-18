#include <stdio.h>
#include <nonansi.h>
#include <attrib.h>
#include <memory.h>
#include <ctype.h>
#include <root.h>
#include <posix.h>
#include <stdlib.h>
#include <task.h>
#include <stddef.h>
#include <servlib.h>
#include <module.h>
#include <link.h>
#include <string.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>

#include <sys/types.h>

/*
 * Basic system types.
 */

#define	quad		quad_t
#define MAXNAMLEN	1024

typedef	long	daddr_t;
typedef	void 	(*sig_t)();
typedef	struct	_quad_t { long val[2]; } quad_t;
typedef	struct	_u_quad { unsigned long val[2]; } u_quad;
typedef	long	qaddr_t;

#include "include/sys/malloc.h"
#include "include/sys/mount.h"
#include "ufs.h"

/* Variables */
int screenx, screeny;
int mapx, mapy;
int mapsize;
int physx, physy;
int poolx;
bool flash = FALSE;
int mode;

Attributes ostate, nstate;

/* Global Vars */
char *volname;
struct kmemstats kmemstats[M_LAST];
u_int bufcachehits, bufcachemiss, total_reads, actual_reads, 
		total_writes, actual_writes, desiredvnodes, numvnodes,
		current_users, max_users, user_calls, 
		stack_size, client_size, msg_size;
struct statfs buf;
byte *OF_Buffer;
int OF_BSize, OF_Marker;


void ansiopen()
{
	GetAttributes(Heliosno(stdin),&ostate);
	nstate = ostate;
	AddAttribute(&nstate,ConsoleRawInput);
	AddAttribute(&nstate, ConsoleRawOutput);
	RemoveAttribute(&nstate, ConsolePause);
	RemoveAttribute(&nstate, ConsoleIgnoreBreak);
	RemoveAttribute(&nstate, ConsoleBreakInterrupt);
	RemoveAttribute(&nstate, ConsoleEcho);
	SetAttributes(Heliosno(stdin),&nstate);
}

void ansiclose()
{
	SetAttributes(Heliosno(stdin),&ostate);
}

int putch(int c) 
{
	putchar(c); 
	if( isprint(c) ) physx++;
}

void ansiparm(n)
int    n;
{
        register int q,r;

        q = n/10;
        if (q != 0) {
		r = q/10;
		if (r != 0) {
			putch((r%10)+'0');
		}
		putch((q%10) + '0');
        }
        putch((n%10) + '0');
}

void poscurs(int x, int y)
{
	if( physx == x && physy == y ) return;
	putch(0x1b);
	putch('[');
	ansiparm(y+1);
	putch(';');
	ansiparm(x+1);
	putch('H');
	fflush(stdout);
	physx = x; 
	physy = y;
}

int getch(int timeout)
{
	char c = 0;
	Read(fdstream(0),&c,1,timeout);
	return c;
}

int clrscrn()
{
	putch(12);	
	physx = physy = 0;
}

int clreol()
{
	putch(0x1b);
	putch('[');
	putch('K');
}

void getxy()
{
	Attributes state;
	GetAttributes(Heliosno(stdin),&state);
	screenx = state.Time;
#ifdef  __HELIOSARM
	screeny = state.Min - 1;
#else
	screeny = state.Min;
#endif
}

void display_screen0(void)
{
	clrscrn();
	printf("Volume:                   Helios File System Monitor\r\n\n");
	printf("Disk Usage :\n\r------------\r\n");
	poscurs(0,4);	printf("UFS Reads  :");
	poscurs(40,4);  printf("UFS Writes   :");
	poscurs(0,5);	printf("Disk Reads :");
	poscurs(40,5);  printf("Disk Writes  :");
	poscurs(0,6);	printf("Cache Hits :");
	poscurs(40,6);	printf("Cache Misses :");
	poscurs(0,7);	printf("Used       :");
	poscurs(40,7);	printf("Free         :");
	printf("\r\n\nMemory Usage :                 Size (bytes)                    Calls\r\n");
	printf("--------------               Current      Maximum       Current        Total\r\n");
	printf("Root Mount Structure\r\nDisk Mount Structure\r\nSuperblocks\r\n");
	printf("Buffer Headers\r\nBuffer Cache\r\nName Table Lookup\r\n");
	printf("Cached inodes\r\nSecurity\r\nTemporary Buffers\r\n");
	printf("Helios Clients\r\n");
	printf("============================================================================\r\n");
	printf("Total\r\n");
	poscurs(0,23);	printf("+/- Sample rate");
	poscurs(70,23);	printf("q=quit");
	poscurs(24,23);	printf("<space>=Toggle to Disk Parameters");
	poscurs(40,23);
}

void display_screen1(void)
{
	clrscrn();
	printf("Volume:                   Helios File System Monitor\r\n\n");

	printf("Disk Data\n\r-----------\n\r");
	poscurs(0,4);	printf("Disk block size :");
	poscurs(0,5);	printf("Fragment size   :");
	poscurs(40,4);  printf("Total blocks     :");
	poscurs(40,5);  printf("Free Blocks      :");
	poscurs(40,6);	printf("Available blocks :");

	poscurs(0,8);	printf("Cached inodes     :");
	poscurs(0,9);	printf("Inode cache limit :");
	poscurs(40,8);	printf("Total inodes     :");
	poscurs(40,9);	printf("Free inodes      :");
	poscurs(0,11);	printf("Cache Limit       :");
	poscurs(40,11);	printf("Buffer Limit     :");

	poscurs(0,13);	printf("Helios usage\n\r------------\n\r");
	printf("Current clients :\n\rMaximum clients :\n\rTotal clients   :");
	poscurs(40,15);	printf("Bytes/client    :");
	poscurs(0,23);	printf("+/- Sample rate");
	poscurs(70,23);	printf("q=quit");
	poscurs(24,23);	printf("<space>=Toggle to Open Files");
	poscurs(40,23);
}

void display_screen2(void)
{
	clrscrn();
	printf("Volume:                   Helios File System Monitor\r\n\n");

	printf("Open Files\n\r----------\n\n\r");
	printf("     ID      Name\n\r");
	poscurs(0,23);	printf("+/- Sample rate");
	poscurs(70,23);	printf("q=quit");
	poscurs(24,23);	printf("<space>=Toggle to Disk Usage");
	poscurs(40,23);
}

void print_kmem(struct kmemstats *kmem)
{
	printf("%10d   %10d    %10d   %10d",
		kmem->ks_memuse,kmem->ks_maxused,kmem->ks_inuse,kmem->ks_calls);
}

void display_data0(void)
{	struct kmemstats ktotal;
	int i;

	poscurs(7,0);	printf("%s",volname);
	poscurs(12,4);	printf("%10d",total_reads);
	poscurs(57,4);	printf("%10d",total_writes);
	poscurs(12,5);	printf("%10d",actual_reads);
	poscurs(57,5);	printf("%10d",actual_writes);
	poscurs(12,6);	printf("%10d (%2d%%)",bufcachehits,(bufcachehits * 100) / (bufcachemiss + bufcachehits));
	poscurs(57,6);	printf("%10d",bufcachemiss);
	poscurs(12,7);	printf("%10d bytes",buf.f_fsize * (buf.f_blocks-buf.f_bfree));
	poscurs(57,7);	printf("%10d bytes",buf.f_fsize * buf.f_bavail);
	
	/* Display Memory Usage */
	poscurs(23,11);		print_kmem(&kmemstats[M_UFSMNT]);
	poscurs(23,12);		print_kmem(&kmemstats[M_MOUNT]);
	poscurs(23,13);		print_kmem(&kmemstats[M_SUPERBLK]);
	poscurs(23,14);		print_kmem(&kmemstats[M_MBUF]);
	poscurs(23,15);		print_kmem(&kmemstats[M_CACHE]);
	poscurs(23,16);		print_kmem(&kmemstats[M_NAMEI]);
	poscurs(23,17);		print_kmem(&kmemstats[M_VNODE]);
	poscurs(23,18);		print_kmem(&kmemstats[M_CRED]);
	poscurs(23,19);		print_kmem(&kmemstats[M_TEMP]);
	
	/* Display Helios Usage */
	poscurs(23,20);
	printf("%10d   %10d    %10d   %10d",
		current_users*stack_size,max_users*stack_size,current_users,user_calls);
	
	/* Calculate the totals */
	ktotal.ks_inuse = 0;
	ktotal.ks_calls = 0;
	ktotal.ks_memuse = 0;
	ktotal.ks_maxused = 0;
	for (i=0; i<M_LAST; i++) {
		ktotal.ks_inuse += kmemstats[i].ks_inuse;
		ktotal.ks_calls += kmemstats[i].ks_calls;
		ktotal.ks_memuse += kmemstats[i].ks_memuse;
		ktotal.ks_maxused += kmemstats[i].ks_maxused;
	}

	/* Add on helios contribution */	
	ktotal.ks_inuse += current_users;
	ktotal.ks_calls += user_calls;
	ktotal.ks_memuse += current_users*stack_size;
	ktotal.ks_maxused += max_users*stack_size;

	/* Display the totals */
	poscurs(23,22);		print_kmem(&ktotal);
	poscurs(40,23);
}

void display_data1(void)
{
	poscurs(7,0);	printf("%s",volname);
	poscurs(17,4);	printf("%10d",buf.f_bsize);
	poscurs(17,5);	printf("%10d",buf.f_fsize);
	poscurs(58,4);	printf("%10d",buf.f_blocks);
	poscurs(58,5);	printf("%10d",buf.f_bfree);
	poscurs(58,6);	printf("%10d",buf.f_bavail);

	poscurs(19,8);	printf("%10d",numvnodes);
	poscurs(19,9);	printf("%10d",desiredvnodes);
	poscurs(58,8);	printf("%10d",buf.f_files);
	poscurs(58,9);	printf("%10d",buf.f_ffree);
	poscurs(19,11);	printf("%10d",kmemstats[M_CACHE].ks_limit);
	poscurs(58,11);	printf("%10d",kmemstats[M_MBUF].ks_limit);

	poscurs(17,15);	printf("%10d",current_users);
	poscurs(17,16);	printf("%10d",max_users);
	poscurs(17,17);	printf("%10d",user_calls);
	poscurs(58,15);	printf("%10d",stack_size+client_size+msg_size);
	poscurs(40,23);
}

#define NLINES 15
void display_data2()
{
	int last, offset;
	struct ufs_open_file *uof;
	struct ufs_open_file data;
	
	last = 0;
	offset = 0;
	poscurs(7,0);	printf("%s",volname);
	poscurs(0,6);
	while ((last < NLINES) && (offset < OF_Marker)) {
		clreol();
		uof = (struct ufs_open_file *)&OF_Buffer[offset];
		memcpy(&data,uof,sizeof(data));
		printf("%8d   %s\n\r",data.pid,data.name);
		offset += data.len;
		last++;
	}
	while (last < NLINES) {
		clreol();
		printf("\n\r");
		last++;
	}
}

void display_screen(int display)
{
	switch (display)
	{
	case 0 : display_screen0();	break;
	case 1 : display_screen1();	break;
	case 2 : display_screen2();	break;
	}
}

void display_data(int display)
{
	switch (display)
	{
	case 0 : display_data0();	break;
	case 1 : display_data1();	break;
	case 2 : display_data2();	break;
	}
}

int main ( int argc, char *argv[] )
{
	int c, i, j;
	int oldc = '?';
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;
	int curtimeout;
  	int display;
	byte *temp;
  	int FID_Close;
  	
    	/* Check args for plausibility	*/
	if (argc == 1) {
	 	fprintf (stderr, "Usage : %s [-c FID] <pathname to fileserver>\n",
 			 argv[0] );
	 	return 1;
	}
	FID_Close = 0;
	volname = NULL;
	for (i=1; i<argc; i++) {
		if (!strcmp(argv[i],"-c")) {
			FID_Close = atoi(argv[++i]);
			continue;
		}
		if (volname == NULL) {
			volname = argv[i];
			continue;
		}
	 	fprintf (stderr, "%s : Further argument %s ignored !\n",
 			 argv[0], argv[i]);
 	}
	if (volname == NULL) {
	 	fprintf (stderr, "Usage : %s [-c FID] <pathname to fileserver>\n",
 			 argv[0] );
	 	return 1;
	}

	/* Get A Reply Port */
 	reply = NewPort ();	
	
	/* Get buffer space for open files */
	OF_BSize = 4096;		/* 4K Buffer Space */
	OF_Buffer = Malloc(OF_BSize);
	
	if (FID_Close) {
		m.Control = Control_V;
		m.Data    = Data_V; 	   
		InitMCB (&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
			FC_GSP | SS_HardDisk | FG_Private | FO_KillOpen);

		m.Timeout = OneSec * 10;
		MarshalCommon (&m, Null(Object), volname);
		MarshalWord (&m, FID_Close);
 
		/* Send the message to the server*/
		e = PutMsg(&m);
		if (e != Err_Null) {
			fprintf(stderr, "%s : Can't send kill message to server :%x\n",
 				argv[0], e);
			FreePort(reply);
			return e;
		}
 	
		/* Expect status response from the ufs file-server. */
		InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
		m.Timeout = OneSec * 10;
		e = GetMsg(&m);
		if (e < 0) {
			fprintf(stderr, "%s : Failed to force file %d closure,  Error %x\n\r",
 				argv[0], FID_Close, e);
 		} else {
			fprintf(stderr, "%s : File %s (%d) closed\n",argv[0],&Data_V,FID_Close);
 		}		

		FreePort(reply);
 		return e;
	}
	
	/* Prepare MCB for marshalling */
	ansiopen();
	getxy();
	oldc = c = 'h';

	display = 0;
	display_screen(display);
	curtimeout = OneSec * 5;

	forever { 					
		/*
		 * Get memory statistics
		 */
		m.Control = Control_V;
		m.Data    = Data_V; 	   
		InitMCB (&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
			FC_GSP | SS_HardDisk | FG_Private | FO_Memstat);

		m.Timeout = OneSec * 10;
		MarshalCommon ( &m, Null(Object), volname );          
 
		/* Send the message to the server*/
		e = PutMsg(&m);
		if (e != Err_Null) {
			fprintf(stderr, "%s : Can't send message to server :%x\n",
 				argv[0], e);
			break;
		}
 	
		/* Expect status response from the ufs file-server. */
		InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
		m.Timeout = OneSec * 10;
		e = GetMsg(&m);
		if (e < 0) break;
			
		memcpy(&kmemstats,&Data_V,sizeof(kmemstats));

		/*
		 * Get disk statistics
		 */
		m.Control = Control_V;
		m.Data    = Data_V; 	   
		InitMCB (&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
			FC_GSP | SS_HardDisk | FG_Private | FO_Diskstat);

		m.Timeout = OneSec * 10;
		MarshalCommon ( &m, Null(Object), volname );          
 
		/* Send the message to the server*/
		e = PutMsg(&m);
		if (e != Err_Null) {
			fprintf(stderr, "%s : Can't send message to server :%x\n",
 				argv[0], e);
			break;
		}
 	
		/* Expect status response from the ufs file-server. */
		InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
		m.Timeout = OneSec * 10;
		e = GetMsg(&m);
		if (e < 0) break;

		/* Extract the data */
		i = 0;
		j = sizeof(u_int);
		memcpy(&current_users,&Data_V[i],j);
		i += j;
		memcpy(&max_users,&Data_V[i],j);
		i += j;
		memcpy(&user_calls,&Data_V[i],j);
		i += j;
		memcpy(&stack_size,&Data_V[i],j);
		i += j;
		memcpy(&client_size,&Data_V[i],j);
		i += j;
		memcpy(&msg_size,&Data_V[i],j);
		i += j;
		memcpy(&desiredvnodes,&Data_V[i],j);
		i += j;
		memcpy(&numvnodes,&Data_V[i],j);
		i += j;
		memcpy(&total_reads,&Data_V[i],j);
		i += j;
		memcpy(&actual_reads,&Data_V[i],j);
		i += j;
		memcpy(&total_writes,&Data_V[i],j);
		i += j;
		memcpy(&actual_writes,&Data_V[i],j);
		i += j;
		memcpy(&bufcachehits,&Data_V[i],j);
		i += j;
		memcpy(&bufcachemiss,&Data_V[i],j);
		i += j;
		memcpy(&buf,&Data_V[i],sizeof(buf));
		
		/*
		 * Get open files
		 */
		m.Control = Control_V;
		m.Data    = Data_V; 	   
		InitMCB (&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
			FC_GSP | SS_HardDisk | FG_Private | FO_GetOpen);

		m.Timeout = OneSec * 10;
		MarshalCommon ( &m, Null(Object), volname );          
 
		/* Send the message to the server*/
		e = PutMsg(&m);
		if (e != Err_Null) {
			fprintf(stderr, "%s : Can't send message to server :%x\n",
 				argv[0], e);
			break;
		}
 	
		/* Get the list of open files */
		OF_Marker = 0;
		do {
			/* Expect status response from the ufs file-server. */
			InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
			m.Timeout = OneSec * 10;
			e = GetMsg(&m);
			if (e < 0) break;

			/* Enough space, or grow buffer */
			if (OF_Marker + m.MsgHdr.DataSize > OF_BSize) {
				/* Not enough room, so extend */
				temp = Malloc(OF_BSize*2);
				memcpy(temp,OF_Buffer,OF_Marker);
				Free(OF_Buffer);
				OF_Buffer = temp;
			}

			/* Shuffle the data into the big array */
			memcpy(OF_Buffer+OF_Marker,&Data_V,m.MsgHdr.DataSize);
			OF_Marker += m.MsgHdr.DataSize;
		} while ((m.MsgHdr.FnRc & ReadRc_EOD) != ReadRc_EOD);

		/*
		 * Display the data.
		 */
		display_data(display);
		
		while (c = getch(curtimeout)) {
			switch( c )
			{
			case '+' : {
					curtimeout = curtimeout - OneSec / 2;
					if (curtimeout < OneSec / 2)
						curtimeout = OneSec / 2;
					break;
				    }
			case '-' : {
					curtimeout = curtimeout + OneSec / 2;
					if (curtimeout > OneSec * 9)
						curtimeout = OneSec * 9;
					break;
				    }
			case ' ':   {
					display = ++display % 3;
					display_screen(display);
					display_data(display);
					break;
				    }
			case 'Q':
			case 'q':
			case 0x1B: 
			case 0x3:
				goto done;
			}
		}
	}

	poscurs(0,23);	printf("\r\nFile server not found or disappeared   \n\r");	

done:	
	Free(OF_Buffer);
	FreePort(reply);
	poscurs(0,screeny);
	ansiclose();
	putchar('\n');
}

