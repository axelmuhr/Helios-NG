/*
 * b2.c message bounce test program bounces messages between two threads on
 * same processor.
 *
 * P.A.Beskeen
 * May 21, '92
 */

#include <helios.h>
#include <stdio.h>
#include <stdlib.h>
#include <nonansi.h>
#include <codes.h>
#include <time.h>

#ifdef __C40
# include "../../kernel/gexec.h"	/* CHEAT!*/
#endif

int msgsize, bounces;

void echo(Port rxport)
{
	MCB m;
	unsigned int i;
	word e = 0;
	byte *buf = (byte *)Malloc(msgsize);
#ifdef __C40
	ExecRoot *xr = GetExecRoot();

	IOdebug("echo : &mcb %a, ss = %a", &m, xr->CurrentSaveArea);
#endif
	m.Data = buf;

	for( i = 1; i <= bounces ; i++ ) {
		if (e == 0 || ((e & EG_Mask) != EG_Timeout)) {
			m.MsgHdr.Dest = rxport;
			m.Timeout = OneSec*5;

			e = GetMsg(&m);
			if (e != 2) {
				IOdebug("echo: %x = GetMsg() cnt %d", e,i);
#ifdef __C40
				/* dbg just loop while yielding */
				forever {
					word *x = (word *)Malloc(1);
					Free(x);
				}
#else
				exit(1);
#endif
			}
			if (m.MsgHdr.Reply == 0)
				IOdebug("echo: GetMsg ReplyPort %x cnt = %d", e, m.MsgHdr.Reply, i);
		}

		InitMCB(&m,0,m.MsgHdr.Reply,NullPort,1);
		*(int *)&m |= msgsize;

		e = PutMsg(&m);
		if (e != 0) {
			IOdebug("echo: %x = PutMsg() txport %x cnt = %d", e, m.MsgHdr.Dest, i);
#ifdef __C40
			/* dbg just loop while yielding */
			forever {
				word *x = (word *)Malloc(1);
				Free(x);
			}
#else
				exit(1);
#endif
		}
	}

	Free(buf);
}

void bounce(Port txport, Port rxport)
{
	unsigned int i;
	MCB m;
	int start , end;
	unsigned int total;
	word e = 0;
	byte *buf = (byte *)Malloc(msgsize);
#ifdef __C40
	ExecRoot *xr = GetExecRoot();

	IOdebug("bounce: &mcb %a, ss = %a", &m, xr->CurrentSaveArea);
#endif
	m.Data = buf;
	start = clock();
	for( i = 1; i <= bounces ; i++ ) {
		if (e == 0 || ((e & EG_Mask) != EG_Timeout)) {
			InitMCB(&m,MsgHdr_Flags_preserve,txport,rxport,2);
			m.Timeout = OneSec*5;
			*(int *)&m |= msgsize;

			e = PutMsg(&m);
			if (e != 0) {
				IOdebug("bounce: %x = PutMsg() txport %x cnt = %d", e, txport, i);
#ifdef __C40
				/* dbg just loop while yielding */
				forever {
					word *x = (word *)Malloc(1);
					Free(x);
				}
#else
				exit(1);
#endif
			}
		}

		m.MsgHdr.Dest = rxport;
		e=GetMsg(&m);
		if (e != 1) {
			IOdebug("bounce: %x = GetMsg cnt = %d", e, i);
#ifdef __C40
			forever {
				/* dbg just loop while yielding */
				word *x = (word *)Malloc(1);
				Free(x);
			}
#else
				exit(1);
#endif
		}
	}
	end = clock();

	total=end-start;

	printf("%u microseconds for %u bounces of a %u byte message\n",
		total*10000,bounces,msgsize);

	printf("%u microseconds per message\n",(total*10000)/(bounces*2));

	printf("%u bytes per second\n",	(msgsize*2*bounces*100)/total);

	Free(buf);
}

void usage(char *name) {
	fprintf(stderr,"usage: %s [msgsize [bounces]]\n",name);
	exit(1);
}

int main(int argc, char **argv)
{
	Port txport, rxport;

	if (argc < 2) {
		msgsize = 512;
		bounces = 10000;
	} else if (argc == 2) {
		if ((msgsize = atoi(argv[1])) == 0) {
			fprintf(stderr,"invalid msg data size %s\n",argv[1]);
			usage(*argv);
		}
	} else if (argc == 3) {
		if ((msgsize = atoi(argv[1])) == 0) {
			fprintf(stderr,"invalid msg data size %s\n", argv[1]);
			usage(*argv);
		}
		if ((bounces = atoi(argv[2])) == 0) {
			fprintf(stderr,"invalid number of bounces %s \n", argv[2]);
			usage(*argv);
		}
	} else
		usage(*argv);
		
	txport = NewPort();
	rxport = NewPort();

	printf("Started bouncing %d messages of %d size...\n", bounces, msgsize);
#ifdef __C40
	printf("(ports are tx %lx, rx %lx)\n", txport, rxport);
#endif
	Fork(4096, echo, 4, txport);

	bounce(txport, rxport);
}


/* end of b2.c */
