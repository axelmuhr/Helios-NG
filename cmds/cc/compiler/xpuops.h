
/* C compiler file xpuops.h :  Copyright (C) A.Mycroft and A.C.Norman */
/* version 0.02a */
/* $Id: xpuops.h,v 1.1 1990/09/13 17:11:17 nick Exp $ */

#ifndef _XPUOPS_LOADED

#define _XPUOPS_LOADED 1


#define MinInt	0x80000000


/* Transputer opcodes... */

/* direct functions */
#define f_j            0x0
#define f_ldlp         0x1
#define f_pfix         0x2
#define f_ldnl         0x3
#define f_ldc          0x4
#define f_ldnlp        0x5
#define f_nfix         0x6
#define f_ldl          0x7
#define f_adc          0x8
#define f_call         0x9
#define f_cj           0xa
#define f_ajw          0xb
#define f_eqc          0xc
#define f_stl          0xd
#define f_stnl         0xe
#define f_opr          0xf

/* operations are 0x100 + operand	*/
#define op_rev           0x100
#define op_lb            0x101
#define op_bsub          0x102
#define op_endp          0x103
#define op_diff          0x104
#define op_add           0x105
#define op_gcall         0x106
#define op_in            0x107
#define op_prod          0x108
#define op_gt            0x109
#define op_wsub          0x10a
#define op_out           0x10b
#define op_sub           0x10c
#define op_startp        0x10d
#define op_outbyte       0x10e
#define op_outword       0x10f

#define op_seterr        0x110

#define op_resetch       0x112
#define op_csub0         0x113

#define op_stopp         0x115
#define op_ladd          0x116
#define op_stlb          0x117
#define op_sthf          0x118
#define op_norm          0x119
#define op_ldiv          0x11a
#define op_ldpi          0x11b
#define op_stlf          0x11c
#define op_xdble         0x11d
#define op_ldpri         0x11e
#define op_rem           0x11f
#define op_ret           0x120
#define op_lend          0x121
#define op_ldtimer       0x122
#define op_testlds	 0x123
#define op_testlde	 0x124
#define op_testldd	 0x125
#define op_teststs	 0x126
#define op_testste	 0x127
#define op_teststd	 0x128
#define op_testerr       0x129
#define op_testpranal    0x12a
#define op_tin           0x12b
#define op_div           0x12c
#define op_testhardchan  0x12d

#define op_dist          0x12e
#define op_disc          0x12f
#define op_diss          0x130
#define op_lmul          0x131
#define op_not           0x132
#define op_xor           0x133
#define op_bcnt          0x134
#define op_lshr          0x135
#define op_lshl          0x136
#define op_lsum          0x137
#define op_lsub          0x138
#define op_runp          0x139
#define op_xword         0x13a
#define op_sb            0x13b
#define op_gajw          0x13c
#define op_savel         0x13d
#define op_saveh         0x13e
#define op_wcnt          0x13f
#define op_shr           0x140
#define op_shl           0x141
#define op_mint          0x142
#define op_alt           0x143
#define op_altwt         0x144
#define op_altend        0x145
#define op_and           0x146
#define op_enbt          0x147
#define op_enbc          0x148  
#define op_enbs          0x149
#define op_move          0x14a
#define op_or            0x14b
#define op_csngl         0x14c
#define op_ccnt1         0x14d
#define op_talt          0x14e
#define op_ldiff         0x14f
#define op_sthb          0x150
#define op_taltwt        0x151
#define op_sum           0x152
#define op_mul           0x153
#define op_sttimer       0x154
#define op_stoperr       0x155
#define op_cword         0x156
#define op_clrhalterr    0x157
#define op_sethalterr    0x158
#define op_testhalterr   0x159
#define op_dup           0x15a    /* Only on T800 */
#define op_move2dinit    0x15b    /* Only on T800 */
#define op_move2dall     0x15c    /* Only on T800 */
#define op_move2dnonzero 0x15d    /* Only on T800 */
#define op_move2dzero    0x15e    /* Only on T800 */

#define op_unpacksn      0x163    /* Not on T800 */

#define op_postnormsn    0x16c    /* Not on T800 */
#define op_roundsn       0x16d    /* Not on T800 */

