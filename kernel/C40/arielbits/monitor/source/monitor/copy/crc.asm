******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 crc.c crc.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o crc.if crc.asm crc.tmp 
	.version	40
FP	.set		AR3
	.file	"crc.c"
	.file	"hydra.h"

	.stag	.fake0,96
	.member	_baud,0,15,8,32
	.member	_parity,32,4,8,32
	.member	_bits,64,4,8,32
	.eos
	.sym	_UART_config,0,8,13,96,.fake0

	.stag	.fake1,512
	.member	_uartA,0,8,8,96,.fake0
	.member	_uartB,96,8,8,96,.fake0
	.member	_dram_size,192,15,8,32
	.member	_cpu_clock,224,15,8,32
	.member	_checksum,256,15,8,32
	.member	_sram1_size,288,15,8,32
	.member	_sram2_size,320,15,8,32
	.member	_sram3_size,352,15,8,32
	.member	_sram4_size,384,15,8,32
	.member	_l_dram_base,416,15,8,32
	.member	_l_jtag_base,448,15,8,32
	.member	_daughter,480,15,8,32
	.eos
	.sym	_hydra_conf,0,8,13,512,.fake1

	.stag	.fake2,128
	.member	_FailAddress,0,31,8,32
	.member	_TestFailed,32,4,8,32
	.member	_Written,64,15,8,32
	.member	_Read,96,15,8,32
	.eos
	.sym	_MemTestStruct,0,8,13,128,.fake2

	.stag	.fake3,64
	.member	_header,0,15,8,32
	.member	_data,32,15,8,32
	.eos
	.sym	_CommMessage,0,8,13,64,.fake3

	.stag	.fake4,1504
	.member	_DSP_ir0,0,15,8,32
	.member	_DSP_ir1,32,15,8,32
	.member	_DSP_ir2,64,15,8,32
	.member	_DSP_ir3,96,15,8,32
	.member	_DSP_ir4,128,15,8,32
	.member	_DSP_ir5,160,15,8,32
	.member	_DSP_ir6,192,15,8,32
	.member	_DSP_ir7,224,15,8,32
	.member	_DSP_ir8,256,15,8,32
	.member	_DSP_ir9,288,15,8,32
	.member	_DSP_ir10,320,15,8,32
	.member	_DSP_ir11,352,15,8,32
	.member	_DSP_fr0,384,15,8,32
	.member	_DSP_fr1,416,15,8,32
	.member	_DSP_fr2,448,15,8,32
	.member	_DSP_fr3,480,15,8,32
	.member	_DSP_fr4,512,15,8,32
	.member	_DSP_fr5,544,15,8,32
	.member	_DSP_fr6,576,15,8,32
	.member	_DSP_fr7,608,15,8,32
	.member	_DSP_fr8,640,15,8,32
	.member	_DSP_fr9,672,15,8,32
	.member	_DSP_fr10,704,15,8,32
	.member	_DSP_fr11,736,15,8,32
	.member	_DSP_ar0,768,15,8,32
	.member	_DSP_ar1,800,15,8,32
	.member	_DSP_ar2,832,15,8,32
	.member	_DSP_ar3,864,15,8,32
	.member	_DSP_ar4,896,15,8,32
	.member	_DSP_ar5,928,15,8,32
	.member	_DSP_ar6,960,15,8,32
	.member	_DSP_ar7,992,15,8,32
	.member	_DSP_DP,1024,15,8,32
	.member	_DSP_IR0,1056,15,8,32
	.member	_DSP_IR1,1088,15,8,32
	.member	_DSP_BK,1120,15,8,32
	.member	_DSP_SP,1152,15,8,32
	.member	_DSP_ST,1184,15,8,32
	.member	_DSP_DIE,1216,15,8,32
	.member	_DSP_IIE,1248,15,8,32
	.member	_DSP_IIF,1280,15,8,32
	.member	_DSP_RS,1312,15,8,32
	.member	_DSP_RE,1344,15,8,32
	.member	_DSP_RC,1376,15,8,32
	.member	_DSP_IVTP,1408,15,8,32
	.member	_DSP_TVTP,1440,15,8,32
	.member	_ret_add,1472,15,8,32
	.eos
	.sym	_reg_set,0,8,13,1504,.fake4
	.globl	_setup
	.globl	_init
	.globl	_bus_error_handler
	.globl	_c_int01
	.globl	_monitor
	.globl	_compare
	.globl	_dump
	.globl	_fill
	.globl	_enter
	.globl	_copy
	.globl	_search
	.globl	_help
	.globl	_ReadEEPROM
	.globl	_WriteEEPROM
	.globl	_c40_printf
	.globl	_putstr
	.globl	_c40_putchar
	.globl	_ftoa
	.globl	_send_host
	.globl	_comm_sen
	.globl	_comm_rec
	.globl	_com_init
	.globl	_o_crdy
	.globl	_i_crdy
	.globl	_xtoa
	.globl	_atox
	.globl	_atod
	.globl	_key_handler
	.globl	_c40_getchar
	.globl	_iswhite
	.globl	_led
	.globl	_configure_uart
	.globl	_get_baud
	.globl	_get_parity
	.globl	_parity_enable
	.globl	_parity_type
	.globl	_get_bits
	.globl	_get_addr
	.globl	_get_clock
	.globl	_get_sram
	.globl	_get_daughter
	.globl	_get_dram
	.globl	_read_config
	.globl	_display_conf
	.globl	_write_config
	.globl	_update_chksum
	.globl	_crcupdate
	.globl	_x_rcvcrc
	.globl	_x_sndcrc
	.globl	_crchware
	.globl	_mk_crctbl
	.globl	_zero_regs
	.globl	_resume_mon
	.globl	_set_brk
	.globl	_del_brk
	.globl	_list_brks
	.globl	_reg_dump
	.globl	_print_reg
	.globl	_step
	.globl	_go
	.globl	_run
	.globl	_writeVIC
	.globl	_readVIC
	.globl	_writeVAC
	.globl	_readVAC
	.globl	_readVACEPROM
	.globl	_test
	.globl	_reset_others
	.globl	_CommTest
	.globl	_BootOthers
	.file	"crc.c"
	.globl	_crctab

	.sym	_crcupdate,_crcupdate,32,2,0
	.globl	_crcupdate

	.func	7
