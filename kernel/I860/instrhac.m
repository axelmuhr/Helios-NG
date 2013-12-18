_report ['include instrhac.m]
_def 'instrhac.m_flag 1

--
-- calli doesn't work yet     Errata Page 18 
--
--  CALLI   reg
--
_defq  'CALLI['label] [
$label$_1      ld.c     fir,r1
         addu     $label$_1 - $label$_2 + 4,r1,r1
         BRI      label
$label$_2
	]

--BUG17 equ 1
--
-- Branches don't work properly either   Errata Page 11 No. 18
--
--         ifd   BUG17
--BRANCHHACK: macro
--         st.b     r0,0(r0)   -- somewhere_or_other
--         nop
--	 NOPALIGN
--endm
--         else
--
-- We still have to ensure no branches on ..11000 boundaries
--
_def 'BRANCHHACK [
	NOPALIGN
	]

_defq 'BC['label] [
	BRANCHHACK
        bc       $label
	]

_defq    'BNC['label] [
         BRANCHHACK
         bnc      $label
	]

_defq	'BCT['label] [
         BRANCHHACK
         bc.t     $label
	]

_defq	'BNCT['label] [
         BRANCHHACK
         bnc.t    $label
	]

_defq 	'BR['label] [
         BRANCHHACK
         br       $label
	]

_defq	'BRI['label] [
         BRANCHHACK
         bri      $label
	]

_defq	'CALL['label] [
         BRANCHHACK
         call     $label
	]

_defq	'BLA['arg1 'arg2 'arg3] [
         BRANCHHACK
         bla      $arg1,$arg2,$arg3
	]

--
-- bte & btne don't work but in a different way! Errata Page 14 No. 29
--
_defq    'BTE['isc1 'isc2 'label] [
         xor      $isc1,$isc2,r0
         BC       $label          -- Must not be at PC{4:0} = 0x18
	]

_defq	'BTNE['isc1 'isc2 'label] [
         xor      $isc1,$isc2,r0
         BNC      $label          -- Must not be at PC{4:0} = 0x18
	]

_defq	'LOADC['const 'src] [
         if       ($const) & 0xffff
            or    ($const) & 0xffff,r0,$src
            if    ($const) & 0xffff0000
            	orh   (($const$)>>16) & 0xffff,$src,$src
            endc
         else
            orh   (($const$)>>16) & 0xffff,r0,$src
         endc
	]
--
-- LEA   symbol,reg
--
_defq	'LEA['arg1 'arg2] [
1$@:     ld.c  fir,$arg2
         addu  $arg1-1$@,$arg2,$arg2
	]

--
-- BUG6 (see Error sheet page 8)
--
_defq	'BUG6 [	st.b	r0,0(r0)]


--
-- Align the PC on a 2^n boundary
--
_defq   'PCALIGN['arg ] [
         if    ((.-.ModStart)&($arg-1))!=0
           ds.b  $arg-((.-.ModStart)&($arg-1))
         endc
]

--
-- Align the PC on a 32 boundary  ?? 
--
_defq	'NOPALIGN [
         if    ((.-.ModStart)&(0x1f))==0x18
           nop
         endc
	]


