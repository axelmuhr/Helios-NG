/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S			                --
--                     -----------                                      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- timeio.c								--
--                                                                      --
--	Author:  BLV 22/3/90						--
--	Rewritten: BLV 2.12.93						--
--                                                                      --
------------------------------------------------------------------------*/
static char *rcsid = "$Header: /hsrc/cmds/private/RCS/timeio.c,v 1.4 1993/02/22 18:34:18 bart Exp $";


#if 1
/*{{{  old code */
#include <helios.h>
#include <stdio.h>
#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>
#include <nonansi.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

/**
*** This program opens a stream to /logger in the  I/O Server and
*** bounces private protocol messages of it. These protocol messages
*** take the following function codes :
*** 0 FC_Private request control_size data_size
*** request = 1, for simple bounce
***           2, return message header only
***           3, send reply message with the specified size
**/

extern void _Trace(int, int, int);

#define LL_Bounce (word)(1 << 24)
#define LL_Empty  (word)(2 << 24)
#define LL_Fill   (word)(3 << 24)

static  Stream  *connect_to_server(string);
static  void     message_handler(Stream *, word loops, word type, word size);

static void usage(void)
{
    fputs("timeio: usage, timeio <bounces>\n", stderr);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    Stream  *my_stream;
    word     loops      = 100;

    if (argc != 1)
    {
        if (argc > 2) usage();
        loops   = atoi(argv[1]);
        if (loops == 0) usage();
    }

    printf("In new version of timeio\n");
    _Trace(1, 2, 3);
    
        my_stream = connect_to_server("/logger");
    printf("Timeio : times are for %ld bounces.\n", loops);

    message_handler(my_stream, loops, LL_Bounce,     0);
    _Trace(1, 2, 4);
    putchar('\n');
    _Trace(1, 2, 5);
    message_handler(my_stream, loops, LL_Empty,     64);
    message_handler(my_stream, loops, LL_Fill,      64);
    message_handler(my_stream, loops, LL_Bounce,    64);
    putchar('\n');
    message_handler(my_stream, loops, LL_Empty,   1024);
    message_handler(my_stream, loops, LL_Fill,    1024);
    message_handler(my_stream, loops, LL_Bounce,  1024);
    putchar('\n');
    message_handler(my_stream, loops, LL_Empty,  32768);
    message_handler(my_stream, loops, LL_Fill,   32768);
    message_handler(my_stream, loops, LL_Bounce, 32768);

    Close(my_stream);
    return(EXIT_SUCCESS);
}

static void message_handler(Stream *my_stream, word loops, word type, word size)
{
    static byte     data_vec[0x0FFFF];
    static word     word_vec[256];
    MCB             mcb;
    word            i;
    clock_t         timer;
    Port            reply_port  = NewPort();;

    if (reply_port == NullPort)
    {
        fputs("timeio: failed to get new message port.\n", stderr);
        exit(EXIT_FAILURE);
    }
    
    mcb.Control = word_vec;
    mcb.Data    = data_vec;

    timer = clock();
     
    for (i = 0; i < loops; i++)
    {
        mcb.MsgHdr.Dest          = my_stream->Server;
        mcb.MsgHdr.Reply         = reply_port;
        mcb.Timeout              = -1;
        mcb.MsgHdr.ContSize      =  0;
        if ((type == LL_Empty) || (type == LL_Bounce))
            mcb.MsgHdr.DataSize  = (unsigned short) size;
        else
            mcb.MsgHdr.DataSize  = 0;
        mcb.MsgHdr.Flags         = MsgHdr_Flags_preserve;
        mcb.MsgHdr.FnRc          = FC_Private + type;
        if ((type == LL_Fill) || (type == LL_Bounce))
            mcb.MsgHdr.FnRc     |= size;
     
        PutMsg(&mcb);

        mcb.MsgHdr.Dest  = mcb.MsgHdr.Reply;
        mcb.Timeout      = -1;

        GetMsg(&mcb);     
    }

    timer = clock() - timer;
    FreePort(reply_port);

    if (size == 0)
    {
#if 0
      printf("Message header only : %6.2f seconds\n",
            (double) timer / (double) CLK_TCK);
#else
      printf("Message header only: %d ticks\n", timer);
#endif
      
    }
    else
    {
        word    rate;
/*
printf("loops %ld, size %ld, timer %d\n", loops, size, timer);
*/
        if (timer == 0)
            rate = INT_MAX;
        else
        {
            rate = (loops * size * CLK_TCK) / (timer * 1024L);
            if (type == LL_Bounce)
                rate *= 2;
        }

#if 0        
        printf("Header + %5ld bytes, %s : %6.2f seconds, %4ld KBytes/second\n",
                size,
                (type == LL_Empty) ? "-> I/O   " :
                (type == LL_Fill)  ? "<- I/O   " :
                                     "both ways",
                (double) timer / (double) CLK_TCK,
                rate);
#else
	printf("Header + %5ld bytes, %s : %d ticks, %4ld KBytes/seconds\n",
                size,
                (type == LL_Empty) ? "-> I/O   " :
                (type == LL_Fill)  ? "<- I/O   " :
                                     "both ways",
                timer,
                rate);
#endif	
	       
    }
}

