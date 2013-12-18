/*------------------------------------------------------------------------
--                                                                      --
--                   H E L I O S   I / O   S E R V E R                  --
--                   ---------------------------------                  --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      sangfifo.c                                                       --
--                                                                      --
--               The link module for sang ml using fifo buffers         --
--                                                                      --
--  Author:  MIG 23/06/94                                               --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: sangfifo.c,v 1.1 1994/07/06 10:49:30 mgun Exp $ */
/* Copyright (C) 1988, Perihelion Distributed Software Ltd. */

/* #define USE_C */

#define RESET_COUNT     3200
#define ANALYSE_COUNT   3200

#if (!MSWINDOWS)
#define normalize(datadr) {\
	unsigned long temp, t1, t2;\
	temp = (unsigned long) datadr;\
	t1 = (temp>>12)&0x000ffff0L;\
	t2 = temp&0x0000ffffL;\
	temp = t1+t2;\
	t1 = (temp & 0x000ffff0L)<<12;\
	t2 = temp & 0x0fL;\
	temp = t1+t2;\
	datadr = (void huge *) temp;\
}

#else
#define normalize(datadr)
#endif

#pragma loop_opt(off)

void sang_reset(void)
{
    register int i;
    
    Debug( Boot_Flag, ("sang_reset") );
    
    outp(int_enable, 0);
    outp(dmla_control, DLA_OFF_WRITE);
    
    outp(link_analyse, 0);
    for (i=0; i<ANALYSE_COUNT; i++);
    
    outp(link_reset, 0);
    for (i=0; i<RESET_COUNT; i++);
    
    outp(link_reset, 1);
    for (i=0; i<RESET_COUNT; i++);
    
    outp(link_reset, 0);
    for (i=0; i<RESET_COUNT; i++);
    
    /* enable input and output signals for C012 */
    outp(link_in_status,  (char)2);
    outp(link_out_status, (char)2);
}

void sang_analyse(void)
{
    register int i;
    
    Debug( Boot_Flag, ("sang_analyse") );

    outp(int_enable, 0);
    outp(dmla_control, DLA_OFF_WRITE);
    
    outp(link_analyse, 0);
    for (i=0; i<ANALYSE_COUNT; i++);

    outp(link_analyse, 1);
    for (i=0; i<ANALYSE_COUNT; i++);

    outp(link_reset, 1);
    for (i=0; i<RESET_COUNT; i++);
    
    outp(link_reset, 0);
    for (i=0; i<RESET_COUNT; i++);
    
    outp(link_analyse, 0);
    for (i=0; i<ANALYSE_COUNT; i++);

    /* enable input and output signals for C012 */
    outp(link_in_status,  (char)2);
    outp(link_out_status, (char)2);
}

#pragma loop_opt()

int sang_send_block(Count, Buffer, timeout)
unsigned int Count;
char huge *Buffer;
int timeout;
{
    register unsigned int number;
    register int i;
    short far *zeig;
    int blocklen;
    int fifo;
    
    number = Count;
    fifo = fifo_acc;
    
    /* Normalize buffer pointer */
    normalize (Buffer);
    
    if (number & 1) {
	while (!(inp(link_out_status) & 1));
	
	outp(link_write, *Buffer++);
	number--;
    }
    
    zeig = (short *)Buffer;
    
    outp(dmla_control, DLA_ON_WRITE);
    
    blocklen = FIFO_LENGTH >> 1;
    while (number >= FIFO_LENGTH) {
#ifdef USE_C
	for (i=blocklen; i>0; i--) {
	    outpw(fifo_acc, *zeig++);
	}
#else
	__asm {
	    pushf;
	    push CX;
	    push DS;
	    push SI;
	    push DX;
	    
	    cld;
	    
	    lds SI, zeig;
	    mov DX, fifo;
	    mov CX, blocklen;
	    rep outsw;
	    
	    pop DX;
	    pop SI;
	    pop DS;
	    pop CX;
	    popf;
	}
	zeig += blocklen;
#endif
	
	while (!(inp(dmla_info) & FIFO_EMPTY));
	
	number -= FIFO_LENGTH;
    }
    
    /* transfer the remaining data */
#ifdef USE_C
    for (i=number>>1; i>0; i--)
    {
	outpw(fifo_acc, *zeig++);
    }
#else
    if (number > 0)
    {
	blocklen = number >> 1;
	__asm {
	    pushf;
	    push CX;
	    push DS;
	    push SI;
	    push DX;
	    
	    cld;
	    
	    lds SI, zeig;
	    mov DX, fifo;
	    mov CX, blocklen;
	    rep outsw;
	    
	    pop DX;
	    pop SI;
	    pop DS;
	    pop CX;
	    popf;
	}
    }
#endif
    
    while (!(inp(dmla_info) & FIFO_EMPTY));
    
    outp(dmla_control, DLA_OFF_WRITE);
    
    return (0);
}

