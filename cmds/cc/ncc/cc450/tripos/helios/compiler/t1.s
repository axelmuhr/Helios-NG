*
*
000000                 __codeseg:
000000   0000 0000      ORI.B   #0,D0
*
*
000004     7a000000     DC.L    $7A000000         ; 'z\0\0\0'
000008     ff000004     DC.L    $FF000004
*
*
00000c                 z:
00000c   2f09           MOVE.L  A1,-(A7)
00000e   e588           LSL.L   #2,D0
000010   226d 0000      MOVEA.L 0(A5),A1
000014   43e9 0000      LEA     0(A1),A1
000018   2251           MOVEA.L (A1),A1
00001a   2031 0800      MOVE.L  0(A1,D0.L),D0
00001e   225f           MOVEA.L (A7)+,A1
000020   4e75           RTS
000022   4e71           NOP
*
*
000024   0000 0000      ORI.B   #0,D0
000028   2f09           MOVE.L  A1,-(A7)
00002a   204d           MOVEA.L A5,A0
00002c   43fa ffdc      LEA     $000c,A1
000030   2089           MOVE.L  A1,(A0)
000032   5888           ADDQ.L  #4,A0
*
*
000034   225f           MOVEA.L (A7)+,A1
000036   4e75           RTS
*
DATA
*
__dataseg:
*
 XDEF z
*
 XREF b
*
 END
