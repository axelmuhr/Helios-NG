/*
 ************************************************************************
 *
 * Communication Host - Transputer via MEGA-Link DMA
 * V 0.9 23-April-1990 by Gerhard Bartz
 * (c) 1990 by SANG Computersysteme GmbH, West Germany
 ************************************************************************
 */

PRIVATE int dla_sort, dla_state;

extern void initC012(void);

#define DLAUNKNOWN -1
#define DLAWRITES 0
#define DLAREADS 1
#define DLAOFF 0
#define DLAON 1
/*
#define FASTVERSION
*/
#define set_dla_write() {\
	if (dla_sort!=DLAWRITES) {\
		while((inp(dmla_info)&0x01)!=1);\
		outp(dmla_control,0);\
		outp(dmla_control,2);\
	}\
	else if (dla_state==DLAOFF)\
		outp(dmla_control,2);\
	dla_sort = DLAWRITES;\
	dla_state = DLAON;\
}

#define set_dla_read() {\
	if (dla_sort!=DLAREADS) {\
		while((inp(dmla_info)&0x01)!=1);\
		outp(dmla_control,0);\
		outp(dmla_control,3);\
	}\
	else if (dla_state==DLAOFF)\
		outp(dmla_control,3);\
	dla_sort = DLAREADS;\
	dla_state = DLAON;\
}

#define dla_off() {\
	while((inp(dmla_info)&0x01)!=1);\
	outp(dmla_control,0);\
	dla_sort = DLAUNKNOWN;\
	dla_state = DLAOFF;\
}

PRIVATE unsigned long ztacount;

#define dla_off_t() {\
	for(ztacount = 400000L;((inp(dmla_info)&0x01)!=1)&&(ztacount>0L);ztacount--);\
	if (ztacount==0L) return(1);\
	outp(dmla_control,0);\
	dla_sort = DLAUNKNOWN;\
	dla_state = DLAOFF;\
}

#define raw_dla_off() {\
	if (dla_sort==DLAWRITES) {\
		outp(dmla_control,0);\
	}\
	else outp(dmla_control,1);\
	dla_state = DLAOFF;\
}

#define RESET_COUNT 32000
#define ANALYSE_COUNT 32000

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

PRIVATE unsigned int fifo_size, fifo_hold;


void get_fifo_size(void)
{
	unsigned int i;

	set_dla_write();
	raw_dla_off();
	for(i=0; ((inp(dmla_info)&0x02)!=0x02); i+=2)
		outpw(fifo_location,0);
	fifo_size=i;
	fifo_hold = 2*fifo_size;
	ServerDebug("FIFO size is %d bytes\n", fifo_size);
}
		

void sang_reset(void)
/* Reset the root transputer. */
{
    register int i;
    outp (link_analyse, FALSE);         /* deassert analyse */
    outp (link_reset, FALSE);           /* deassert reset   */
    for (i=0; i < RESET_COUNT; i++)     /* wait awhile      */
        ;
    outp (link_reset, TRUE);            /* assert reset     */
    for (i=0; i < RESET_COUNT; i++)     /* wait awhile      */
        ;
    outp (link_reset, FALSE);           /* deassert reset   */
    for (i=0; i < RESET_COUNT; i++)     /* wait awhile      */
        ;
    get_fifo_size();
    outp (link_reset, TRUE);            /* assert reset     */
    for (i=0; i < RESET_COUNT; i++)     /* wait awhile      */
        ;
    outp (link_reset, FALSE);           /* deassert reset   */
    for (i=0; i < RESET_COUNT; i++)     /* wait awhile      */
        ;
    initC012();
}

void sang_analyse (void)
/* Reset the root transputer into analyse mode. */
{
    register int i;
    outp (link_reset, FALSE);           /* deassert reset   */
    outp (link_analyse, FALSE);         /* deassert analyse */
    for (i=0; i < RESET_COUNT; i++)     /* wait awhile      */
        ;
    outp (link_analyse, TRUE);          /* assert analyse   */
    for (i=0; i < ANALYSE_COUNT; i++)   /* wait awhile      */
        ;
    outp (link_reset, TRUE);            /* assert reset     */
    for (i=0; i < RESET_COUNT; i++)     /* wait awhile      */
        ;
    outp (link_reset, FALSE);           /* deassert reset   */
    outp (link_analyse, FALSE);         /* deassert analyse */
    initC012();
}


