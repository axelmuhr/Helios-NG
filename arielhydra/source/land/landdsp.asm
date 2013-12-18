******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -q -DUSE_INT landdsp.c C:\TMP\landdsp.if 
;	opt30 NOT RUN
;	cg30 -v40 -q -o -o C:\TMP\landdsp.if C:\TMP\landdsp.asm C:\TMP\landdsp.tmp 
	.version	40
FP	.set		AR3
	.file	"landdsp.c"
	.file	"C:\TIC40\INCLUDE\stdlib.h"
	.sym	_size_t,0,14,13,32
	.sym	_wchar_t,0,2,13,32

	.stag	__div_t,64
	.member	_quot,0,4,8,32
	.member	_rem,32,4,8,32
	.eos
	.sym	_div_t,0,8,13,64,__div_t
	.sym	_ldiv_t,0,8,13,64,__div_t
	.globl	_atof
	.globl	_atoi
	.globl	_atol
	.globl	_strtod
	.globl	_strtol
	.globl	_strtoul
	.globl	_rand
	.globl	_srand
	.globl	_calloc
	.globl	_free
	.globl	_malloc
	.globl	_minit
	.globl	_realloc
	.globl	_abort
	.globl	_exit
	.globl	_atexit
	.globl	_bsearch
	.globl	_qsort
	.globl	_abs
	.globl	_labs
	.globl	_div
	.globl	_ldiv
	.globl	_getenv
	.file	"landdsp.c"
	.sym	_u_long,0,15,13,32

	.sect	".cinit"
	.word	1,_VIC_virsr
	.word	-1073807328

	.sym	_VIC_virsr,_VIC_virsr,31,2,32
	.globl	_VIC_virsr
	.bss	_VIC_virsr,1

	.stag	_line3d,96
	.member	_flag_and_color,0,15,8,32
	.member	_start_point,32,15,8,32
	.member	_end_point,64,15,8,32
	.eos
	.globl	_testline
	.globl	_lines
	.globl	_addline
	.globl	_draw_lines

	.word	1,_nl
	.word	0

	.sym	_nl,_nl,4,2,32
	.globl	_nl
	.bss	_nl,1
	.globl	_DEEP
	.globl	_steep
	.globl	_sealevel
	.globl	_ybottom
	.globl	_WATERCOLOR
	.globl	_LANDCOLOR

	.word	1,_seed
	.word	0

	.sym	_seed,_seed,4,2,32
	.globl	_seed
	.bss	_seed,1

	.word	1,_start_flag
	.word	0

	.sym	_start_flag,_start_flag,4,2,32
	.globl	_start_flag
	.bss	_start_flag,1

	.word	1,_ready_flag
	.word	0

	.sym	_ready_flag,_ready_flag,4,2,32
	.globl	_ready_flag
	.bss	_ready_flag,1

	.word	1,_done_flag
	.word	0

	.sym	_done_flag,_done_flag,4,2,32
	.globl	_done_flag
	.bss	_done_flag,1
	.globl	_intpri
	.globl	_intvec
	.text

	.sym	_main,_main,36,2,0
	.globl	_main

	.func	51
;>>>> 	int main()
******************************************************
* FUNCTION DEF : _main
******************************************************
_main:
	PUSH	FP
	LDI	SP,FP
	.line	3
;>>>> 	    GIEOn();
	CALL	_GIEOn
	.line	4
;>>>> 	    testline.flag_and_color = 0x12345678;
	LDI	@CONST+0,R0
	STI	R0,@_testline
	.line	5
;>>>> 	    testline.start_point = 0x11112222;
	LDI	@CONST+1,R1
	STI	R1,@_testline+1
	.line	6
;>>>> 	    testline.end_point = 0x33334444;
	LDI	@CONST+2,R2
	STI	R2,@_testline+2
L1:
	.line	8
;>>>> 	    while (seed == 0) ;
	LDI	@_seed,R0
	BZ	L1
	.line	10
