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
--                                                                      --
------------------------------------------------------------------------*/
static char *rcsid = "$Header: /hsrc/cmds/private/RCS/timeio.c,v 1.4 1993/02/22 18:34:18 bart Exp $";

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

    my_stream = connect_to_server("/logger");
    printf("Timeio : times are for %ld bounces.\n", loops);

    message_handler(my_stream, loops, LL_Bounce,     0);
    putchar('\n');
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
        printf("Message header only : %6.2f seconds\n",
            (double) timer / (double) CLK_TCK);
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
        
        printf("Header + %5ld bytes, %s : %6.2f seconds, %4ld KBytes/second\n",
                size,
                (type == LL_Empty) ? "-> I/O   " :
                (type == LL_Fill)  ? "<- I/O   " :
                                     "both ways",
                (double) timer / (double) CLK_TCK,
                rate);
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