/*
 ************************************************************************
 * DMA routines
 ************************************************************************
 */


#define DMALNG real_len-1

/*
 * initialyze the C012 and read MLDMA configuration
 */

PRIVATE unsigned int b_internal;
PRIVATE BYTE b_buffer;
PRIVATE unsigned int mlen;

void initC012(void)
{
	int statr;

	outp(link_in_status,(char)2);
	outp(link_out_status,(char)2);
	statr = inp(dmla_info);
	if (statr&0x30) {
		DMAchannel = statr&0x30;
		DMAchannel>>=4;
		b_internal = 0;
		mlen = 0;
	}
	else {
		ServerDebug("NO DMA ON LINKBASE %x\n",link_base);
		exit(0);
	}
}

#define add24bit(datadr, byte0, byte1, byte2) {\
	unsigned long temp,t1,t2;\
	temp = (long) datadr;\
	t1 = (temp>>12)&0x000ffff0L;\
	t2 = temp&0x0000ffffL;\
	temp = t1+t2;\
	byte2 = (temp>>16)&0x00ffL;\
	temp >>= 1;\
	byte0 = temp&0x00ffL;\
	byte1 = (temp>>8)&0x00ffL;\
}

/*** test wether 64 K overflow of ptr+add occurs ***/
/*** return an allowed value avoiding overflow   ***/

unsigned int size_granted64(dpntr, addd)
char far *dpntr;
unsigned int addd;
{
	unsigned long now,next,t1,t2,instead;
	unsigned int help;

	now = (unsigned long) dpntr;
	t1 = (now>>12)&0x000ffff0L;
	t2 = now&0x0000ffffL;
	now = t1+t2;
	next = now + addd;
	if (((next&0x00ff0000L)-(now&0x00ff0000L))==0L) {
		/*** be happy, don't worry ***/
		return(addd);
	}
	else {
		/*** force alignment ***/
		instead = now & 0x00ff0000L;
		instead = instead - now;
		help = (unsigned int) instead;
		return(help);
	}
}



#define aac(addrch,pagech,wordch) { \
	outp((pagech), addr2);\
	outp((DMAffclear),0);\
	outp((addrch), addr0);\
        outp((addrch), addr1);\
	outp((wordch), length&0x00ff);\
	outp((wordch), (length>>8)&0x00ff);\
}


void setupDMAC (readnotwrite, length, data)
unsigned int readnotwrite, length;
char far *data;
{
	unsigned char addr0,addr1,addr2 /* direction */;
	add24bit(data, addr0, addr1, addr2);
	switch (DMAchannel) {
		case chan5: aac(addrch5, pagech5, wordch5);
		  break;
		case chan6: aac(addrch6, pagech6, wordch6);
		  break;
		case chan7: aac(addrch7, pagech7, wordch7);
		  break;
	}

 	outp(DMAmode, 0x00|readnotwrite|DMAchannel);

	outp(DMAcommand, 0x10);
	outp(DMAsignmask, DMAchannel); 
}


void pollDMAC(void)
{
	unsigned char chanmask;
	chanmask = 1<<DMAchannel;
	while((inp(DMAstatus)&chanmask)==0);
}

int pollDMAC_t()
{
	unsigned char chanmask;
	unsigned long i;

	i=3000000L;
	chanmask = 1<<DMAchannel;
	while((inp(DMAstatus)&chanmask)==0) if (--i==0L) break;
	return(i!=0L);
	/* 1 means OK, 0 means ERROR */
}

/*
 ************************************************************************
 * SEND/FETCH routines
 * using 16 Bit DMA to MEGA-Link DMA
 * They return 1 for error, 0 for success
 * To gain clean pointer arithmetics:
 * huge pointers are used rather than far ones
 * Due to sever requirements a modified byte_from_link
 * and a modified rdrdy are necessary
 ************************************************************************
 */

 
