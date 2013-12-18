  2ffc00 0xc4045601  ... V ......  
  2ffc01 0x00000000  ............  ABSF  R0, R0
  2ffc02 0x000003b8  ............  ABSF  R<illegal>
  2ffc03 0x15610302  ...... a ...  STIK  1,  *+AR3(2)
  2ffc04 0x085d0301  ...... ] ...  LDI   *+AR3(1), R9
  2ffc05 0x08750000  ...... u ...  LDI   0, ST
  2ffc06 0x1a7d0020     ... } ...  TSTB  0x20, R9
  2ffc07 0x6a060001  ......... j   Bne   +1             (branch to 2ffc09)
  2ffc08 0x10750800  ...... u ...  OR    0x800, ST
  2ffc09 0x1a7d0008  ...... } ...  TSTB  0x8, R9
  2ffc0a 0x6a060004  ......... j   Bne   +4             (branch to 2ffc0f)
  2ffc0b 0x08606000  ... `  ` ...  LDI   0x6000, R0
  2ffc0c 0x08180000  ............  LDI   R0, IIF
  2ffc0d 0x1fe90010  ............  LDHI  0x10, AR1
  2ffc0e 0x10690020     ... i ...  OR    0x20, AR1
  2ffc0f 0x1a7d0018  ...... } ...  TSTB  0x18, R9
  2ffc10 0x6a050008  ......... j   Beq   +8             (branch to 2ffc19)
  2ffc11 0x08410301  ...... A ...  LDI   *+AR3(1), R1
  2ffc12 0x37000102  ......... 7   SUBI3 2, R1, R0
  2ffc13 0x1fec002f   / .........  LDHI  0x2f, AR4
  2ffc14 0x106cf800  ...... l ...  OR    0xf800, AR4
  2ffc15 0x139b0000  ............  RPTS  R0
  2ffc16 0xda412403  ... $  A ...  LDI   *+AR3(1), R1
                                || STI   R1,  *AR4++(1)
  2ffc17 0x15412401  ... $  A ...  STI   R1,  *AR4++(1)
  2ffc18 0x6a000049   I ...... j   Bu    +73            (branch to 2ffc62)
  2ffc19 0x1fea7000  ... p ......  LDHI  0x7000, AR2
  2ffc1a 0x0840c100  ...... @ ...  LDI   *AR1, R0
  2ffc1b 0x1562c100  ...... b ...  STIK  2,  *AR1
  2ffc1c 0x086107d0  ...... a ...  LDI   0x7d0, R1
  2ffc1d 0x18610001  ...... a ...  SUBI  1, R1
  2ffc1e 0x6a06fffe  ......... j   Bne   -2             (branch to 2ffc1d)
  2ffc1f 0x1566c100  ...... f ...  STIK  6,  *AR1
  2ffc20 0x086107d0  ...... a ...  LDI   0x7d0, R1
  2ffc21 0x18610001  ...... a ...  SUBI  1, R1
  2ffc22 0x6a06fffe  ......... j   Bne   -2             (branch to 2ffc21)
  2ffc23 0x1562c100  ...... b ...  STIK  2,  *AR1
  2ffc24 0x086107d0  ...... a ...  LDI   0x7d0, R1
  2ffc25 0x18610001  ...... a ...  SUBI  1, R1
  2ffc26 0x6a06fffe  ......... j   Bne   -2             (branch to 2ffc25)
  2ffc27 0x1566c100  ...... f ...  STIK  6,  *AR1
  2ffc28 0x02e00008  ............  AND   0x8, R0
  2ffc29 0x6a050016  ......... j   Beq   +22            (branch to 2ffc40)
  2ffc2a 0x087b001f  ...... { ...  LDI   31, RC
  2ffc2b 0x64000002  ......... d   RPTB  +2             (branch to 2ffc2e)
  2ffc2c 0x08422201  ... "  B ...  LDI   *AR2++(1), R2
  2ffc2d 0x09e2ffff  .........\t   LSH   -1, R2
  2ffc2e 0x1363ffff  ...... c ...  RORC  R3
  2ffc2f 0x08000003  ............  LDI   R3, R0
  2ffc30 0x6a050031   1 ...... j   Beq   +49            (branch to 2ffc62)
  2ffc31 0x087b001f  ...... { ...  LDI   31, RC
  2ffc32 0x64000002  ......... d   RPTB  +2             (branch to 2ffc35)
  2ffc33 0x08422201  ... "  B ...  LDI   *AR2++(1), R2
  2ffc34 0x09e2ffff  .........\t   LSH   -1, R2
  2ffc35 0x1363ffff  ...... c ...  RORC  R3
  2ffc36 0x080c0003  ......\f ...  LDI   R3, AR4
  2ffc37 0x087b001f  ...... { ...  LDI   31, RC
  2ffc38 0x64000002  ......... d   RPTB  +2             (branch to 2ffc3b)
  2ffc39 0x08422201  ... "  B ...  LDI   *AR2++(1), R2
  2ffc3a 0x09e2ffff  .........\t   LSH   -1, R2
  2ffc3b 0x1363ffff  ...... c ...  RORC  R3
  2ffc3c 0x15432401  ... $  C ...  STI   R3,  *AR4++(1)
  2ffc3d 0x18600001  ...... ` ...  SUBI  1, R0
  2ffc3e 0x6a06fff8  ......... j   Bne   -8             (branch to 2ffc37)
  2ffc3f 0x6a00ffea  ......... j   Bu    -22            (branch to 2ffc2a)
  2ffc40 0x08610000  ...... a ...  LDI   0, R1
  2ffc41 0x08630000  ...... c ...  LDI   0, R3
  2ffc42 0x087b0007  ...... { ...  LDI   7, RC
  2ffc43 0x64000004  ......... d   RPTB  +4             (branch to 2ffc48)
  2ffc44 0x08422201  ... "  B ...  LDI   *AR2++(1), R2
  2ffc45 0x02e2000f  ............  AND   0xf, R2
  2ffc46 0x09820001  .........\t   LSH   R1, R2
  2ffc47 0x10030002  ............  OR    R2, R3
  2ffc48 0x02610004  ...... a ...  ADDI  4, R1
  2ffc49 0x08000003  ............  LDI   R3, R0
  2ffc4a 0x6a050017  ......... j   Beq   +23            (branch to 2ffc62)
  2ffc4b 0x08610000  ...... a ...  LDI   0, R1
  2ffc4c 0x08630000  ...... c ...  LDI   0, R3
  2ffc4d 0x087b0007  ...... { ...  LDI   7, RC
  2ffc4e 0x64000004  ......... d   RPTB  +4             (branch to 2ffc53)
  2ffc4f 0x08422201  ... "  B ...  LDI   *AR2++(1), R2
  2ffc50 0x02e2000f  ............  AND   0xf, R2
  2ffc51 0x09820001  .........\t   LSH   R1, R2
  2ffc52 0x10030002  ............  OR    R2, R3
  2ffc53 0x02610004  ...... a ...  ADDI  4, R1
  2ffc54 0x080c0003  ......\f ...  LDI   R3, AR4
  2ffc55 0x08610000  ...... a ...  LDI   0, R1
  2ffc56 0x08630000  ...... c ...  LDI   0, R3
  2ffc57 0x087b0007  ...... { ...  LDI   7, RC
  2ffc58 0x64000004  ......... d   RPTB  +4             (branch to 2ffc5d)
  2ffc59 0x08422201  ... "  B ...  LDI   *AR2++(1), R2
  2ffc5a 0x02e2000f  ............  AND   0xf, R2
  2ffc5b 0x09820001  .........\t   LSH   R1, R2
  2ffc5c 0x10030002  ............  OR    R2, R3
  2ffc5d 0x02610004  ...... a ...  ADDI  4, R1
  2ffc5e 0x15432401  ... $  C ...  STI   R3,  *AR4++(1)
  2ffc5f 0x18600001  ...... ` ...  SUBI  1, R0
  2ffc60 0x6a06fff4  ......... j   Bne   -12            (branch to 2ffc55)
  2ffc61 0x6a00ffde  ......... j   Bu    -34            (branch to 2ffc40)
  2ffc62 0x1fe80010  ............  LDHI  0x10, AR0
  2ffc63 0x1fed002f   / .........  LDHI  0x2f, AR5
  2ffc64 0x106df800  ...... m ...  OR    0xf800, AR5
  2ffc65 0x08400510  ...... @ ...  LDI   *+AR5(16), R0
  2ffc66 0x15400000  ...... @ ...  STI   R0,  *+AR0(0)
  2ffc67 0x08400511  ...... @ ...  LDI   *+AR5(17), R0
  2ffc68 0x15400004  ...... @ ...  STI   R0,  *+AR0(4)
  2ffc69 0x0840050d  ...... @ ...  LDI   *+AR5(13), R0
  2ffc6a 0x15400028   ( ... @ ...  STI   R0,  *+AR0(40)
  2ffc6b 0x0840050e  ...... @ ...  LDI   *+AR5(14), R0
  2ffc6c 0x15400038   8 ... @ ...  STI   R0,  *+AR0(56)
  2ffc6d 0x0840050f  ...... @ ...  LDI   *+AR5(15), R0
  2ffc6e 0x09e0fff0  .........\t   LSH   -16, R0
  2ffc6f 0x1fe1c000  ............  LDHI  0xc000, R1
  2ffc70 0x10000001  ............  OR    R1, R0
  2ffc71 0x15400030   0 ... @ ...  STI   R0,  *+AR0(48)
  2ffc72 0x1a7d0008  ...... } ...  TSTB  0x8, R9
  2ffc73 0x6a060001  ......... j   Bne   +1             (branch to 2ffc75)
  2ffc74 0x1562c100  ...... b ...  STIK  2,  *AR1
  2ffc75 0x08401503  ...... @ ...  LDI   *++AR5(3), R0
  2ffc76 0x08670004  ...... g ...  LDI   4, R7
  2ffc77 0x0849c500  ...... I ...  LDI   *AR5, AR1
  2ffc78 0x085e0501  ...... ^ ...  LDI   *+AR5(1), R10
  2ffc79 0x04e70001  ............  CMPI  1, R7
  2ffc7a 0x6a060001  ......... j   Bne   +1             (branch to 2ffc7c)
  2ffc7b 0x085e0d03  ...... ^ ...  LDI   *-AR5(3), R10
  2ffc7c 0x08400504  ...... @ ...  LDI   *+AR5(4), R0
  2ffc7d 0x6a060017  ......... j   Bne   +23            (branch to 2ffc95)
  2ffc7e 0x0860ffff  ...... ` ...  LDI   -1, R0
  2ffc7f 0x04890000  ............  CMPI  R0, AR1
  2ffc80 0x6a050014  ......... j   Beq   +20            (branch to 2ffc95)
  2ffc81 0x08611000  ...... a ...  LDI   0x1000, R1
  2ffc82 0x08620c40   @ \f  b ...  LDI   0xc40, R2
  2ffc83 0x1542c100  ...... B ...  STI   R2,  *AR1
  2ffc84 0x15410101  ...... A ...  STI   R1,  *+AR1(1)
  2ffc85 0x1e8a0009  \t .........  LDA   AR1, AR2
  2ffc86 0x23a0c102  ......... #   CMPI3 R2,  *AR1, R0
  2ffc87 0x6a06000b  \v ...... j   Bne   +11            (branch to 2ffc93)
  2ffc88 0x020a0001  ......\n ...  ADDI  R1, AR2
  2ffc89 0x0803000a  \n .........  LDI   AR2, R3
  2ffc8a 0x02030002  ............  ADDI  R2, R3
  2ffc8b 0x1543c200  ...... C ...  STI   R3,  *AR2
  2ffc8c 0x23a0c102  ......... #   CMPI3 R2,  *AR1, R0
  2ffc8d 0x6a060005  ......... j   Bne   +5             (branch to 2ffc93)
  2ffc8e 0x049e0009  \t .........  CMPI  AR1, R10
  2ffc8f 0x6a050003  ......... j   Beq   +3             (branch to 2ffc93)
  2ffc90 0x0846c200  ...... F ...  LDI   *AR2, R6
  2ffc91 0x04860003  ............  CMPI  R3, R6
  2ffc92 0x6a05fff5  ......... j   Beq   -11            (branch to 2ffc88)
  2ffc93 0x27000a09  \t \n ... '   SUBI3 AR1, AR2, R0
  2ffc94 0x15400504  ...... @ ...  STI   R0,  *+AR5(4)
  2ffc95 0x08401501  ...... @ ...  LDI   *++AR5(1), R0
  2ffc96 0x18670001  ...... g ...  SUBI  1, R7
  2ffc97 0x6a06ffdf  ......... j   Bne   -33            (branch to 2ffc77)
  2ffc98 0x1fed002f   / .........  LDHI  0x2f, AR5
  2ffc99 0x106df800  ...... m ...  OR    0xf800, AR5
  2ffc9a 0x1a7d0001  ...... } ...  TSTB  0x1, R9
  2ffc9b 0x6a060004  ......... j   Bne   +4             (branch to 2ffca0)
  2ffc9c 0x08490505  ...... I ...  LDI   *+AR5(5), AR1
  2ffc9d 0x312e094d   M \t  .  1   ADDI3 *+AR5(09), AR1, AR6
  2ffc9e 0x04e9ffff  ............  CMPI  -1, AR1
  2ffc9f 0x6a060003  ......... j   Bne   +3             (branch to 2ffca3)
  2ffca0 0x08490506  ...... I ...  LDI   *+AR5(6), AR1
  2ffca1 0x312e0955   U \t  .  1   ADDI3 *+AR5(10), AR1, AR6
  2ffca2 0x6a000007  ......... j   Bu    +7             (branch to 2ffcaa)
  2ffca3 0x086cffff  ...... l ...  LDI   -1, AR4
  2ffca4 0x04cc0506  ............  CMPI  *+AR5(6), AR4
  2ffca5 0x6a060004  ......... j   Bne   +4             (branch to 2ffcaa)
  2ffca6 0x1fec4000  ... @ ......  LDHI  0x4000, AR4
  2ffca7 0x0489000c  \f .........  CMPI  AR4, AR1
  2ffca8 0x50f10000  ......... P   LDIlo 0, IR0
  2ffca9 0x6a070007  ......... j   Blt   +7             (branch to 2ffcb1)
  2ffcaa 0x1fec402f   /  @ ......  LDHI  0x402f, AR4
  2ffcab 0x106cf000  ...... l ...  OR    0xf000, AR4
  2ffcac 0x048e000c  \f .........  CMPI  AR4, AR6
  2ffcad 0x51910009  \t ...... Q   LDIhi AR1, IR0
  2ffcae 0x6a030002  ......... j   Bhi   +2             (branch to 2ffcb1)
  2ffcaf 0x1ff1002f   / .........  LDHI  0x2f, IR0
  2ffcb0 0x1071f000  ...... q ...  OR    0xf000, IR0
  2ffcb1 0x76810009  \t ...... v   LDPE  AR1, TVTP
  2ffcb2 0x1551011c  ...... Q ...  STI   IR0,  *+AR1(28)
  2ffcb3 0x155d011d  ...... ] ...  STI   R9,  *+AR1(29)
  2ffcb4 0x08140009  \t .........  LDI   AR1, SP
  2ffcb5 0x02740076   v ... t ...  ADDI  118, SP
  2ffcb6 0x080a0014  ......\n ...  LDI   SP, AR2
  2ffcb7 0x026a01ff  ...... j ...  ADDI  0x1ff, AR2
  2ffcb8 0x036a01ff  ...... j ...  ANDN  0x1ff, AR2
  2ffcb9 0x7680000a  \n ...... v   LDPE  AR2, IVTP
  2ffcba 0x026a0040   @ ... j ...  ADDI  64, AR2
  2ffcbb 0x1a7d0002  ...... } ...  TSTB  0x2, R9
  2ffcbc 0x6a050002  ......... j   Beq   +2             (branch to 2ffcbf)
  2ffcbd 0x084c0503  ...... L ...  LDI   *+AR5(3), AR4
  2ffcbe 0x6a000003  ......... j   Bu    +3             (branch to 2ffcc2)
  2ffcbf 0x1a7d0004  ...... } ...  TSTB  0x4, R9
  2ffcc0 0x6a050007  ......... j   Beq   +7             (branch to 2ffcc8)
  2ffcc1 0x084c0504  ...... L ...  LDI   *+AR5(4), AR4
  2ffcc2 0x0800000a  \n .........  LDI   AR2, R0
  2ffcc3 0x18000011  ............  SUBI  IR0, R0
  2ffcc4 0x09e00002  .........\t   LSH   2, R0
  2ffcc5 0x15400103  ...... @ ...  STI   R0,  *+AR1(3)
  2ffcc6 0x080a000c  \f ...\n ...  LDI   AR4, AR2
  2ffcc7 0x6a000001  ......... j   Bu    +1             (branch to 2ffcc9)
  2ffcc8 0x15600103  ...... ` ...  STIK  0,  *+AR1(3)
  2ffcc9 0x154a0104  ...... J ...  STI   AR2,  *+AR1(4)
  2ffcca 0x08400301  ...... @ ...  LDI   *+AR3(1), R0
  2ffccb 0x080c000a  \n ...\f ...  LDI   AR2, AR4
  2ffccc 0x15402401  ... $  @ ...  STI   R0,  *AR4++(1)
  2ffccd 0x09e0fffe  .........\t   LSH   -2, R0
  2ffcce 0x18600003  ...... ` ...  SUBI  3, R0
  2ffccf 0x08410301  ...... A ...  LDI   *+AR3(1), R1
  2ffcd0 0x139b0000  ............  RPTS  R0
  2ffcd1 0xda412403  ... $  A ...  LDI   *+AR3(1), R1
                                || STI   R1,  *AR4++(1)
  2ffcd2 0x15412401  ... $  A ...  STI   R1,  *AR4++(1)
  2ffcd3 0x310c0946   F \t \f  1   ADDI3 70, AR1, AR4
  2ffcd4 0x0840c500  ...... @ ...  LDI   *AR5, R0
  2ffcd5 0x08412501  ... %  A ...  LDI   *AR5++(1), R1
  2ffcd6 0x15412401  ... $  A ...  STI   R1,  *AR4++(1)
  2ffcd7 0x18600001  ...... ` ...  SUBI  1, R0
  2ffcd8 0x6a06fffc  ......... j   Bne   -4             (branch to 2ffcd5)
  2ffcd9 0x08400103  ...... @ ...  LDI   *+AR1(3), R0
  2ffcda 0x04e00000  ............  CMPI  0, R0
  2ffcdb 0x6a060005  ......... j   Bne   +5             (branch to 2ffce1)
  2ffcdc 0x0800000a  \n .........  LDI   AR2, R0
  2ffcdd 0x18000011  ............  SUBI  IR0, R0
  2ffcde 0x09e00002  .........\t   LSH   2, R0
  2ffcdf 0x0240c200  ...... @ ...  ADDI  *AR2, R0
  2ffce0 0x15400103  ...... @ ...  STI   R0,  *+AR1(3)
  2ffce1 0x086e0045   E ... n ...  LDI   69, AR6
  2ffce2 0x020e0009  \t .........  ADDI  AR1, AR6
  2ffce3 0x10758000  ...... u ...  OR    0x8000, ST
  2ffce4 0x0800000b  \v .........  LDI   AR3, R0
  2ffce5 0x0801000a  \n .........  LDI   AR2, R1
  2ffce6 0x026a0001  ...... j ...  ADDI  1, AR2
  2ffce7 0x0849c200  ...... I ...  LDI   *AR2, AR1
  2ffce8 0x09e9fffe  .........\t   LSH   -2, AR1
  2ffce9 0x0209000a  \n ...\t ...  ADDI  AR2, AR1
  2ffcea 0x0269000f  ...... i ...  ADDI  15, AR1
  2ffceb 0x1a7d0008  ...... } ...  TSTB  0x8, R9
  2ffcec 0x6a060002  ......... j   Bne   +2             (branch to 2ffcef)
  2ffced 0x086a2000  ...    j ...  LDI   0x2000, AR2
  2ffcee 0x0818000a  \n .........  LDI   AR2, IIF
  2ffcef 0x68000009  \t ...... h   Bu    AR1
  2ffcf0 0x00000000  ............  ABSF  R0, R0
