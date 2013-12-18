#include "comm.h"

#define done    7            /* a flag for termination of a test */
#define field   3            /* the length of each field of a control data */
#define numb   6           /* the number of ports in a DSP */

unsigned long i_mesg,o_mesg;
int o_err=0;
int out_port;

void td2cp(void)
{
  int i=0;
  do
  {  
     while (!comm_rec(i,&i_mesg))    /* waiting for the control word */
    {
      i++;
      if (i==numb)
	i=0;
    }
    out_port=i_mesg & 0x7;         /* get the outgoing port number */
    if (out_port==done)           /* if the current test done ? */
      break;
    o_mesg=i_mesg >> field;       /* strip off the used port number */
    if (!comm_sen(out_port, o_mesg))   /* send the control word */
    {
      o_err=1;
      break;
    }
    while(!comm_rec(i,&i_mesg));     /* recevie the data */
    o_mesg=i_mesg;
    if (!comm_sen(out_port,o_mesg))      /* send the data out */
    {
      o_err=1;
      break;
    }
  } while (out_port!=done);      /* if the current test done ? */
}