int sang_send_block(length,slice,timeout)
unsigned int length;
char huge *slice;
int timeout;
{
	unsigned int real_len, current_len;
	unsigned long tempptr;
	real_len = length;

/*
	if (real_len>mlen) {
		mlen = real_len;
		ServerDebug("max %x\n", mlen);
	}
*/
	tempptr = (unsigned long) slice;
	if ((tempptr&1L)==1L) {
		if (byte_to_link(*slice)) return(1);
		slice++;
		real_len--;
	}
	while (real_len>32000) {
		current_len = size_granted64(slice, 32000);
		real_len -= current_len;
		setupDMAC(toML, (current_len>>1)-1, slice);
                set_dla_write();
		inp(dmla_control);
		slice+=(current_len&0xfffe);
                if (!pollDMAC_t()) return(1);
		if (current_len&1) { 
                dla_off();
		if (byte_to_link(*slice)) return(1);
		slice++;
		}
	}
	while (real_len>1) {
		current_len = size_granted64(slice, real_len);
		real_len -= current_len;
		setupDMAC(toML, (current_len>>1)-1, slice);
                set_dla_write();
		inp(dmla_control);
		slice+=(current_len&0xfffe);
                if (!pollDMAC_t()) return(1);
		if (current_len&1) { 
			dla_off();
			if (byte_to_link(*slice)) return(1);
			slice++;
		}
	}
	if (dla_sort==DLAWRITES) dla_off();
	for(;real_len>0; real_len--,slice++) if (byte_to_link(*slice)) return(1);
	return(0);
}
/*
#define SFBTDEBUG 1
*/
#define SFBDEBUG 1
/*
#define SFISDEBUG 1
*/

int sang_fetch_block(length,slice,timeout)
unsigned int length;
char huge *slice;
int timeout;
{
	unsigned int real_len, current_len;
	unsigned long tempptr;
	unsigned int d_buffer, count;
/*
	unsigned char *p1, *p2;
*/
	if (length<=fifo_hold) return(fetch_block(length,slice,timeout));
	real_len = length;
	/*** if starting address is odd, get single byte ***/
	tempptr = (unsigned long) slice;

	if ((tempptr&1L)==1L) {
		if (byte_from_link(slice)) return(1);
		slice++;
		real_len--;
	}

	/*** DMA access ! ***/
	while (real_len>30000+fifo_size) {
		current_len = size_granted64(slice,30000);
		setupDMAC(toAT, current_len/2-1, slice);
	        set_dla_read();
		inp(dmla_control);
		real_len-=current_len&0xfffe;
		slice+=current_len&0xfffe;

             	if (!pollDMAC_t()) return(1);
       	}
	while (real_len>10000+fifo_size) {
		current_len = size_granted64(slice,10000);
		setupDMAC(toAT, current_len/2-1, slice);
	        set_dla_read();
		inp(dmla_control);
		real_len-=current_len&0xfffe;
		slice+=current_len&0xfffe;

             	if (!pollDMAC_t()) return(1);

       	}
	while (real_len>fifo_hold) {
		current_len = size_granted64(slice,fifo_size);
		setupDMAC(toAT, current_len/2-1, slice);
	        set_dla_read();
		inp(dmla_control);
		real_len-=current_len&0xfffe;
		slice+=current_len&0xfffe;
             	if (!pollDMAC_t()) return(1);
       	}
       	if (dla_sort == DLAREADS) {
		/*** empty full fifo only ***/

		while (!(inp(dmla_info)&0x02));

	       	raw_dla_off();

		d_buffer = fifo_size;
		while (d_buffer>0) {
			current_len = size_granted64(slice,d_buffer);
			setupDMAC(toAT, current_len/2-1, slice);
			inp(dmla_control);
			real_len-=current_len&0xfffe;
			d_buffer-=current_len&0xfffe;
			slice+=current_len&0xfffe;

	             	if (!pollDMAC_t()) return(1);
	       	}
	}
       	/*** read last bytes, if necessary ***/
       	dla_off();
       	if (real_len>0) return(fetch_block(real_len,slice,timeout));
}