#define op_ldinf         0x171    /* Not on T800 */
#define op_fmul          0x172    /* Not on 16-bit Transputers (?) */
#define op_cflerr        0x173    /* Not on T800 */
#define op_crcword       0x174    /* Only on T800 */
#define op_crcbyte       0x175    /* Only on T800 */
#define op_bitcnt        0x176    /* Only on T800 */
#define op_bitrevword    0x177    /* Only on T800 */
#define op_bitrevnbits   0x178    /* Only on T800 */
#define op_pop		 0x179	  /* T425 only */
#define op_timerdisableh 0x17a	  /* T425 only */
#define op_timerdisablel 0x17b	  /* T425 only */
#define op_timerenableh  0x17c	  /* T425 only */
#define op_timerenablel  0x17d	  /* T425 only */
#define op_ldmemstartval 0x17e	  /* T425 only */

#define op_wsubdb        0x181    /* Only on T800 */
#define op_fpldndbi      0x182    /* All the rest are T800 only */
#define op_fpchkerr      0x183
#define op_fpstnldb      0x184

#define op_fpldnlsni     0x186
#define op_fpadd         0x187
#define op_fpstnlsn      0x188
#define op_fpsub         0x189
#define op_fpldnldb      0x18a
#define op_fpmul         0x18b
#define op_fpdiv         0x18c

#define op_fpldnlsn      0x18e
#define op_fpremfirst    0x18f
#define op_fpremstep     0x190
#define op_fpnan         0x191
#define op_fpordered     0x192
#define op_fpnotfinite   0x193
#define op_fpgt          0x194
#define op_fpeq          0x195
#define op_fpi32tor32    0x196

#define op_fpi32tor64    0x198

#define op_fpb32tor64    0x19a

#define op_fptesterr     0x19c
#define op_fprtoi32      0x19d
#define op_fpstnli32     0x19e
#define op_fpldzerosn    0x19f
#define op_fpldzerodb    0x1a0
#define op_fpint         0x1a1

#define op_fpdup         0x1a3
#define op_fprev         0x1a4

#define op_fpldnladddb   0x1a6

#define op_fpldnlmuldb   0x1a8

#define op_fpldnladdsn   0x1aa
#define op_fpentry       0x1ab
#define op_fpldnlmulsn   0x1ac

#define op_break         0x1b1	/* T425 only */
#define op_clrj0break    0x1b2	/* T425 only */
#define op_setj0break    0x1b3	/* T425 only */
#define op_testj0break   0x1b4	/* T425 only */

/* fpu operations are 0x200 + operand	*/

#define op_fpusqrtfirst   0x201
#define op_fpusqrtstep    0x202
#define op_fpusqrtlast    0x203
#define op_fpurp          0x204
#define op_fpurm          0x205
#define op_fpurz          0x206
#define op_fpur32tor64    0x207
#define op_fpur64tor32    0x208
#define op_fpuexpdec32    0x209
#define op_fpuexpinc32    0x20a
#define op_fpuabs         0x20b

#define op_fpunoround     0x20d
#define op_fpuchki32      0x20e
#define op_fpuchki64      0x20f

#define op_fpudivby2      0x211
#define op_fpumulby2      0x212

#define op_fpurn          0x222
#define op_fpuseterr      0x223

#define op_fpuclrerr      0x29c

/* the following are 3 byte operates */
#define op_start	  0x2ff
#define op_lddevid	  0x27c

/* pseudo operators, expand to real ones in back end */

#define p_infoline	0x300		/* line info		*/
#define p_call		0x304		/* fn call macro	*/
#define p_callpv	0x305		/* proc var call macro	*/
#define p_fnstack	0x306		/* stack adjustment	*/
#define p_ldx		0x307		/* load external	*/
#define p_stx		0x308		/* store external	*/
#define p_ldxp		0x309		/* load external pointer*/
#define p_ret		0x30a		/* return macro		*/
#define p_ldpi		0x30b		/* load pointer to code */
#define p_ldpf		0x30c		/* load pointer to func */
#define p_setv1		0x30d		/* initialise vector	*/
#define p_setv2		0x30e		/* initialise vector	*/
#define p_lds		0x30f		/* load static		*/
#define p_sts		0x310		/* store static		*/
#define p_ldsp		0x311		/* load static pointer	*/
#define p_j             0x312           /* ldc 0; cj            */
#define p_noop          0x313           /* no op                */
#define p_ldvl          0x314           /* load virtual local   */
#define p_ldvlp         0x315           /* load virtual local   */
#define p_stvl          0x316           /* store virtual local  */
#define p_fpcall	0x317		/* fp emulator call	*/
#define p_case		0x318		/* case jump table	*/
#define p_noop2		0x319		/* for non-empty blocks */

#endif

/* end of xpuops.h */
