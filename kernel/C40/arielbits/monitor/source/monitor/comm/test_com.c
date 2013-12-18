#include "comm.h"
#define   count1  2
#define   count2  4

int o_err=0, i_err=0;
unsigned long i_mesg;
int k;

main()
{
    

    com_init();
     
    for (k=0; k<count1; k++)
    {
       if (!comm_sen(2,15))
          o_err=1;
     }

     for (k=0; k<count2; k++)
     { 
        if (!comm_rec(5, &i_mesg)) 
    	  i_err=1; 
     }
}