static Stream *connect_to_server(string name)
{
    Object *server;
    Stream *stream;

    server = Locate(NULL, name);
    if (server == Null(Object))
    {
        fprintf(stderr, "timeio: failed to locate %s\n", name);
        exit(EXIT_FAILURE);
    }

    stream = Open(server, Null(char), O_ReadWrite);
    if (stream == Null(Stream))
    {
        fprintf(stderr, "timeio: failed to Open %s\n", server->Name);
        Close(server);
        exit(EXIT_FAILURE);
    }
   
    Close(server);
    return(stream);
}


/*}}}*/
#else
/*{{{  header files */
#include <helios.h>
#include <syslib.h>    
#include <message.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>    

#include <rmlib.h>
#include <farmlib.h>    
/*}}}*/
/*{{{  data structures and statics */

				/* This structure defines a "test", some number of	*/
				/* message bounces of a suitable size and direction.	*/
typedef struct TimeioTest 
{
    int		Count;		/* of message bounces					*/
    int		Size;		/* of the messages					*/
    char	Direction;	/* one of 'i', 'o' or 'b'				*/
} TimeioTest;

				/* This describes the results of a test, including	*/
				/* timings and any corruption detected.			*/
typedef struct TimeioResult 
{
    struct timeval	Start;			/* Start time, using gettimeofday()	*/
    struct timeval	End;			/* Finishing time.			*/
    int			InputCorruptions;	/* Detected by I/O Server		*/
    int			OutputCorruptions;	/* Detected by timeio			*/
} TimeioResult;

				/* This table holds the default set of tests.		*/
static TimeioTest DefaultTests[] = 
{
    {	100,	   64,	'i' },
    {	100,	   64,	'o' },
    {	100,	   64,	'b' },
    {	100,	 1024,	'i' },
    {	100,	 1024,	'o' },
    {	100,	 1024,	'b' },
    {	100,	32768,	'i' },
    {	100,	32768,	'o' },
    {	100,	32768,	'b' },
    {	 -1,	   -1,	' ' }
};

				/* This array is filled in with an alternative set	*/
				/* of tests if the user so specifies. There is a	*/
				/* counter for the next free slot. If this is still 0	*/
				/* after processing the arguments then the default	*/
				/* tests should be used.				*/
#define MaxTests	64
static TimeioTest	OtherTests[MaxTests+1];		/* allow for -1 terminator	*/
static int		NextTest		= 0;

				/* Checksumming is disabled by default.			*/
static bool		Checksumming		= FALSE;

				/* If all of these are -1 then the bounce routine will	*/
				/* use the number of bounces field in the test		*/
				/* structure. Otherwise it will run for the specified	*/
				/* amount of time.					*/
static int		Days			= -1;
static int		Hours			= -1;
static int		Minutes			= -1;

				/* Network flooding is disabled by default.		*/
static bool		FloodNetwork		= FALSE;    

				/* Running on a remote processor is disabled by		*/
				/* default.						*/
static char		*RemoteProcessor	= NULL;

				/* By default only a single thread should perform I/O	*/
static int		NumberOfThreads		= 1;

				/* This Semaphore is used to handle termination if	*/
				/* multiple threads are in use.				*/
