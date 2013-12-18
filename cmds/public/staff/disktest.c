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

#include <stdio.h>

#define STDBUFFER 0	/* use our own buffering or std c lib buffering */

#ifdef __STDC__		/* __STDC__ = ANSI C, else UNIX and K&R */
#include <stdlib.h>
#include <time.h>
#else
char *malloc();
#define remove unlink

#include <sys/time.h>
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

int
main(argc, argv)
int argc;
char **argv;
{
	float	opentime, writetime, readtime, seektime, closetime, deltime;
	int	starttime, endtime, i, nblocks, blksize;
	FILE *fp;

#ifdef __STDC__	
void *tmparea, *buffarea;
#else
char *tmparea, *buffarea;
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
	remove("speedtst.tmp");

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
	endtime = clock();
	opentime = endtime - starttime;

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
	setvbuf(fp, buffarea, _IOFBF, blksize);
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
	fflush(fp);	/* wait until the last block is actually written */
	endtime = clock();
	writetime = endtime - starttime;

	printf("Seeking. "); fflush(stdout);
	starttime = clock();
	fseek(fp, 0, 0);
	putc('x',fp);		/* make sure it really seeks */
	endtime = clock();
	seektime = endtime - starttime;
	fseek(fp, 0, 0);
	
	printf("Reading. "); fflush(stdout);
	tmparea = malloc(blksize);
	starttime = clock();
	for (i=0; i < nblocks; i++)
		fread(tmparea, blksize, 1, fp);
	endtime = clock();
	free(tmparea);
	readtime = endtime - starttime;

	printf("Closing. "); fflush(stdout);
	starttime = clock();
	fclose(fp);
	endtime = clock();
	closetime = endtime - starttime;

	printf("Deleting. "); fflush(stdout);
	starttime = clock();
	remove("speedtst.tmp");
	endtime = clock();
	deltime = endtime - starttime;
	
	printf("Finished.\n\n");
	printf("Elapsed times (in seconds):\n");
	printf("Create and open %10.2f\n", opentime / CLK_TCK);
	printf("Write %4dk     %10.2f\n", nblocks*blksize/1024, writetime / CLK_TCK);
	printf("Seek %4dk      %10.2f\n", nblocks*blksize/1024, seektime / CLK_TCK);
	printf("Read %4dk      %10.2f\n", nblocks*blksize/1024, readtime / CLK_TCK);
	printf("Close file      %10.2f\n", closetime / CLK_TCK);
	printf("Delete file     %10.2f\n", deltime / CLK_TCK);

	printf("\nTotal time      %10.2f\n\n",
	    (opentime+writetime+seektime+readtime+closetime+deltime) / CLK_TCK);

	printf("Transfer rate per second:\n");
	if (readtime/CLK_TCK > 0)
		printf("Read             %10.3fk\n", (nblocks * blksize) / (readtime / CLK_TCK) / 1024 );
	else
		printf("Read             Instantaneous!\n");
	if (writetime/CLK_TCK > 0)
		printf("Write            %10.3fk\n", (nblocks * blksize) / (writetime / CLK_TCK) / 1024);
	else
		printf("Write            Instantaneous!\n");

	return(0);
}
