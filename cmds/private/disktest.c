/************************************************************************/
/*									*/
/* Disk Speed Test Program v1.1						*/
/*									*/
/* Copyright (C) Perihelion Software 1989				*/
/*									*/
/*									*/
/* PAB 10/5/89			 					*/
/* V1.1 18/5/89	Unbuffered IO and UNIX compatibility			*/
/*									*/
/* Usage: dt [numblocks blocksize]					*/
/*									*/
/************************************************************************/

#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/private/RCS/disktest.c,v 1.5 1994/03/08 12:45:13 nickc Exp $";
#endif

#include <stdio.h>

#define STDBUFFER 0	/* use our own buffering or std c lib buffering */

#ifdef __STDC__		/* __STDC__ = ANSI C, else UNIX and K&R */
#include <stdlib.h>
#include <time.h>
#else
char *malloc();
#define remove unlink

#include <time.h>
#define CLK_TCK 1000000	 /* Usecs */

int clock()
{
	struct timeval t;
	struct timezone tz;

	gettimeofday(&t, &tz);

	return(t.tv_sec * 1000000 + t.tv_usec);
}
#endif

#define DEF_NBLOCKS 10

#if STDBUFFER
#define DEF_BLKSIZE BUFSIZ/1024
#else
#define DEF_BLKSIZE 32
#endif

#ifndef __ARM
#define USE_FP
#endif

int
main(argc, argv)
int argc;
char **argv;
{
	int	opentime,  writetime, readtime, seektime, closetime, deltime;
	int	starttime, i, nblocks, blksize;
#ifdef USE_FP
	double	ftick;
#endif
	
	FILE *  fp;

#ifdef __STDC__	
	void *	tmparea;
	void *	buffarea;
#else
	char *	tmparea;
	char *	buffarea;
#endif

	if (argc != 3)
	{
		nblocks = DEF_NBLOCKS;
		blksize = DEF_BLKSIZE * 1024;
	}
	else
	{
		nblocks = atoi(argv[1]);
#if STDBUFFER
		blksize = BUFSIZ;
#else
		blksize = atoi(argv[2]) * 1024;
#endif
	}

	if (blksize == 0 || nblocks == 0 || blksize * nblocks > 2000*1024)
	{
		printf("ERROR invalid number of blocks or block size - aborting\n");
		exit(1);
	}
	
	remove( "speedtst.tmp" );

	printf("Helios Disk Speed Test Program v1.1");
	
#ifdef __STDC__
printf(" (ANSI)\n");
#else
printf(" (UNIX)\n");
#endif
	printf("------------------------------------------\n\n");

	printf("Test based on opening file, writing %d %dk blocks,\n", nblocks, blksize/1024);
	printf("seeking back to the start, reading %d %dk blocks,\n", nblocks, blksize/1024);
	printf("closing and then deleting the file.\n");
#if STDBUFFER
	printf("Uses only the C libraries standard buffer.\n");
#endif

	printf("\nOpening. "); fflush(stdout);
	
	starttime = clock();
	if ((fp = fopen("speedtst.tmp", "wb+")) == NULL)
	{
		printf("ERROR opening test file - aborting\n");
		exit(1);
	}
	opentime = clock() - starttime;

#if !STDBUFFER
	if ((buffarea = malloc(blksize)) == NULL)
	{
		printf("ERROR cannot get memory for buffer - aborting\n");
		remove("speedtst.tmp");
		exit(1);
	}
#endif
	
#if __STDC__
#if !STDBUFFER
	setvbuf(fp, (char *)buffarea, _IOFBF, blksize);
#endif
#else
#if !STDBUFFER
	setbuffer(fp, buffarea, blksize);
#endif
#endif
	printf("Writing. "); fflush(stdout);
	
	starttime = clock();
	for (i=0; i < nblocks; i++)
	  fwrite("Gobbledygook", blksize, 1, fp);
	fflush(fp);
	/* wait until the last block is actually written */
	writetime = clock() - starttime;

	printf("Seeking. "); fflush(stdout);
	
	starttime = clock();
	fseek(fp, 0, 0);
	putc('x',fp);		/* make sure it really seeks */
	seektime = clock() - starttime;
	fseek(fp, 0, 0);
	
	printf("Reading. "); fflush(stdout);
	
	tmparea = (char *)malloc(blksize);
	starttime = clock();
	for (i=0; i < nblocks; i++)
	  (void) fread(tmparea, blksize, 1, fp);
	readtime = clock() - starttime;

	free(tmparea);

	printf("Closing. "); fflush(stdout);
	
	starttime = clock();
	fclose(fp);
	closetime = clock() - starttime;

	printf("Deleting. "); fflush(stdout);
	
	starttime = clock();
	remove("speedtst.tmp");
	deltime = clock() - starttime;
	
#if 0
	printf("open = %5.2f, write = %5.2f seek = %5.2f read = %5.2f close = %5.2f del = %5.2f\n",opentime,writetime,seektime,readtime,closetime,deltime); 
	printf("CLK_TCK = %d\n",CLK_TCK);
#endif
#ifdef USE_FP
	ftick = (double)CLK_TCK;
#endif
	
	nblocks *= blksize / 1024;
		
	printf("Finished.\n\n");
	printf("Elapsed times (in seconds):\n");
#ifdef USE_FP
	printf("Create and open %10.2f\n", opentime / ftick );
	printf("Write %4dk     %10.2f\n",  nblocks, writetime / ftick );
	printf("Seek %4dk      %10.2f\n",  nblocks, seektime  / ftick );
	printf("Read %4dk      %10.2f\n",  nblocks, readtime  / ftick );
	printf("Close file      %10.2f\n", closetime / ftick );
	printf("Delete file     %10.2f\n", deltime   / ftick );

	printf("\nTotal time      %10.2f\n\n",
	       (opentime + writetime + seektime + readtime + closetime + deltime) / ftick );

	printf("Transfer rate per second:\n");

	printf("Read             %10.3fk\n", nblocks * ftick / readtime  );
	printf("Write            %10.3fk\n", nblocks * ftick / writetime );
#else
	printf("Create and open %10d\n", opentime / CLK_TCK );
	printf("Write %4dk     %10d\n",  nblocks, writetime / CLK_TCK );
	printf("Seek %4dk      %10d\n",  nblocks, seektime  / CLK_TCK );
	printf("Read %4dk      %10d\n",  nblocks, readtime  / CLK_TCK );
	printf("Close file      %10d\n", closetime / CLK_TCK );
	printf("Delete file     %10d\n", deltime   / CLK_TCK );

	printf("\nTotal time      %10d\n\n",
	       (opentime + writetime + seektime + readtime + closetime + deltime) / CLK_TCK );

	printf("Transfer rate per second:\n");

	printf("Read             %10dk\n", nblocks * CLK_TCK / readtime  );
	printf("Write            %10dk\n", nblocks * CLK_TCK / writetime );
#endif
	
	return(0);
}