static Semaphore	Finished;

#define ThreadStackSize	4096

				/* The way the direction and message count options are	*/
				/* used is fairly subtle. There are statics containing	*/
				/* the current values for direction and count. If a new */
				/* message size is specified then another field in the	*/
				/* OtherTests[] array is allocated and filled in	*/
				/* appropriately. Thus using a single command line it	*/
				/* is possible to specify several different sizes each	*/
				/* with a different direction and count.		*/
				/* 							*/
				/* There is a subtlety. If the command line changes	*/
				/* the direction or the count but does not specify any	*/
				/* sizes then the changed values should affect the	*/
				/* default tests.					*/
#define DefaultCount		100
#define DefaultDirection	'a'
static int		CurrentCount		= DefaultCount;
static char		CurrentDirection	= DefaultDirection;
static int		CurrentSize		= 0;

/*}}}*/
/*{{{  open connection to server */

/*}}}*/
/*{{{  bounce messages */

/*}}}*/
/*{{{  latency test */

/*}}}*/
/*{{{  perform test */
/*{{{  perform_tests1() */
				/* This routine actually performs the appropriate tests.	*/
				/* First it has to work out what the tests should be, including	*/
				/* the buffer size required. This buffer is then allocated, and	*/
				/* the appropriate number of message bounces are performed.	*/
static void perform_tests1(void)
{
    TimeioTest		*tests;
    TimeioResult	 result;
    int		 	 i;
    int			 bufsize;
    int			*buffer;

    if (NextTest == 0)
	tests = DefaultTests;
    else
    {
	tests = OtherTests;
	tests[NextTest].Count	= -1;
    }

    bufsize	= 0;
    for (i = 0; tests[i].Count != -1; i++)
	if (tests[i].Size > bufsize)
	    bufsize = tests[i].Size;

    buffer = (int *) malloc(bufsize);

    for (i = 0; tests[i].Count != -1; i++)
    {
	do_bounces(&(tests[i]), &result, buffer);
	display_results(&(tests[i]), &result);
    }
    
	

    
    

    
	
}


/*}}}*/
    
static void perform_tests(void)
{
				/* This is called from inside main() or from inside the Farm	*/
				/* worker routine. It must establish the connection to the	*/
				/* error logger in the I/O Server, perform the initial latency	*/
				/* test, and then do the actual tests specified by the user.	*/
    connect_to_server();
    latency_test();

    if (NumberThreads > 1)
    {
	int	i;
	
	InitSemaphore(&Finished, 0);
	for (i = 0; i < NumberThreads; i++)
	    unless(Fork(ThreadStackSize, &perform_tests1, 0))
	    {
		fputs("timeio: out of memory Fork()'ing thread\n", stderr);
		exit(EXIT_FAILURE);
	    }
	for (i = 0; i < NumberThreads; i++)
	    Wait(&Finished);
    }
    else
    {
	perform_tests1();
    }

    shutdown_connection();
}
    
/*}}}*/
/*{{{  flood network support */

/*{{{  producer routin */
static void timeio_producer(void)
{
    void	*job;

    job = FmGetJobBuffer(0);
    FmSendJob(Fm_All, TRUE, job);
}
/*}}}*/
/*{{{  consumer routin */
static void timeio_consumer(void)
{
    int		 i;
    void	*reply;
    
    for (i = 0; i < FmNumberWorkers; i++)
	reply	= FmGetReply(i);
}
/*}}}*/
/*{{{  worker routin */
static	void	timeio_worker(void)
{
    void	*job;
    void	*reply;

    job		= FmGetJob();
    reply	= FmGetReplyBuffer(job, 0);

    perform_tests();

    FmSendReply(reply);
}    
/*}}}*/

static int flood_network(void)
{
    FmWorker			= &timeio_worker;
    FmProducer			= &timeio_producer;
    FmConsumer			= &timeio_consumer;
    FmOverloadController	= TRUE;
    
    FmInitialise();

    return(EXIT_SUCCESS);
}

/*}}}*/
/*{{{  remote processor support */

				/* To run timeio remotely I want to invoke the remote	*/
				/* command. This will take care of running in user or	*/
				/* supervisor mode.					*/
