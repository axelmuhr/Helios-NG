******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 com_port.c com_port.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o com_port.if com_port.asm com_port.tmp 
	.version	40
FP	.set		AR3
	.file	"com_port.c"
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
	.file	"com_port.c"

	.sym	_comm_sen,_comm_sen,36,2,0
	.globl	_comm_sen

	.func	195
;>>>> 	  int comm_sen(int no, unsigned long omessage, int tries)
******************************************************
* FUNCTION DEF : _comm_sen
******************************************************
_comm_sen:
	PUSH	FP
	LDI	SP,FP
	.sym	_no,-2,4,9,32
	.sym	_omessage,-3,15,9,32
	.sym	_tries,-4,4,9,32
	.line	2
	.line	3
;>>>> 	     if (o_crdy(no, tries))
	LDI	*-FP(4),R0
	PUSH	R0
	LDI	*-FP(2),R1
	PUSH	R1
	CALL	_o_crdy
	SUBI	2,SP
	CMPI	0,R0
	BZ	L1
	.line	5
;>>>> 	        *((unsigned long *)0x100042 + (no*0x10)) = omessage;
	LDA	*-FP(2),IR0
	LSH	4,IR0,IR1
	LDA	IR1,AR0
	LDI	*-FP(3),R0
	LDA	@CONST+0,IR1
	STI	R0,*+AR0(IR1)
	.line	6
;>>>> 	        return  1;
;>>>> 	     else
	LDI	1,R0
	B	EPI0_1
L1:
	.line	10
;>>>> 	        return  0;
	LDI	0,R0
EPI0_1:
	.line	11
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	205,000000000H,0

	.sym	_comm_rec,_comm_rec,36,2,0
	.globl	_comm_rec

	.func	207
;>>>> 	   int comm_rec(int no, unsigned long *imessage, int tries)
******************************************************
* FUNCTION DEF : _comm_rec
******************************************************
_comm_rec:
	PUSH	FP
	LDI	SP,FP
	.sym	_no,-2,4,9,32
	.sym	_imessage,-3,31,9,32
	.sym	_tries,-4,4,9,32
	.line	2
	.line	3
;>>>> 	      if (i_crdy(no, tries))
	LDI	*-FP(4),R0
	PUSH	R0
	LDI	*-FP(2),R1
	PUSH	R1
	CALL	_i_crdy
	SUBI	2,SP
	CMPI	0,R0
	BZ	L2
	.line	5
;>>>> 	          *imessage = *((unsigned long *)0x100041 + (no*0x10));
	LDA	*-FP(2),IR1
	LSH	4,IR1,IR0
	LDA	IR0,AR0
	LDA	*-FP(3),AR1
	LDA	@CONST+1,IR0
	LDI	*+AR0(IR0),R0
	STI	R0,*AR1
	.line	6
;>>>> 	          return  1;
;>>>> 	      else
	LDI	1,R0
	B	EPI0_2
L2:
	.line	9
;>>>> 	          return  0;
	LDI	0,R0
EPI0_2:
	.line	10
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	216,000000000H,0

	.sym	_com_init,_com_init,32,2,0
	.globl	_com_init

	.func	218
;>>>> 	    void com_init(void)
******************************************************
* FUNCTION DEF : _com_init
******************************************************
_com_init:
	PUSH	FP
	LDI	SP,FP
EPI0_3:
	.line	3
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	220,000000000H,0

	.sym	_o_crdy,_o_crdy,36,2,0
	.globl	_o_crdy

	.func	227
;>>>> 	int o_crdy( int channel, int tries )
******************************************************
* FUNCTION DEF : _o_crdy
******************************************************
_o_crdy:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_channel,-2,4,9,32
	.sym	_tries,-3,4,9,32
	.sym	_i,1,4,1,32
	.line	2
;>>>> 		int i;
	.line	5
;>>>> 		if( tries )
	LDI	*-FP(3),R0
	BZ	L7
	.line	7
;>>>> 			for( i=0 ; i < tries ; i++ )
	STIK	0,*+FP(1)
	CMPI	R0,*+FP(1)
	BGE	L5
L4:
	.line	8
;>>>> 				if( (*(unsigned long *)(0x100040 + (channel* 0x10)) & 0x1e0) != 0x1e0 )
	LDA	*-FP(2),AR0
	LSH	4,AR0,AR1
	LDA	@CONST+2,IR0
	LDI	*+AR1(IR0),R0
	AND	01e0h,R0
	CMPI	480,R0
	BZ	L6
	.line	9
;>>>> 					return( 1 );
	LDI	1,R0
	B	EPI0_4
L6:
	.line	7
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	*-FP(3),R0
	BLT	L4
L5:
	.line	10
;>>>> 			return( 0 );
;>>>> 		else
	LDI	0,R0
	B	EPI0_4
L7:
	.line	14
;>>>> 			while( (*(unsigned long *)(0x100040 + (channel* 0x10)) & 0x1e0) == 0x1e0 );
	LDA	*-FP(2),AR0
	LSH	4,AR0,AR1
	LDA	@CONST+2,IR1
	LDI	*+AR1(IR1),R0
	AND	01e0h,R0
	CMPI	480,R0
	BZ	L7
	.line	16
;>>>> 			return( 1 );
	LDI	1,R0
EPI0_4:
	.line	18
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	244,000000000H,1

	.sym	_i_crdy,_i_crdy,36,2,0
	.globl	_i_crdy

	.func	247
;>>>> 	int i_crdy( int channel, int tries )
******************************************************
* FUNCTION DEF : _i_crdy
******************************************************
_i_crdy:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_channel,-2,4,9,32
	.sym	_tries,-3,4,9,32
	.sym	_i,1,4,1,32
	.line	2
;>>>> 		int i;
	.line	5
;>>>> 		if( tries )
	LDI	*-FP(3),R0
	BZ	L12
	.line	7
;>>>> 			for( i=0 ; i < tries ; i++ )
	STIK	0,*+FP(1)
	CMPI	R0,*+FP(1)
	BGE	L10
L9:
	.line	8
;>>>> 				if( *(unsigned long *)(0x100040 + (channel* 0x10)) & 0x1e00 )
	LDA	*-FP(2),AR0
	LSH	4,AR0,AR1
	LDA	@CONST+2,IR0
	LDI	*+AR1(IR0),R0
	TSTB	7680,R0
	BZ	L11
	.line	9
;>>>> 					return( 1 );
	LDI	1,R0
	B	EPI0_5
L11:
	.line	7
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	*-FP(3),R0
	BLT	L9
L10:
	.line	11
;>>>> 			return( 0 );
;>>>> 		else
	LDI	0,R0
	B	EPI0_5
L12:
	.line	15
;>>>> 			while( (!*(unsigned long *)(0x100040 + (channel* 0x10)) & 0x1e00) );
	LDA	*-FP(2),AR0
	LSH	4,AR0,AR1
	LDA	@CONST+2,IR1
	LDI	*+AR1(IR1),R0
	LDIU	0,R0
	LDIZ	1,R0
	TSTB	7680,R0
	BNZ	L12
	.line	17
;>>>> 			return( 1 );
	LDI	1,R0
EPI0_5:
	.line	19
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	265,000000000H,1
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,3
	.sect	".cinit"
	.word	3,CONST
	.word 	1048642          ;0
	.word 	1048641          ;1
	.word 	1048640          ;2
	.end
