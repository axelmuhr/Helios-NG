/* fgrep */
/* Converted to Helios - John Fitch 1988 April 25 */
/* fixed iflag (old yflag) + fixed match of blank lines PAB 15/6/88*/
/* + incorporated fgrep.h file */
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/textutil/RCS/fgrep.c,v 1.6 1994/03/08 11:58:16 nickc Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <syslib.h>

#define FTRUE    -1
#define MAX_LINE 257
#define CHARSET	 256

#define CMD_ERR     0
#define OPT_ERR     1
#define INP_ERR     2
#define STR_ERR     3
#define MEM_ERR     4

typedef int BOOL;

typedef struct queue
{
    struct state_el *st_ptr;
    struct queue *next_el;
} QUEUE;

typedef struct transition
{
    char lchar;
    struct state_el *nextst_ptr;
    struct transition *next_el;
}  TRANSITION;

typedef struct state_el
{
    TRANSITION *go_ls;
    TRANSITION *mv_ls;
    struct state_el *fail_state;
    char *out_str;
} FSA;

FSA FAIL_STATE;

FSA **MZ;

BOOL vflag=FALSE,
    cflag=FALSE,
    lflag=FALSE,
    nflag=FALSE,
    hflag=FALSE,
    iflag=FALSE,
    eflag=FALSE,
    fflag=FALSE,
    xflag=FALSE,
    pflag=FALSE,
    sflag=FALSE;


char *
strsave(char * s ) /* K&R p103*/
{
  char *p;
  if ((p = (char *)Malloc((long)strlen(s)+1))!=NULL)
    strcpy(p,s);
  return(p);
}

/* FGREP functions 1*/


BOOL
proc_file(
	  char *in_file,
	  BOOL prt_flag )
{
    char buffer[MAX_LINE],
         save_buffer[MAX_LINE],
        *nl,
        *stoupper(char *);
    long line_cnt = 0L,
        mtch_cnt = 0L;
    BOOL mtch_flag,
        run_fsa(char *);
    FILE *in_fd;
    void error(int, char *);

    if (in_file != NULL)
    {
        if ((in_fd = fopen(in_file,"r")) == NULL)
            error(INP_ERR,in_file);
    }
    else
        in_fd = stdin;
    while (fgets(buffer,MAX_LINE,in_fd))
    {
        if ((nl = strchr (buffer, '\n')) != NULL)
            *nl='\0';
        if (iflag == FTRUE)
            {
              strcpy(save_buffer,buffer);
              stoupper(buffer);
            }
        line_cnt++;
        if ((mtch_flag = run_fsa(buffer))==FTRUE)
            mtch_cnt++;
        if (cflag == FALSE && lflag == FALSE && sflag ==FALSE &&
            ((mtch_flag == FTRUE && vflag == FALSE) ||
            (mtch_flag == FALSE && vflag == FTRUE)))
        {
            if (hflag == FALSE && prt_flag == FTRUE)
                printf("%s: ",in_file);
            if (nflag == FTRUE)
                printf("%05ld: ",line_cnt);
        if (iflag == FTRUE)
	    puts(save_buffer);
	else
            puts(buffer);
        }
    }
    if (lflag == FTRUE && mtch_cnt > 0)
        printf("%s\n",in_file);
    else if (cflag == FTRUE && sflag == FALSE)
        printf("%ld\n",mtch_cnt);
    if (in_file !=NULL)
        fclose(in_fd);
    if (mtch_cnt)
        return FTRUE;
    else
        return FALSE;
}

/* RUN_FSA */

BOOL
run_fsa(register char *str)
{
    register FSA *st_ptr;
    char *message = NULL;
    BOOL msg_flag = FALSE;
    FSA *go(FSA *,char),
        *move(FSA *,char);

    st_ptr = NULL;
    if (xflag == FALSE)
    {
        while(*str)
        {
            st_ptr =move(st_ptr,*str);
            if (st_ptr == 0 && message)
            {
                printf("--> %s\n",message);
                message = NULL;
                st_ptr = move(st_ptr,*str);
            }
            str++;
            if (st_ptr) {
                if (st_ptr ->out_str) {
                    if (pflag == FTRUE)
                    {
                        message = st_ptr -> out_str;
                        msg_flag = FTRUE;
                    }
                    else
                        return FTRUE;
		  }
	      }
        }
        if (message)
            printf("--> %s\n",message);

        return msg_flag;
    }
    else
    {
    	if (!*str) /* fix for empty str PAB */
    		return FALSE;
        while(*str)
        {
            st_ptr = go(st_ptr,*str++);
            if (!st_ptr || st_ptr == &FAIL_STATE)
                return FALSE;
        }
        return FTRUE;
    }
}

/* GO */