static int run_remote(int argc, char **argv)
{
    Environ	 *env		= getenviron();
    char	**new_argv;
    int		  i;
    int		  index;
    int		  pid;
    int		  status;
    
    new_argv	= (char **) malloc(sizeof(char *) * (4 + argc));
    if (new_argv == NULL)
    {
	fprintf(stderr, "timeio: out of memory attempting to migrate to %s\n", RemoteProcessor);
	exit(EXIT_FAILURE);
    }
    new_argv[0]		= "remote";
    new_argv[1]		= RemoteProcessor;
    new_argv[2]		= env->Objv[OV_Code]->Name;
    
				/* Copy the current arguments except -p			*/
    index	= 3;
    for (i = 1; i < argc; i++)
    {
	if ((argv[i][0] == '-') && (argv[i][1] == 'p'))
	{
	    if (argv[i][2] == '\0') i++;	/* skip next argument as well		 */
	    continue;
	}
	else
	{
	    new_argv[index++] = argv[i];
	}
    }

    if ((pid = vfork()) == 0)
    {
	if (execv("/helios/bin/remote", new_argv))
	    _exit(1);
    }
    else
	wait(&status);

    return(status);
}

/*}}}*/
/*{{{  usage() */
static void usage(void)
{
    fputs("timeio: usage, timeio [-c] {-d<dir>} [-f] {-n<count>} [-p<proc>] {-s<size>} [-t<threads>] [time]\n", stderr);
    fputs("        -c             : checksum all data\n", stderr);
    fputs("        -d<dir>        : specify direction of traffic\n", stderr);
    fputs("                -di    : input only, send full messages to the I/O Server and get empty replies\n", stderr);
    fputs("                -do    : output only, send empty messages and get full replies\n", stderr);
    fputs("                -db    : bidirectional traffic only, full messages and replies\n", stderr);
    fputs("                -da    : all traffic, do all of the above tests in sequence\n", stderr);
    fputs("        -f             : flood the network with timeio programs\n", stderr);
    fputs("        -n<count>      : specifies the number of messages to be sent to and from\n", stderr);
    fputs("        -p<proc>       : run onthe specified processor\n", stderr);
    fputs("        -s<size>       : gives the size of messages\n", stderr);
    fputs("        -t<threads>    : spawn the specified number of threads, each bouncing messages\n", stderr);
    fputs("        [time]         : give the duration of the run in days, hours and minutes\n", stderr);
    fputs("\n", stderr);
    fputs("-f and -p are mutually exclusive, as are -n and [time]\n", stderr);
    exit(EXIT_FAILURE);
}
/*}}}*/
/*{{{  main() */
/*{{{  my_atoi() */
				/* This routine is similar to atoi(), i.e. it converts	*/
				/* a string to a number, but it also validates the	*/
				/* number.						*/
static int my_atoi(char *str)
{
    char	*tmp;

    for (tmp = str; *tmp; tmp++)
	unless(isdigit(*tmp))
	{
	    fprintf(stderr, "timeio: invalid argument %s, number expected\n", str);
	    exit(EXIT_FAILURE);
	}

    return(atoi(str));
}

/*}}}*/
    
