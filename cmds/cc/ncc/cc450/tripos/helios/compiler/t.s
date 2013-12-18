*
*
000000                 __cd694d274:
000000   0000 0000      ORI.B   #0,D0
*
*
000004     61626364     DC.L    $61626364         ; 'abcd'
000008     65666768     DC.L    $65666768         ; 'efgh'
00000c     696a6b6c     DC.L    $696A6B6C         ; 'ijkl'
000010     6d6e6f70     DC.L    $6D6E6F70         ; 'mnop'
000014     71727374     DC.L    $71727374         ; 'qrst'
000018     75767778     DC.L    $75767778         ; 'uvwx'
00001c     797a0000     DC.L    $797A0000         ; 'yz\0\0'
*
*
000020     6d61696e     DC.L    $6D61696E         ; 'main'
000024     00000000     DC.L    $00000000         ; '\0\0\0\0'
000028     ff000008     DC.L    $FF000008
*
*
00002c                 main:
00002c   48e7 0060      MOVEM.L A1-A2,-(A7)
000030   246d 0000      MOVEA.L 0(A5),A2
000034   45ea 0000      LEA     0(A2),A2
000038   2252           MOVEA.L (A2),A1
00003a   2549 0004      MOVE.L  A1,4(A2)
00003e   4cdf 0600      MOVEM.L (A7)+,A1-A2
000042   4e75           RTS
*
*
000044   0000 0000      ORI.B   #0,D0
000048   2f09           MOVE.L  A1,-(A7)
00004a   220d           MOVE.L  A5,D1
00004c   206d 0000      MOVEA.L 0(A5),A0
000050   43fa ffda      LEA     $002c,A1
000054   2089           MOVE.L  A1,(A0)
000056   5888           ADDQ.L  #4,A0
*
*
000058   43fa ffaa      LEA     $0004,A1
00005c   2089           MOVE.L  A1,(A0)
00005e   5888           ADDQ.L  #4,A0
*
*
000060   7e00           MOVEQ   #0,D7
000062   2087           MOVE.L  D7,(A0)
000064   5888           ADDQ.L  #4,A0
*
*
000066   2a41           MOVEA.L D1,A5
000068   225f           MOVEA.L (A7)+,A1
00006a   4e75           RTS
00006c   4e71           NOP
*
DATA
*
__dd694d274:
abc:
 DC.L   __cd694d274+4
def:
 DC.L   $00000000
*
 XDEF main
*
*
 END
