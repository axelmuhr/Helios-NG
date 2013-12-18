#include <math.h>
#include "comm.h"

#define done     7             /* a flag for termination of a test */
#define nu_patn  4             /* number of data pattens */
#define nu_rout  6             /* number of routes being tested */
#define numb     3             /* number of connected ports in DSP1 */
unsigned long data[nu_patn]={0x0,0xFFFFFFFF,0xAAAAAAAA,0x55555555};   /* data to be sent */

unsigned long  out_2=1;         /* the route ID when without the daughter card attached */
unsigned long   rout[nu_rout]={0053,0143,043,003,003,043};   /* route IDs in octal */

unsigned long   i_mesg;         /* the received message */
int  port[numb]={0,1,2};        /* the connected port numbers in DSP1 */
int  o_err=0, t_err=0;
int  out_port=3;


void td1cp( int daughter )
{
  int i,j,k=0;

  if (daughter)             /* Test All Four Processors */
  {
    
   for (i=0;i<nu_rout;i++)
   {  
    if (!fmod(i,2))               /* if it is a forword or backword test ? */
      out_port=0;
    else 
      out_port=k;
    for ( j=0;j<nu_patn;j++)
      {
        if (!comm_sen(out_port,rout[i]))   /* send the control word out */
        {
          o_err=1;
   	  break;
	}
	if (!comm_sen(out_port,data[j]))   /* send the data out */	
        {
	  o_err=1;
          break;
        }
       while (!comm_rec(port[k],&i_mesg))   /* wait for the control word coming back */
        {	  
          k++;
          if (k==numb)
          k=0;
        }
        while (!comm_rec(port[k],&i_mesg));   /* dump the control word and get the data coming back */

        if (i_mesg!=data[j])       /* is there any error in the received data */
        {
          t_err=1;
          break;
        }
      }
      if( o_err==1||t_err==1)    /* if error happened, break out */ 
	break;
    }
  
    for (i=0;i<numb;i++)
      if (!comm_sen(i,done))      /* send the "done" flag to all other DSPs */
      {
        o_err=1;
        break;
      }

 }
 else            /* Only test DSP 2 */
 {
     for ( j=0;j<nu_patn;j++)
      {
        if (!comm_sen(out_port,out_2))   /* send the control word out */
        {
          o_err=1;
   	  break;
	}
	if (!comm_sen(out_port,data[j]))   /* send the data out */	
        {
	  o_err=1;
	  break;
	}
       while (!comm_rec(out_port,&i_mesg));   /* wait for the control word coming back */
       while (!comm_rec(out_port,&i_mesg));   /* dump the control word and get the data coming back */
       
       if (i_mesg!=data[j])       /* is there any error in the received data */
        {
          t_err=1;
          break;
        }
      }
      if (!comm_sen(out_port,done))      /* send the "done" flag to DSP2 */
        o_err=1;
   }
}	
						  