;>>>> 	    srand(seed);
;>>>> 	    while (1) {
	PUSH	R0
	CALL	_srand
	SUBI	1,SP
L4:
	.line	13
;>>>> 	        while (!start_flag) ;
	LDI	@_start_flag,R0
	BZ	L4
	.line	15
;>>>> 	        frac(DEEP, XA,YA,XB,ybottom,ZA,ZA,ZA,ZA);
	LDI	0,R0
	PUSH	R0
	PUSH	R0
	PUSH	R0
	PUSH	R0
	LDI	@_ybottom,R1
	PUSH	R1
	LDI	500,R2
	PUSH	R2
	PUSH	R0
	PUSH	R0
	LDI	@_DEEP,R3
	PUSH	R3
	CALL	_frac
	SUBI	9,SP
	.line	16
;>>>> 		if (nl > 0) draw_lines();
	LDI	@_nl,R0
	BLE	L5
	CALL	_draw_lines
L5:
	.line	17
;>>>> 		start_flag = 0;
	STIK	0,@_start_flag
	.line	18
;>>>> 		done_flag = 1;
	STIK	1,@_done_flag
	.line	19
	B	L4
	.endfunc	70,000000000H,0

	.sym	_frac,_frac,32,2,0
	.globl	_frac

	.func	73
;>>>> 	void frac(int depth, int x0, int y0, int x2, int y2, 
;>>>> 		int z0, int z1, int z2, int z3) 
******************************************************
* FUNCTION DEF : _frac
******************************************************
_frac:
	PUSH	FP
	LDI	SP,FP
	ADDI	7,SP
	.sym	_depth,-2,4,9,32
	.sym	_x0,-3,4,9,32
	.sym	_y0,-4,4,9,32
	.sym	_x2,-5,4,9,32
	.sym	_y2,-6,4,9,32
	.sym	_z0,-7,4,9,32
	.sym	_z1,-8,4,9,32
	.sym	_z2,-9,4,9,32
	.sym	_z3,-10,4,9,32
	.sym	_newz,1,4,1,32
	.sym	_xmid,2,4,1,32
	.sym	_ymid,3,4,1,32
	.sym	_z01,4,4,1,32
	.sym	_z12,5,4,1,32
	.sym	_z23,6,4,1,32
	.sym	_z30,7,4,1,32
	.line	3
;>>>> 	    int newz; /* new center point */
;>>>> 	    int xmid,ymid,z01,z12,z23,z30;
	.line	10
;>>>> 	    if (RANDOM() < 16384) {
	CALL	_rand
	ASH	-16,R0
	CMPI	16384,R0
	BGE	L6
	.line	11
;>>>> 	        newz = (z0+z1+z2+z3) / 4 + (int)((RANDOM() / 32768.0) * ((y2-y0)* steep));
;>>>> 	    else {
	CALL	_rand
	ASH	-16,R0
	FLOAT	R0
	MPYF	@CONST+3,R0
	LDI	*-FP(6),R1
	SUBI	*-FP(4),R1
	FLOAT	R1
	MPYF	@_steep,R1
	MPYF	R1,R0
	FIX	R0,R1
	NEGF	R0
	FIX	R0
	NEGI	R0
	LDILE	R0,R1
	LDI	*-FP(7),R0
	ADDI	*-FP(8),R0
	ADDI	*-FP(9),R0
	ADDI	*-FP(10),R0
	ASH	-2,R0
	ADDI	R1,R0
	STI	R0,*+FP(1)
	B	L7
L6:
	.line	14
;>>>> 	        newz = (z0+z1+z2+z3) / 4 - (int)((RANDOM() / 32768.0) * ((y2-y0)* steep));
	CALL	_rand
	ASH	-16,R0
	FLOAT	R0
	MPYF	@CONST+3,R0
	LDI	*-FP(6),R1
	SUBI	*-FP(4),R1
	FLOAT	R1
	MPYF	@_steep,R1
	MPYF	R1,R0
	FIX	R0,R1
	NEGF	R0
	FIX	R0
	NEGI	R0
	LDILE	R0,R1
	LDI	*-FP(7),R0
	ADDI	*-FP(8),R0
	ADDI	*-FP(9),R0
	ADDI	*-FP(10),R0
	ASH	-2,R0
	SUBI	R1,R0
	STI	R0,*+FP(1)
L7:
	.line	16
;>>>> 	    xmid = (x0+x2) >> 1;
	LDI	*-FP(3),R1
	ADDI	*-FP(5),R1
	ASH	-1,R1
	STI	R1,*+FP(2)
	.line	17
;>>>> 	    ymid = (y0+y2) >> 1;
	LDI	*-FP(4),R2
	ADDI	*-FP(6),R2
	ASH	-1,R2
	STI	R2,*+FP(3)
	.line	18
;>>>> 	    z12 = (z1+z2) >> 1;
	LDI	*-FP(8),R3
	ADDI	*-FP(9),R3
	ASH	-1,R3
	STI	R3,*+FP(5)
	.line	19
;>>>> 	    z30 = (z3+z0) >> 1;
	LDI	*-FP(10),R9
	ADDI	*-FP(7),R9
	ASH	-1,R9
	STI	R9,*+FP(7)
	.line	20
;>>>> 	    z01 = (z0+z1) >> 1;
	LDI	*-FP(7),R10
	ADDI	*-FP(8),R10
	ASH	-1,R10
	STI	R10,*+FP(4)
	.line	21
;>>>> 	    z23 = (z2+z3) >> 1;
	LDI	*-FP(9),R11
	ADDI	*-FP(10),R11
	ASH	-1,R11
	STI	R11,*+FP(6)
	.line	22
;>>>> 	    depth--;
	LDI	*-FP(2),R11
	SUBI	1,R11
	STI	R11,*-FP(2)
	.line	23
;>>>> 	    if (depth>=0 ) {
	BLT	L8
	.line	24
;>>>> 	        frac(depth, x0,y0, xmid,ymid, z0,z01,newz,z30);
	PUSH	R9
	PUSH	R0
	PUSH	R10
	LDI	*-FP(7),R11
	PUSH	R11
	PUSH	R2
	PUSH	R1
	LDI	*-FP(4),R11
	PUSH	R11
	LDI	*-FP(3),R11
	PUSH	R11
	LDI	*-FP(2),R11
	PUSH	R11
	CALL	_frac
	SUBI	9,SP
	.line	25
;>>>> 	        frac(depth, xmid,y0, x2,ymid, z01,z1,z12,newz);
	LDI	*+FP(1),R0
	PUSH	R0
	LDI	*+FP(5),R1
	PUSH	R1
	LDI	*-FP(8),R2
	PUSH	R2
	LDI	*+FP(4),R3
	PUSH	R3
	LDI	*+FP(3),R9
	PUSH	R9
	LDI	*-FP(5),R10
	PUSH	R10
	LDI	*-FP(4),R11
	PUSH	R11
	LDI	*+FP(2),R11
	PUSH	R11
	LDI	*-FP(2),R11
	PUSH	R11
	CALL	_frac
	SUBI	9,SP
	.line	26
;>>>> 	        frac(depth, x0,ymid, xmid,y2, z30,newz,z23,z3);
	LDI	*-FP(10),R0
	PUSH	R0
	LDI	*+FP(6),R1
	PUSH	R1
	LDI	*+FP(1),R2
	PUSH	R2
	LDI	*+FP(7),R3
	PUSH	R3
	LDI	*-FP(6),R9
	PUSH	R9
	LDI	*+FP(2),R10
	PUSH	R10
	LDI	*+FP(3),R11
	PUSH	R11
	LDI	*-FP(3),R11
	PUSH	R11
	LDI	*-FP(2),R11
	PUSH	R11
	CALL	_frac
	SUBI	9,SP
	.line	27
;>>>> 	        frac(depth, xmid,ymid, x2,y2, newz,z12,z2,z23);
;>>>> 	    else {
	LDI	*+FP(6),R0
	PUSH	R0
	LDI	*-FP(9),R1
	PUSH	R1
	LDI	*+FP(5),R2
	PUSH	R2
	LDI	*+FP(1),R3
	PUSH	R3
	LDI	*-FP(6),R9
	PUSH	R9
	LDI	*-FP(5),R10
	PUSH	R10
	LDI	*+FP(3),R11
	PUSH	R11
	LDI	*+FP(2),R11
	PUSH	R11
	LDI	*-FP(2),R11
	PUSH	R11
	CALL	_frac
	SUBI	9,SP
	B	EPI0_2
L8:
	.line	30
;>>>> 	        if (newz<=sealevel ) { /*above sea level*/
	CMPI	@_sealevel,R0
	BGT	L10
	.line	32
;>>>> 	            addline(xmid,ymid,newz, x2,ymid,z12);
	PUSH	R3
	PUSH	R2
	LDI	*-FP(5),R11
	PUSH	R11
	PUSH	R0
	PUSH	R2
	PUSH	R1
	CALL	_addline
	SUBI	6,SP
	.line	33
;>>>> 	            addline(xmid,ymid,newz, x0,ymid,z30);
;>>>> 	        else {
	LDI	*+FP(7),R0
	PUSH	R0
	LDI	*+FP(3),R1
	PUSH	R1
	LDI	*-FP(3),R2
	PUSH	R2
	LDI	*+FP(1),R3
	PUSH	R3
	PUSH	R1
	LDI	*+FP(2),R9
	PUSH	R9
	CALL	_addline
	SUBI	6,SP
	B	EPI0_2
L10:
	.line	37
;>>>> 	            addline(xmid,ymid,sealevel, 0,0,-9999);
	LDI	-9999,R11
	PUSH	R11
	LDI	0,R11
	PUSH	R11
	PUSH	R11
	LDI	@_sealevel,R11
	PUSH	R11
	PUSH	R2
	PUSH	R1
	CALL	_addline
	SUBI	6,SP
EPI0_2:
	.line	40
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	9,SP
	B	R1
	.endfunc	112,000000000H,7

	.sym	_addline,_addline,32,2,0
	.globl	_addline

	.func	114
;>>>> 	void addline(int x0, int y0, int z0, int x1, int y1, int z1)
******************************************************
* FUNCTION DEF : _addline
******************************************************
_addline:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
	PUSH	R4
	PUSH	R5
	PUSH	R8
	PUSH	AR4
	.sym	_x0,-2,4,9,32
	.sym	_y0,-3,4,9,32
	.sym	_z0,-4,4,9,32
	.sym	_x1,-5,4,9,32
	.sym	_y1,-6,4,9,32
	.sym	_z1,-7,4,9,32
	.sym	_tt,4,4,4,32
	.sym	_xs,5,4,4,32
	.sym	_ys,22,4,4,32
	.sym	_xe,1,4,1,32
	.sym	_ye,2,4,1,32
	.sym	_tline,12,24,4,32,_line3d
	.line	2
;>>>> 	    register int tt;
;>>>> 	    register int xs, ys, xe, ye;
	.line	5
;>>>> 	    register struct line3d *tline = &lines[nl];
	LDA	@_nl,AR4
	MPYI	3,AR4
	ADDI	@CONST+4,AR4
	.line	7
;>>>> 	    if (z1 == -9999) {
	LDI	*-FP(7),R0
	CMPI	-9999,R0
	BNZ	L12
	.line	8
;>>>> 	        tline->flag_and_color = 0x10000 | WATERCOLOR;
	LDI	@_WATERCOLOR,R1
	OR	@CONST+5,R1
	STI	R1,*AR4
	.line	9
;>>>> 	        xs = (y0 >> 1) + x0;
	LDI	*-FP(3),R5
	ASH	-1,R5
	ADDI	*-FP(2),R5
	.line	10
;>>>> 	        ys = YADD + y0 + z0;
	LDI	*-FP(3),R8
	ADDI	*-FP(4),R8
	ADDI	38,R8
	.line	11
;>>>> 	        tline->start_point = (xs << 16) | ys;
;>>>> 	    else {
	LDI	16,R1
	LSH	R1,R5,R2
	OR	R8,R2
	STI	R2,*+AR4(1)
	B	L13
L12:
	.line	14
;>>>> 	        tt = (abs(z1) >> 1) + 172;
	ABSI	R0,R4
	ASH	-1,R4
	ADDI	172,R4
	.line	15
;>>>> 		if(tt > 255) {
	CMPI	255,R4
	BLE	L14
	.line	16
;>>>> 		    tt = 255;
	LDI	255,R4
L14:
	.line	18
;>>>> 	        tline->flag_and_color = tt;
	STI	R4,*AR4
	.line	19
;>>>> 	        xs = (y0 >> 1) + x0;
	LDI	*-FP(3),R5
	ASH	-1,R5
	ADDI	*-FP(2),R5
	.line	20
;>>>> 	        ys = YADD + y0 + z0;
	LDI	*-FP(3),R8
	ADDI	*-FP(4),R8
	ADDI	38,R8
	.line	21
;>>>> 	        xe = (y1 >> 1) + x1;
	LDI	*-FP(6),R1
	ASH	-1,R1,R2
	ADDI	*-FP(5),R2
	STI	R2,*+FP(1)
	.line	22
;>>>> 	        ye = (YADD + y1 + z1);
	ADDI	R0,R1,R3
	ADDI	38,R3
	STI	R3,*+FP(2)
	.line	23
;>>>> 	        tline->start_point = (xs << 16) | ys;
	LDI	16,R9
	LSH	R9,R5,R10
	OR	R8,R10
	STI	R10,*+AR4(1)
	.line	24
;>>>> 	        tline->end_point = (xe << 16) | ye;
	LSH	R9,R2,R10
	OR	R3,R10
	STI	R10,*+AR4(2)
L13:
	.line	26
;>>>> 	    nl++;
	LDI	@_nl,R1
	ADDI	1,R1,R2
	STI	R2,@_nl
	.line	27
;>>>> 	    if (nl == NLINES) {
	CMPI	4000,R2
	BNZ	EPI0_3
	.line	28
;>>>> 	        draw_lines();
	CALL	_draw_lines
	.line	29
;>>>> 	        nl = 0;
	STIK	0,@_nl
EPI0_3:
	.line	31
	LDI	*-FP(1),R1
	LDI	*FP,FP
	POP	AR4
	POP	R8
	POP	R5
	POP	R4
	SUBI	4,SP
	B	R1
	.endfunc	144,000401030H,2

	.sym	_draw_lines,_draw_lines,32,2,0
	.globl	_draw_lines

	.func	147
;>>>> 	void draw_lines()
;>>>> 	    struct line3d *tline;
;>>>> 	    static int i;
******************************************************
* FUNCTION DEF : _draw_lines
******************************************************
_draw_lines:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP

	.sym	_i,STATIC_1,4,3,32
	.bss	STATIC_1,1
	.sym	_tline,1,24,1,32,_line3d
	.line	6
;>>>> 	    ready_flag = 1;
;>>>> 	#ifdef USE_INT
	STIK	1,@_ready_flag
	.line	9
;>>>> 	    HOST_INTERRUPT();
;>>>> 	#endif
	LDA	@_VIC_virsr,AR0
	LDA	@_intpri,IR0
	LDI	@_intvec,R0
	STI	R0,*+AR0(IR0)
	LDI	1,R1
	LSH	@_intpri,R1
	ADDI	1,R1
	STI	R1,*AR0
L16:
	.line	12
;>>>> 	    while (ready_flag) ;
	LDI	@_ready_flag,R0
	BNZ	L16
	.line	14
;>>>> 	    for (i=0; i<10000;) {
	STIK	0,@STATIC_1
	LDI	@STATIC_1,R0
	CMPI	10000,R0
	BGE	EPI0_4
L17:
	.line	15
;>>>> 	    	i++;
	LDI	@STATIC_1,R0
	ADDI	1,R0,R1
	STI	R1,@STATIC_1
	.line	14
	CMPI	10000,R1
	BLT	L17
EPI0_4:
	.line	17
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	163,000000000H,1

	.sym	_lines,_lines,56,2,384000,_line3d,4000
	.globl	_lines
	.bss	_lines,12000

	.sym	_intpri,_intpri,4,2,32
	.globl	_intpri
	.bss	_intpri,1

	.sym	_LANDCOLOR,_LANDCOLOR,4,2,32
	.globl	_LANDCOLOR
	.bss	_LANDCOLOR,1

	.sym	_testline,_testline,8,2,96,_line3d
	.globl	_testline
	.bss	_testline,3

	.sym	_WATERCOLOR,_WATERCOLOR,4,2,32
	.globl	_WATERCOLOR
	.bss	_WATERCOLOR,1

	.sym	_intvec,_intvec,4,2,32
	.globl	_intvec
	.bss	_intvec,1

	.sym	_DEEP,_DEEP,4,2,32
	.globl	_DEEP
	.bss	_DEEP,1

	.sym	_ybottom,_ybottom,4,2,32
	.globl	_ybottom
	.bss	_ybottom,1

	.sym	_steep,_steep,6,2,32
	.globl	_steep
	.bss	_steep,1

	.sym	_sealevel,_sealevel,4,2,32
	.globl	_sealevel
	.bss	_sealevel,1
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,6
	.sect	".cinit"
	.word	6,CONST
	.word 	305419896        ;0
	.word 	286335522        ;1
	.word 	858997828        ;2
	.float	3.0517578125e-5  ;3
	.word 	_lines           ;4
	.word 	010000h          ;5
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_GIEOn
	.end