;>>>> 	void crcupdate( unsigned long data, unsigned long *accum )
******************************************************
* FUNCTION DEF : _crcupdate
******************************************************
_crcupdate:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_data,-2,15,9,32
	.sym	_accum,-3,31,9,32
	.sym	_comb_val,1,15,1,32
	.line	2
;>>>> 		unsigned long comb_val;
	.line	5
;>>>> 		comb_val = (*accum >> 8) ^ (data & 0xFF);
	LDA	*-FP(3),AR0
	LSH	-8,*AR0,R0
	LDI	*-FP(2),R1
	AND	0ffh,R1
	XOR	R1,R0
	STI	R0,*+FP(1)
	.line	6
;>>>> 		*accum = (*accum << 8) ^ crctab[comb_val];
	LSH	8,*AR0,R1
	LDA	*+FP(1),IR0
	LDA	@CONST+0,AR1
	XOR	*+AR1(IR0),R1
	STI	R1,*AR0
EPI0_1:
	.line	7
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	13,000000000H,1

	.sym	_x_rcvcrc,_x_rcvcrc,47,2,0
	.globl	_x_rcvcrc

	.func	16
;>>>> 	unsigned long x_rcvcrc( unsigned long *data, int data_size )
******************************************************
* FUNCTION DEF : _x_rcvcrc
******************************************************
_x_rcvcrc:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
	.sym	_data,-2,31,9,32
	.sym	_data_size,-3,4,9,32
	.sym	_i,1,15,1,32
	.sym	_accum,2,15,1,32
	.line	2
;>>>> 		unsigned long i, accum;
	.line	5