int main(int argc, char **argv)
{
    int		i;

/*{{{  Process arguments */

    for (i = 1; i < argc; i++)
    {
	if (argv[i][0] == '-')
	{
	    
	    switch(argv[i][1])
	    {
	    case 'b':
		if (argv[i][2] != '\0')
		    CurrentCount = my_atoi(&(argv[i][2]));
		elif (++i == argc)
		    usage();
		else
		    CurrentCount = my_atoi(&(argv[i][0]));
		break;

	    case 'c':
		Checksumming = TRUE;
		break;
		
	    case 'd':
		CurrentDirection = argv[i][2];
		if (isupper(CurrentDirection)) CurrentDirection = tolower(CurrentDirection);
		if ((CurrentDirection != 'i') && (CurrentDirection != 'o') &&
		    (CurrentDirection != 'b') && (CurrentDirection != 'a'))
		    usage();
		break;
		
	    case 'f':
		FloodNetwork	= TRUE;
		break;

	    case 'p':
		if (argv[i][2] != '\0')
		    RemoteProcessor = &(argv[i][2]);
		elif (++i == argc)
		    usage();
		else
		    RemoteProcessor = &(argv[i][0]);
		break;
	
	    case 's':
		if (argv[i][2] != '\0')
		    CurrentSize = my_atoi(&(argv[i][2]));
		elif (++i == argc)
		    usage();
		else
		    CurrentSize	= my_atoi(&(argv[i][0]));

		if (((CurrentDirection == 'a') && ((NextTest + 3) >= MaxTests)) ||
		    ((CurrentDirection != 'a') && ((NextTest + 1) >= MaxTests)))
		{
		    fputs("timeio: sorry, cannot cope with that many tests.\n", stderr);
		    exit(EXIT_FAILURE);
		}

		if (CurrentDirection == 'a')
		{
		    OtherTests[NextTest  ].Count	= CurrentCount;
		    OtherTests[NextTest+1].Count	= CurrentCount;
		    OtherTests[NextTest+2].Count	= CurrentCount;
		    OtherTests[NextTest  ].Size		= CurrentSize;
		    OtherTests[NextTest+1].Size		= CurrentSize;
		    OtherTests[NextTest+2].Size		= CurrentSize;
		    OtherTests[NextTest  ].Direction	= 'i';
		    OtherTests[NextTest+1].Direction	= 'o';
		    OtherTests[NextTest+2].Direction	= 'b';
		    NextTest += 3;
		}
		else
		{
		    OtherTests[NextTest].Count		= CurrentCount;
		    OtherTests[NextTest].Size		= CurrentSize;
		    OtherTests[NextTest].Direction	= CurrentDirection;
		    NextTest += 1;
		}
		
		break;
	
	    case 't':
		if (argv[i][2] != '\0')
		    NumberOfThreads = my_atoi(&(argv[i][2]));
		elif (++i == argc)
		    usage();
		else
		    NumberOfThreads = my_atoi(&(argv[i][0]));
		break;
		
	    default:
		usage();
	    }
	}
	else
	{
	    if (Hours == -1)
		Hours		= Minutes;
	    elif (Days == -1)
	    {
		Days		= Hours;
		Hours		= Minutes;
	    }
	    Minutes = my_atoi(argv[i]);
	}
    }

/*}}}*/

				/* Check for argument conflicts.			 */
    if ((RemoteProcessor != NULL) && (FloodNetwork))
    {
	fputs("timeio: cannot specify a particular processor and flood the network\n", stderr);
	exit(EXIT_FAILURE);
    }
    if ((CurrentCount != DefaultCount) && (Minutes != -1))
    {
	fputs("timeio: cannot specify both an iteration count and a duration\n", stderr);
	exit(EXIT_FAILURE);
    }

				/* If the default sizes should be used but the user	*/
				/* supplied an alternative count or direction then	*/
				/* the default tests should be changed.			*/
    if ((NextTest == 0) && (CurrentCount != DefaultCount) && (CurrentDirection == DefaultDirection))
    {
	for (i = 0; DefaultTests[i].Count != -1; i++)
	    DefaultTests[i].Count	= CurrentCount;
    }
    elif ((NextTest == 0) && (CurrentDirection != DefaultDirection))
    {
	OtherTests[0].Count	= CurrentCount;
	OtherTests[1].Count	= CurrentCount;
	OtherTests[2].Count	= CurrentCount;
	OtherTests[0].Size	= 64;
	OtherTests[1].Size	= 1024;
	OtherTests[0].Size	= 32768;
	OtherTests[0].Direction	= CurrentDirection;
	OtherTests[1].Direction	= CurrentDirection;
	OtherTests[2].Direction = CurrentDirection;
	NextTest = 3;
    }

				/* If the -p option has been used, migrate to the	*/
				/* specified processor.					*/
    if (RemoteProcessor != NULL)
	return(run_remote(argc, argv));
    
				/* If the -f option has been used, switch to the	*/
				/* Farm library.					*/
    if (FloodNetwork)
	return(flood_network());

				/* Now perform the actual tests.			*/
    perform_tests();

    return(EXIT_SUCCESS);
}  

/*}}}*/
#endif

