#include "hydra.h"

#define  nu_port  6

#define  buf_size 0

#if buf_size > 0      /* Use Software Buffering */

  #define  o_flag   1
  #define  i_flag   0

  typedef struct
  {
     unsigned long ibuf[buf_size],obuf[buf_size];
     int ihead,itail;
     int ohead,otail;
     int iful;
     int oful;
   } comm_buf;

   static comm_buf  port[nu_port];

   typedef void (*fcn_ptr)();
   fcn_ptr stubs[12] = { c_int02, c_int03, c_int04, c_int05, c_int06, c_int07,
                         c_int08, c_int09, c_int10, c_int11, c_int12, c_int13 };



    int  comm_sen(int no, unsigned long omessage, int tries)

    {
	dis_port(no,o_flag);

	if (port[no].ohead==port[no].otail)
	    if (port[no].oful==1)
	    {
	       en_port(no, o_flag);
	       return   0;
	    }                        /* error  */

	port[no].obuf[port[no].ohead++]=omessage;

	if (port[no].ohead==buf_size)
	    port[no].ohead=0;

	if (port[no].ohead==port[no].otail)
	    port[no].oful=1;
	en_port(no, o_flag);
	return    1;                    /* normal return */
    }


    void o_isr(int no)
    {
	while  (o_crdy(no, NumTries))
	{
	     if  (port[no].ohead==port[no].otail)    /* if obuf empty or full? */
		  if (port[no].oful==0)
		  {
		      dis_port(no, o_flag);
		      return;                       /* obuf empty */
		  }
	     /* obuf is not empty */
	     *((unsigned long *)0x100042 + (no*0x10))=port[no].obuf[port[no].otail++];
	     if (port[no].otail==buf_size)
		 port[no].otail=0;
	     port[no].oful=0;

	}
    }




    int   comm_rec(int no, unsigned long *imessage, int tries)

    {
       dis_port(no,i_flag);

       if (port[no].ihead==port[no].itail)
	 if (port[no].iful==0)
	 {
	    en_port(no, i_flag);
	    return  0;               /* empty ibuf, error */
	 }

       *imessage=port[no].ibuf[port[no].itail++];
       if (port[no].itail==buf_size)
	  port[no].itail=0;
       port[no].iful=0;
       en_port(no,i_flag);
       return   1;                 /* normal return */
    }


    void i_isr(int no)
    {
       while (i_crdy(no, tries))
       {
	 port[no].ibuf[port[no].ihead++]=*((unsigned long *)0x100041 + (no*0x10));

	 if (port[no].ihead==buf_size)
	     port[no].ihead=0;

	 if (port[no].ihead==port[no].itail)      /* ibuf full ? */
	 {
	     port[no].iful=1;                 /* yes */
	     dis_port(no,i_flag);                  /* turn off the interrupts */
             return;
	  }
        }
    }


    void com_init(void)
    {
      int i;

      SetISRs( stubs );

      for ( i=0;i<nu_port;i++)
      {
	port[i].ihead=0;
	port[i].itail=0;
	port[i].ohead=0;
	port[i].otail=0;
	port[i].iful=0;
	port[i].oful=0;
      }
    }

    /*********************************************************************
     These are interrupt entry points.  They call the underlying ISR's 
     and identify which port is requesting service.
    **********************************************************************/


    void c_int02( void )
    {
	i_isr( 0 );
    }
    void c_int03( void )
    {
	o_isr( 0 );
    }

    void c_int04( void )
    {
	i_isr( 1 );
    }
    void c_int05( void )
    {
	o_isr( 1 );
    }

    void c_int06( void )
    {
	i_isr( 2 );
    }
    void c_int07( void )
    {
	o_isr( 2 );
    }

    void c_int08(void)
    {

        i_isr(3);
    }
    void c_int09(void)
    {
        o_isr(3);
    }

    void c_int10(void)
    {
        o_isr(4);
    }
    void c_int11(void)
    {
        i_isr(4);
    }

    void c_int12(void)
    {
        i_isr(5);
    }
    void c_int13(void)
    {
        o_isr(5);
    }

#else         /* Do Not Use Software Buffering */

  int comm_sen(int no, unsigned long omessage, int tries)
  {
     if (o_crdy(no, tries))
     {
        *((unsigned long *)0x100042 + (no*0x10)) = omessage;
        return  1;
     }

     else
        return  0;
   }

   int comm_rec(int no, unsigned long *imessage, int tries)
   {
      if (i_crdy(no, tries))
      {
          *imessage = *((unsigned long *)0x100041 + (no*0x10));
          return  1;
      }
      else
          return  0;
   }

    void com_init(void)
    {
    }

#endif     




int o_crdy( int channel, int tries )
{
	int i;

	if( tries )
	{
		for( i=0 ; i < tries ; i++ )
			if( (*(unsigned long *)(0x100040 + (channel* 0x10)) & 0x1e0) != 0x1e0 )
				return( 1 );
		return( 0 );
	}
	else
	{
		while( (*(unsigned long *)(0x100040 + (channel* 0x10)) & 0x1e0) == 0x1e0 );

		return( 1 );
	}	
} 


int i_crdy( int channel, int tries )
{
	int i;

	if( tries )
	{
		for( i=0 ; i < tries ; i++ )
			if( *(unsigned long *)(0x100040 + (channel* 0x10)) & 0x1e00 )
				return( 1 );

		return( 0 );
	}
	else
	{
		while( (!*(unsigned long *)(0x100040 + (channel* 0x10)) & 0x1e00) );

		return( 1 );
	}
}
