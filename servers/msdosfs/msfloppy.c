/*------------------------------------------------------------------------
--                                                                      --
-- MSFLOPPY.C                                                           --
--                                                                      --
-- Author : MJT 26/02/91                                                -- 
--                                                                      --
-- msfloppy - format or check an MS-DOS floppy. All the hard work is    --
--            carried out by the msdosfs server.                        --
--                                                                      --
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /hsrc/servers/msdosfs/RCS/msfloppy.c,v 1.2 1991/03/28 18:04:09 martyn Exp $";

#include "msdosfs.h"

char *progname;

Port my_connect_to_server(string);
void usage(void);
void printstats(word reply);

int main(int argc, char **argv)
{

    char *servername = NULL;
    Object *server;
    MCB  mcb;
    Port stream_port;
    Port myport = NewPort();
    word result;
    word format_flags = DT_MFM | DT_IBM;
    word format = FALSE;
    word chkdsk = FALSE;
    char junk[10];
    word ret_code = -1;

    progname = argv[0];

    fprintf(stdout, "%s (Version %s)\n", progname, VERSION);

    while(++argv, --argc)
        {
        switch(*argv[0])
            {
            case '-':
                switch(argv[0][1])
                    {
                    case 'f':
                        format = TRUE;
                        if(chkdsk)
                            {
                            fprintf(stderr, "(overriding -c option)\n");
                            fflush(stderr);
                            chkdsk = FALSE;
                            }
                        break;

                    case 'c':
                        chkdsk = TRUE;
                        if(format)
                            {
                            fprintf(stderr, "(overriding -f option)\n");
                            fflush(stderr);
                            format = FALSE;
                            }
                        break;

                    case 'h':
                        format_flags |= DT_HIGHDEN;
                        break;

                    case 'w':
                        format_flags |= FSCK_WRITE;
                        break;

                    default:
                        usage();
                    }
                break;

            default:
                if(servername != NULL)
                    {
                    fprintf(stderr, "(overriding %s)\n", servername);
                    fflush(stderr);
                    }
                servername = argv[0];
                break;
            }
        }

    if(chkdsk || !format)
        format_flags |= FSCK;
    
    if(servername == NULL)
        servername = "/dos";

    stream_port = my_connect_to_server(servername);
    if (stream_port == NullPort)
        {
        fprintf(stderr,"%s: could not locate %s\n",progname,servername);
        fflush(stderr);
        exit(2);
        }

    if (myport == NullPort)
    {
        fprintf(stderr, "%s: failed to get a port\n", progname);
        fflush(stderr);
        exit(3);
    }

    fprintf(stdout, "\nInsert floppy to be %s, hit RETURN when ready ...",
            format ? "formatted" : "checked");
    fflush(stdout);

    if(fgets(junk, 9, stdin) == NULL)
        exit(4);

    fprintf(stdout, "\n%s %s ...\n\n", format ? "Formatting" : "Checking",
                servername);
    fflush(stdout);

    mcb.Control = (word *) &ret_code;
    mcb.Data    = (char *) &format_flags;

    mcb.MsgHdr.DataSize = sizeof(word);
    mcb.MsgHdr.ContSize = 0;

    mcb.MsgHdr.Dest  = stream_port;
    mcb.MsgHdr.Reply = myport;
    mcb.MsgHdr.Flags = MsgHdr_Flags_preserve;   
    mcb.MsgHdr.FnRc  = FC_GSP + FG_Format;
    mcb.Timeout      = 5 * OneSec;

    if ((result = PutMsg(&mcb)) < 0)
    {
        fprintf(stderr, "%s: server connection failure (w)- %x\n",
                    progname, result);
        fflush(stderr);
        exit(5);
    }

    mcb.MsgHdr.Dest = myport;
    mcb.Timeout     = 5 * OneSec;

    while ((result = GetMsg(&mcb)) == EK_Timeout);	/* wait for message */

    printstats(ret_code);
printf("result %x\n", result);

    if (result < 0) 
    {
        fprintf(stderr, "%s: %s failed - %x\n", format ? "format" : "check",
			progname, result);
        fflush(stderr);
        exit(6);
    }
}

Port my_connect_to_server(string name)
{
    Object *server;
    Stream *stream;

    server = Locate(Null(Object), name);
    if (server == Null(Object))
        return(NullPort);

    stream = Open(server, Null(char), O_ReadWrite);
    if (stream == Null(Stream))
    {
        Close(server);
        return(NullPort);
    }

    Close(server);
    return(stream->Server);
}

void usage(void)
{
    fprintf(stderr, "%s: usage -\n\t%s [-c [-w] | -f [-h]] [servername]\n",
            progname, progname);
    fprintf(stderr, "where:\n\t-c\tturns on chkdsk option\n");
    fprintf(stderr, "\t-c -w\tturns on chkdsk and update option\n");
    fprintf(stderr, "\t-f\tturns on format option\n");
    fprintf(stderr, "\t-f -h\tturns on format high density option\n");
    fprintf(stderr, "default is:\n\t%s -c /dos\n", progname);
    fflush(stderr);
    exit(1);
}

void printstats(word reply)
{
	printf("returned %x\n", reply);
}