FSA *
go(FSA *st_ptr,char litchar)
{
    register TRANSITION *current;

    if (!st_ptr)
        return MZ[litchar];
    else
    {
        current = st_ptr ->go_ls;
        while (current)
        {
            if (current->lchar == litchar)
                break;
            current = current->next_el;
        }
        return current ? current->nextst_ptr : &FAIL_STATE;
    }
}

/* MOVE */

FSA *move(FSA *st_ptr,char litchar)
{
    register TRANSITION *current;
    if (!st_ptr)
        return MZ[litchar];
    else
    {
        current = st_ptr->mv_ls;
        while (current)
        {
            if (current ->lchar == litchar)
                break;
            current = current ->next_el;
        }
        return current ? current->nextst_ptr : NULL;
    }
}

/* BD_GO */

void bd_go(char *str)
{
    register char litchar;
    char *nl,
        buffer [MAX_LINE],
        *stoupper(char *);
    FILE *str_fd;
    void error(int, char *),
        enter(char *);

    for (litchar=1;litchar <=127; litchar++)
        MZ[litchar] =&FAIL_STATE;
    if (fflag == FTRUE)
    {
        if ((str_fd = fopen(str,"r"))==NULL)
            error(STR_ERR,str);
        while(fgets(buffer, MAX_LINE,str_fd))
        {
            if ((nl= strchr (buffer,'\n')) != NULL)
                *nl='\0';
        if (iflag == FTRUE)
            stoupper(buffer);
        enter(buffer);
        }
        fclose(str_fd);
    }
    else
    {
        if (iflag == FTRUE)
            stoupper(str);	/*PAB15/6/88*/
        enter(str);
    }
    for (litchar =1;litchar <=127;litchar++)
        if(MZ[litchar] == &FAIL_STATE)
            MZ[litchar] = NULL;
}

/* ENTER */

void enter(char *str)
{
    FSA *s,
        *create(void);
    TRANSITION *current,
            *insert(FSA *,char);
    char *strsave(char *);
    register char *temp;
    register FSA *st_ptr = NULL;
    register FSA *nextst_ptr;
/*    void error(int, char *); */

    temp=str;
    while ((s=go(st_ptr,*temp)) != &FAIL_STATE)
    {
        temp++;
        st_ptr = s;
    }
    while (*temp)
    {
        if (!st_ptr)
            nextst_ptr = MZ[*temp++] = create();
        else if ((current = st_ptr -> go_ls) == NULL)
        {
            nextst_ptr = create();
            st_ptr->go_ls = insert(nextst_ptr,*temp++);
        }
        else
        {
            while(current->next_el)
                current = current ->next_el;
            nextst_ptr =create();
            current->next_el = insert(nextst_ptr,*temp++);
        }
        st_ptr =nextst_ptr;
    }
    st_ptr->out_str = strsave(str);
}

/* INSERT */

TRANSITION *insert(FSA *st_ptr,char litchar)
{
    TRANSITION *current;
    void error(int, char *);

    if((current = (TRANSITION *)Malloc(sizeof(TRANSITION))) == NULL)
        error(MEM_ERR,NULL);
    current ->lchar = litchar;
    current ->nextst_ptr = st_ptr;
    current ->next_el = NULL;
    return current;
}

/* CREATE */

FSA *create()
{
    FSA *st_ptr;
    void error(int, char *);

    if ((st_ptr = (FSA *)Malloc(sizeof(FSA)))==NULL)
        error (MEM_ERR,NULL);
    st_ptr->go_ls = st_ptr->mv_ls = NULL;
    st_ptr->fail_state = NULL;
    st_ptr->out_str = NULL;
    return st_ptr;
}

/* BD_MOVE */

void bd_move()
{
    register char litchar;
    register FSA *r,
        *s,
        *t;
    FSA *go(FSA *,char),
        *move(FSA *,char);
    TRANSITION *current = NULL,
        *insert(FSA *,char);
    QUEUE *first,
        *last;
    void add_queue(QUEUE **, QUEUE **,FSA *),
        delete_queue(QUEUE **);

    last=first=NULL;
    for (litchar=1; litchar<=127; litchar++)
        if ((s=go(NULL,litchar)) != NULL)
            add_queue(&first,&last,s);
    while (first)
    {
        r=first->st_ptr;
        delete_queue(&first);
        if (!r->go_ls)
            continue;
        if(r->out_str)
            r->mv_ls =r->go_ls;
        for (litchar =1 ; litchar <=127;litchar++)
        {
            if ((s=go(r,litchar)) != &FAIL_STATE)
            {
                add_queue(&first,&last,s);
                t =r->fail_state;
                while (go(t,litchar) ==&FAIL_STATE)
                    t = t->fail_state;
                s->fail_state = go (t,litchar);
            }
            else
            {
                s = move(r->fail_state,litchar);
            }
            if (s && !r->out_str) {
                if (!r->mv_ls)
                    current = r->mv_ls = insert(s,litchar);
                else
                    current = current->next_el = insert(s,litchar);
	      }
        }
    }
}