;>>>> 		for( accum=i=0 ; i < data_size+2 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
	STI	R0,*+FP(2)
	LDI	*-FP(3),R1
	ADDI	2,R1
	CMPI	R1,R0
	BHS	L2
L1:
	.line	6
;>>>> 			crcupdate( *data++, &accum );
	LDI	FP,R0
	ADDI	2,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	LDI	*AR0++,R0
	STI	AR0,*-FP(2)
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
	.line	5
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	LDI	*-FP(3),R1
	ADDI	2,R1
	CMPI	R1,R0
	BLO	L1
L2:
	.line	7
;>>>> 		return( accum );
	LDI	*+FP(2),R0
EPI0_2:
	.line	8
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	4,SP
	B	R1
	.endfunc	23,000000000H,2

	.sym	_x_sndcrc,_x_sndcrc,47,2,0
	.globl	_x_sndcrc

	.func	27
;>>>> 	unsigned long x_sndcrc( unsigned long *buff, int data_size )
******************************************************
* FUNCTION DEF : _x_sndcrc
******************************************************
_x_sndcrc:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
	.sym	_buff,-2,31,9,32
	.sym	_data_size,-3,4,9,32
	.sym	_i,1,15,1,32
	.sym	_accum,2,15,1,32
	.line	2
;>>>> 		unsigned long i, accum;
	.line	5
;>>>> 		for( accum=i=0 ; i < data_size ; ++i )
	LDI	0,R0
	STI	R0,*+FP(1)
	STI	R0,*+FP(2)
	CMPI	*-FP(3),R0
	BHS	L4
L3:
	.line	6
;>>>> 			crcupdate( *buff++, &accum );
	LDI	FP,R0
	ADDI	2,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	LDI	*AR0++,R0
	STI	AR0,*-FP(2)
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
	.line	5
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	*-FP(3),R0
	BLO	L3
L4:
	.line	9
;>>>> 		return( (accum >> 8) + (accum << 8) );
	LSH	-8,*+FP(2),R0
	LSH	8,*+FP(2),R1
	ADDI	R1,R0
EPI0_3:
	.line	10
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	4,SP
	B	R1
	.endfunc	36,000000000H,2

	.sym	_crchware,_crchware,47,2,0
	.globl	_crchware

	.func	42
;>>>> 	unsigned long crchware( unsigned long data, unsigned long genpoly, unsigned long accum )
******************************************************
* FUNCTION DEF : _crchware
******************************************************
_crchware:
	PUSH	FP
	LDI	SP,FP
	.sym	_data,-2,15,9,32
	.sym	_genpoly,-3,15,9,32
	.sym	_accum,-4,15,9,32

	.sym	_i,STATIC_1,4,3,32
	.bss	STATIC_1,1
	.line	2
;>>>> 		static int i;
	.line	5
;>>>> 		data <<= 8;
	LDI	*-FP(2),R0
	LSH	8,R0,R1
	STI	R1,*-FP(2)
	.line	6
;>>>> 		for( i=8 ; i > 0 ; i-- )
	STIK	8,@STATIC_1
	LDI	@STATIC_1,R0
	BLE	L6
L5:
	.line	8
;>>>> 			if( (data^accum) & 0x8000 )
	LDI	*-FP(2),R0
	XOR	*-FP(4),R0
	TSTB	@CONST+1,R0
	BZ	L7
	.line	9
;>>>> 				accum = (accum << 1) ^ genpoly;
;>>>> 			else
	LDI	*-FP(4),R0
	LSH	1,R0,R1
	XOR	*-FP(3),R1
	STI	R1,*-FP(4)
	B	L8
L7:
	.line	11
;>>>> 				accum <<= 1;
	LDI	*-FP(4),R0
	LSH	1,R0,R1
	STI	R1,*-FP(4)
L8:
	.line	13
;>>>> 			data <<= 1;
	LDI	*-FP(2),R0
	LSH	1,R0,R2
	STI	R2,*-FP(2)
	.line	6
	LDI	@STATIC_1,R0
	SUBI	1,R0,R3
	STI	R3,@STATIC_1
	BGT	L5
L6:
	.line	16
;>>>> 		return( accum );
	LDI	*-FP(4),R0
EPI0_4:
	.line	17
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	58,000000000H,0

	.sym	_mk_crctbl,_mk_crctbl,32,2,0
	.globl	_mk_crctbl

	.func	64
;>>>> 	void mk_crctbl( unsigned long poly, unsigned long (*crcfn)(unsigned long,unsigned long,unsigned long) )
******************************************************
* FUNCTION DEF : _mk_crctbl
******************************************************
_mk_crctbl:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_poly,-2,15,9,32
	.sym	_crcfn,-3,159,9,32
	.sym	_i,1,4,1,32
	.line	2
;>>>> 		int i;
	.line	5
;>>>> 		for( i=0 ; i < 256 ; i++ )
	STIK	0,*+FP(1)
	LDI	256,R0
	CMPI	R0,*+FP(1)
	BGE	EPI0_5
L9:
	.line	6
;>>>> 			crctab[i] = (*crcfn)( i, poly, 0 );
	LDI	0,R0
	PUSH	R0
	LDI	*-FP(2),R1
	PUSH	R1
	LDI	*+FP(1),R2
	PUSH	R2
	LDI	*-FP(3),R3
	CALLU	R3
	SUBI	3,SP
	LDA	*+FP(1),IR1
	LDA	@CONST+0,AR0
	STI	R0,*+AR0(IR1)
	.line	5
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	256,R0
	BLT	L9
EPI0_5:
	.line	7
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	70,000000000H,1
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,2
	.sect	".cinit"
	.word	2,CONST
	.word 	_crctab          ;0
	.word 	32768            ;1
	.end