int sang_fetch_block(Count, Buffer, timeout)
char huge *Buffer;
unsigned int Count;
int timeout;
{
    register unsigned int number;
    register int i;
    short far *zeig;
    int blocklen;
    int fifo;
    int c12is;
    int c12id;
    
    /* Normalize buffer pointer */
    normalize (Buffer); 
  
    number = Count;
    
    fifo = fifo_acc;
    c12is = link_in_status;
    c12id = link_read;
    
    if (number < FIFO_LENGTH) {
#ifdef USE_C
	for (i=number; i>0; i--) {
	    while (!(inp(link_in_status) & 1));
	    *Buffer++ = inp(link_read);
	}
#else
	__asm {
	    pushf;
	    push CX;
	    push DS;
	    push SI;
	    push DX;
	    
	    cld;
	    
	    les DI, Buffer;
	    mov CX, number;
	}
	lab1:
	__asm {
	    mov DX, c12is;
	}
	lab2:
	__asm {
	    in  AL, DX;
	    and AL, 1h;
	    jz  lab2;
	    mov DX, c12id;
	    insb;
	    loop lab1;
	    
	    pop DX;
	    pop SI;
	    pop DS;
	    pop CX;
	    popf;
	}
#endif
	return(0);
    }
    
    /* Handle the larger data transfers */
    if (number & 1) {
	while (!(inp(link_in_status) & 1));
	*Buffer++ = inp(link_read);
	number--;
    }
    
    zeig = (short *)Buffer;
    
    /* start DLA */
    outp(dmla_control, DLA_ON_READ);
    
    /* align data to FIFO_LENGTH */
    blocklen = (number % FIFO_LENGTH) >> 1;
    
    while (!(inp(dmla_info) & FIFO_FULL));
    
    /* transfer alignment block */
#ifdef USE_C
    for (i=blocklen; i>0; i--) {
	*zeig++ = inpw(fifo_acc);
    }
#else
    __asm {
	pushf;
	push CX;
	push DS;
	push SI;
	push DX;
	
	cld;
	
	les DI, zeig;
	mov DX, fifo;
	mov CX, blocklen;
	rep insw;
	
	pop DX;
	pop SI;
	pop DS;
	pop CX;
	popf;
    }
    zeig += blocklen;
#endif
    
    number -= number % FIFO_LENGTH;
    
    blocklen = FIFO_LENGTH >> 1;
    while (number > 0) {

	while (!(inp(dmla_info) & FIFO_FULL));
	
	if (number == FIFO_LENGTH) {
	    outp(dmla_control, DLA_OFF_READ);
	}
	
#ifdef USE_C
	for (i=blocklen; i>0; i--) {
	    *zeig++ = inpw(fifo_acc);
	}
#else
	__asm {
	    pushf;
	    push CX;
	    push DS;
	    push SI;
	    push DX;
	    
	    cld;
	    
	    les DI, zeig;
	    mov DX, fifo;
	    mov CX, blocklen;
	    rep insw;
	    
	    pop DX;
	    pop SI;
	    pop DS;
	    pop CX;
	    popf;
	}
	zeig += blocklen;
#endif
	
	number -= FIFO_LENGTH;
    }
    
    return (0);
}