/* ADD_QUEUE */

void
add_queue(
	  QUEUE **	head_ptr,
	  QUEUE **	tail_ptr,
	  FSA *		st_ptr )
{
    QUEUE *pq;
    void error(int, char *);

    if((pq= (QUEUE *)Malloc(sizeof(QUEUE))) == NULL)
        error (MEM_ERR,NULL);
    pq->st_ptr = st_ptr;
    pq->next_el = NULL;
    if (!*head_ptr)
        *tail_ptr =*head_ptr = pq;
    else
        *tail_ptr = (*tail_ptr)->next_el = pq;
}

/* DELETE_QUEUE */

void delete_queue(QUEUE **head_ptr)
{
    *head_ptr = (*head_ptr)->next_el;
}

/* STOUPPER */

char *stoupper(register char *str)
{
    register char *temp;

    temp = str;
    while (*temp)
        { *temp = toupper(*temp);
	  temp++;
	}
    
    return str;
}

/* ERROR */

void error(
	   int n,
	   char *str )
{
    fprintf(stderr,"\007\n*** ERROR -");
    switch(n)
    {
        case CMD_ERR:
            fprintf(stderr,"Illegal command line");
            break;
        case OPT_ERR:
            fprintf(stderr,"Illegal command line option");
            break;
        case INP_ERR:
            fprintf(stderr,"Can't open input file %s",str);
            break;
        case STR_ERR:
            fprintf(stderr,"Can't open string file %s",str);
            break;
        case MEM_ERR:
            fprintf(stderr,"Out of memory");
            break;
        default:
            fprintf(stderr,"Unknown error #%d",n);
            break;
    }
    fprintf(stderr," ***\n\nUsage: fgrep [-vclnhiefxps]");
    fprintf(stderr," <strings> [<file>...]\n");
    exit (2);
}


int
main(
     int argc,
     char **argv )
{
    char *temp;
    BOOL match_flag = FALSE,
         proc_file(char *,BOOL);
    void bd_go(char *),
         bd_move(void),
         error(int, char *);

    if (argc<2)
        error(CMD_ERR,NULL);

    while (--argc && (*++argv)[0] == '-' && eflag == FALSE)
        for (temp= argv[0]+1; *temp != '\0'; temp++)
            switch (toupper(*temp))
            {
                case 'V':
                    vflag= FTRUE;	/* all but matching printed */
                    break;
                case 'C':
                    cflag= FTRUE;	/* Only print count of matching lines */
                    break;
                case 'L':
                    lflag= FTRUE;	/* Only print filename that incl. match */
                    break;
                case 'N':
                    nflag= FTRUE;	/* Print line number of matched line */
                    break;
                case 'H':
                    hflag= FTRUE;	/* don't incl. filename in output */
                    break;
                case 'I': /* swaped -y to -i = unix std ignore case PAB */
                    iflag= FTRUE;	/* Ignore case in matches */
                    break;
                case 'E':
                    eflag= FTRUE;	/* prefixed to allow strings to start with a - */
                    break;		/* eg 'fgrep -e --argc tst.c' */
                case 'F':
                    fflag= FTRUE;	/* take list of strings to match from file */
                    break;		/* eg 'fgrep -f find.txt tst.c' */
                case 'X':
                    xflag= FTRUE;	/* Only print lines that exactly match the whole string */
                    break;
                case 'P':
                    pflag= FTRUE;	/* Note what string was matched to the following line output */
                    break;		/* --> string */
                case 'S':
                    sflag= FTRUE;	/* Silent - print only errors */
                    break;		/* ret code 0 = a match, 1 = no matches */
					/* 2 = error */
                default:
                    error(OPT_ERR,NULL);
            }

    if (vflag == FTRUE ||
        cflag == FTRUE ||
        lflag == FTRUE ||
        xflag == FTRUE ||
        sflag == FTRUE) pflag = FALSE;

    if ( ( MZ = (FSA **) malloc ( ( CHARSET  ) * sizeof ( * MZ ) ) ) == (FSA **) NULL ) {
    	fprintf(stderr,"fgrep: Unable to allocate temporary workspace.\n");
    	exit (1);
    }

    
    bd_go(*argv++);
    argc--;

    bd_move();

    if (argc < 2)
        hflag = FTRUE;
    if (!argc)
    {
        if (proc_file(NULL,FALSE) ==FTRUE && match_flag== FALSE)
            match_flag = FTRUE;
    }
    else
        while (argc--)
            if (proc_file(*argv++,FTRUE) ==FTRUE && match_flag== FALSE)
            match_flag = FTRUE;
    if (match_flag == FTRUE)
        exit(0);
    else
        exit(1);
}

