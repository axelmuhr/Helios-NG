METACOMCO ASSEMBLER V10.212                                                                                PAGE 1

  LOC        OBJECT          STMT             SOURCE STATEMENT

                                4
                                5          XREF        _DOSBase
                                6
                                7 PARM0    MACRO
                                8 \1       MOVE.L      _DOSBase,A0
                                9          JSR         _LVO\1(A0)
                               10          RTS
                               11          ENDM
                               12
                               13 PARM1    MACRO
                               14 \1       MOVE.L      D0,D1
                               15          MOVE.L      _DOSBase,A0
                               16          JSR         _LVO\1(A0)
                               17          RTS
                               18          ENDM
                               19
                               20 PARM2    MACRO
                               21 \1       MOVE.L      D1,D2
                               22          MOVE.L      D0,D1
                               23          MOVE.L      _DOSBase,A0
                               24          JSR         _LVO\1(A0)
                               25          RTS
                               26          ENDM
                               27
                               28 PARM3    MACRO
                               29 \1       MOVE.L      D2,D3
                               30          MOVE.L      D1,D2
                               31          MOVE.L      D0,D1
                               32          MOVE.L      _DOSBase,A0
                               33          JSR         _LVO\1(A0)
                               34          RTS
                               35          ENDM
                               36
                               37 PARM4    MACRO
                               38 \1       MOVE.L      D4,-(SP)
                               39          MOVE.L      D3,D4
                               40          MOVE.L      D2,D3
                               41          MOVE.L      D1,D2
                               42          MOVE.L      D0,D1
                               43          MOVE.L      _DOSBase,A0
                               44          JSR         _LVO\1(A0)
                               45          MOVE.L      (SP)+,D4
                               46          RTS
                               47          ENDM
                               48
                               49          PARM1       Exit
0000'  2200                    49+Exit     MOVE.L      D0,D1
0002'  2079 0000 0000          49+         MOVE.L      _DOSBase,A0
0008'  4EA8 0000               49+         JSR         _LVOExit(A0)
000C'  4E75                    49+         RTS
                               50          PARM1       Delay
000E'  2200                    50+Delay    MOVE.L      D0,D1
0010'  2079 0000 0000          50+         MOVE.L      _DOSBase,A0
0016'  4EA8 0008               50+         JSR         _LVODelay(A0)
001A'  4E75                    50+         RTS
METACOMCO ASSEMBLER V10.212                                                                                PAGE 2

  LOC        OBJECT          STMT             SOURCE STATEMENT

                               51          PARM0       Input
001C'  2079 0000 0000          51+Input    MOVE.L      _DOSBase,A0
0022'  4EA8 0010               51+         JSR         _LVOInput(A0)
0026'  4E75                    51+         RTS
                               52          PARM0       Output
0028'  2079 0000 0000          52+Output   MOVE.L      _DOSBase,A0
002E'  4EA8 0018               52+         JSR         _LVOOutput(A0)
0032'  4E75                    52+         RTS
                               53          PARM1       UnLoadSeg
0034'  2200                    53+UnLoadSeg MOVE.L     D0,D1
0036'  2079 0000 0000          53+         MOVE.L      _DOSBase,A0
003C'  4EA8 0020               53+         JSR         _LVOUnLoadSeg(A0)
0040'  4E75                    53+         RTS
                               54          PARM2       WaitForChar
0042'  2401                    54+WaitForChar MOVE.L   D1,D2
0044'  2200                    54+         MOVE.L      D0,D1
0046'  2079 0000 0000          54+         MOVE.L      _DOSBase,A0
004C'  4EA8 0028               54+         JSR         _LVOWaitForChar(A0)
0050'  4E75                    54+         RTS
                               55          PARM1       Close
0052'  2200                    55+Close    MOVE.L      D0,D1
0054'  2079 0000 0000          55+         MOVE.L      _DOSBase,A0
005A'  4EA8 0030               55+         JSR         _LVOClose(A0)
005E'  4E75                    55+         RTS
                               56          PARM1       UnLock
0060'  2200                    56+UnLock   MOVE.L      D0,D1
0062'  2079 0000 0000          56+         MOVE.L      _DOSBase,A0
0068'  4EA8 0038               56+         JSR         _LVOUnLock(A0)
006C'  4E75                    56+         RTS
                               57          PARM1       DupLock
006E'  2200                    57+DupLock  MOVE.L      D0,D1
0070'  2079 0000 0000          57+         MOVE.L      _DOSBase,A0
0076'  4EA8 0040               57+         JSR         _LVODupLock(A0)
007A'  4E75                    57+         RTS
                               58          PARM3       Seek
007C'  2602                    58+Seek     MOVE.L      D2,D3
007E'  2401                    58+         MOVE.L      D1,D2
0080'  2200                    58+         MOVE.L      D0,D1
0082'  2079 0000 0000          58+         MOVE.L      _DOSBase,A0
0088'  4EA8 0048               58+         JSR         _LVOSeek(A0)
008C'  4E75                    58+         RTS
                               59          PARM3       Read
008E'  2602                    59+Read     MOVE.L      D2,D3
0090'  2401                    59+         MOVE.L      D1,D2
0092'  2200                    59+         MOVE.L      D0,D1
0094'  2079 0000 0000          59+         MOVE.L      _DOSBase,A0
009A'  4EA8 0050               59+         JSR         _LVORead(A0)
009E'  4E75                    59+         RTS
                               60          PARM3       Write
00A0'  2602                    60+Write    MOVE.L      D2,D3
00A2'  2401                    60+         MOVE.L      D1,D2
00A4'  2200                    60+         MOVE.L      D0,D1
00A6'  2079 0000 0000          60+         MOVE.L      _DOSBase,A0
00AC'  4EA8 0058               60+         JSR         _LVOWrite(A0)
00B0'  4E75                    60+         RTS
METACOMCO ASSEMBLER V10.212                                                                                PAGE 3

  LOC        OBJECT          STMT             SOURCE STATEMENT

                               61          PARM1       ParentDir
00B2'  2200                    61+ParentDir MOVE.L     D0,D1
00B4'  2079 0000 0000          61+         MOVE.L      _DOSBase,A0
00BA'  4EA8 0060               61+         JSR         _LVOParentDir(A0)
00BE'  4E75                    61+         RTS
                               62          PARM3       Execute
00C0'  2602                    62+Execute  MOVE.L      D2,D3
00C2'  2401                    62+         MOVE.L      D1,D2
00C4'  2200                    62+         MOVE.L      D0,D1
00C6'  2079 0000 0000          62+         MOVE.L      _DOSBase,A0
00CC'  4EA8 0068               62+         JSR         _LVOExecute(A0)
00D0'  4E75                    62+         RTS
                               63          PARM1       IsInteractive
00D2'  2200                    63+IsInteractive MOVE.L D0,D1
00D4'  2079 0000 0000          63+         MOVE.L      _DOSBase,A0
00DA'  4EA8 0070               63+         JSR         _LVOIsInteractive(A0)
00DE'  4E75                    63+         RTS
                               64          PARM1       DateStamp
00E0'  2200                    64+DateStamp MOVE.L     D0,D1
00E2'  2079 0000 0000          64+         MOVE.L      _DOSBase,A0
00E8'  4EA8 0078               64+         JSR         _LVODateStamp(A0)
00EC'  4E75                    64+         RTS
                               65          PARM2       SetProtection
00EE'  2401                    65+SetProtection MOVE.L D1,D2
00F0'  2200                    65+         MOVE.L      D0,D1
00F2'  2079 0000 0000          65+         MOVE.L      _DOSBase,A0
00F8'  4EA8 0080               65+         JSR         _LVOSetProtection(A0)
00FC'  4E75                    65+         RTS
                               66          PARM2       SetComment
00FE'  2401                    66+SetComment MOVE.L    D1,D2
0100'  2200                    66+         MOVE.L      D0,D1
0102'  2079 0000 0000          66+         MOVE.L      _DOSBase,A0
0108'  4EA8 0088               66+         JSR         _LVOSetComment(A0)
010C'  4E75                    66+         RTS
                               67          PARM1       DeviceProc
010E'  2200                    67+DeviceProc MOVE.L    D0,D1
0110'  2079 0000 0000          67+         MOVE.L      _DOSBase,A0
0116'  4EA8 0090               67+         JSR         _LVODeviceProc(A0)
011A'  4E75                    67+         RTS
                               68          PARM1       LoadSeg
011C'  2200                    68+LoadSeg  MOVE.L      D0,D1
011E'  2079 0000 0000          68+         MOVE.L      _DOSBase,A0
0124'  4EA8 0098               68+         JSR         _LVOLoadSeg(A0)
0128'  4E75                    68+         RTS
                               69          PARM4       CreateProc
012A'  2F04                    69+CreateProc MOVE.L    D4,-(SP)
012C'  2803                    69+         MOVE.L      D3,D4
012E'  2602                    69+         MOVE.L      D2,D3
0130'  2401                    69+         MOVE.L      D1,D2
0132'  2200                    69+         MOVE.L      D0,D1
0134'  2079 0000 0000          69+         MOVE.L      _DOSBase,A0
013A'  4EA8 00A0               69+         JSR         _LVOCreateProc(A0)
013E'  281F                    69+         MOVE.L      (SP)+,D4
0140'  4E75                    69+         RTS
                               70          PARM0       IoErr
METACOMCO ASSEMBLER V10.212                                                                                PAGE 4

  LOC        OBJECT          STMT             SOURCE STATEMENT

0142'  2079 0000 0000          70+IoErr    MOVE.L      _DOSBase,A0
0148'  4EA8 00A8               70+         JSR         _LVOIoErr(A0)
014C'  4E75                    70+         RTS
                               71          PARM1       CurrentDir
014E'  2200                    71+CurrentDir MOVE.L    D0,D1
0150'  2079 0000 0000          71+         MOVE.L      _DOSBase,A0
0156'  4EA8 00B0               71+         JSR         _LVOCurrentDir(A0)
015A'  4E75                    71+         RTS
                               72          PARM1       CreateDir
015C'  2200                    72+CreateDir MOVE.L     D0,D1
015E'  2079 0000 0000          72+         MOVE.L      _DOSBase,A0
0164'  4EA8 00B8               72+         JSR         _LVOCreateDir(A0)
0168'  4E75                    72+         RTS
                               73          PARM2       Info
016A'  2401                    73+Info     MOVE.L      D1,D2
016C'  2200                    73+         MOVE.L      D0,D1
016E'  2079 0000 0000          73+         MOVE.L      _DOSBase,A0
0174'  4EA8 00C0               73+         JSR         _LVOInfo(A0)
0178'  4E75                    73+         RTS
                               74          PARM2       ExNext
017A'  2401                    74+ExNext   MOVE.L      D1,D2
017C'  2200                    74+         MOVE.L      D0,D1
017E'  2079 0000 0000          74+         MOVE.L      _DOSBase,A0
0184'  4EA8 00C8               74+         JSR         _LVOExNext(A0)
0188'  4E75                    74+         RTS
                               75          PARM2       Examine
018A'  2401                    75+Examine  MOVE.L      D1,D2
018C'  2200                    75+         MOVE.L      D0,D1
018E'  2079 0000 0000          75+         MOVE.L      _DOSBase,A0
0194'  4EA8 00D0               75+         JSR         _LVOExamine(A0)
0198'  4E75                    75+         RTS
                               76          PARM2       Lock
019A'  2401                    76+Lock     MOVE.L      D1,D2
019C'  2200                    76+         MOVE.L      D0,D1
019E'  2079 0000 0000          76+         MOVE.L      _DOSBase,A0
01A4'  4EA8 00D8               76+         JSR         _LVOLock(A0)
01A8'  4E75                    76+         RTS
                               77          PARM2       Rename
01AA'  2401                    77+Rename   MOVE.L      D1,D2
01AC'  2200                    77+         MOVE.L      D0,D1
01AE'  2079 0000 0000          77+         MOVE.L      _DOSBase,A0
01B4'  4EA8 00E0               77+         JSR         _LVORename(A0)
01B8'  4E75                    77+         RTS
                               78          PARM1       DeleteFile
01BA'  2200                    78+DeleteFile MOVE.L    D0,D1
01BC'  2079 0000 0000          78+         MOVE.L      _DOSBase,A0
01C2'  4EA8 00E8               78+         JSR         _LVODeleteFile(A0)
01C6'  4E75                    78+         RTS
                               79          PARM2       Open
01C8'  2401                    79+Open     MOVE.L      D1,D2
01CA'  2200                    79+         MOVE.L      D0,D1
01CC'  2079 0000 0000          79+         MOVE.L      _DOSBase,A0
01D2'  4EA8 00F0               79+         JSR         _LVOOpen(A0)
01D6'  4E75                    79+         RTS
                               80          PARM2       PktWait
METACOMCO ASSEMBLER V10.212                                                                                PAGE 5

  LOC        OBJECT          STMT             SOURCE STATEMENT

01D8'  2401                    80+PktWait  MOVE.L      D1,D2
01DA'  2200                    80+         MOVE.L      D0,D1
01DC'  2079 0000 0000          80+         MOVE.L      _DOSBase,A0
01E2'  4EA8 00F8               80+         JSR         _LVOPktWait(A0)
01E6'  4E75                    80+         RTS
                               81          PARM3       VDU
01E8'  2602                    81+VDU      MOVE.L      D2,D3
01EA'  2401                    81+         MOVE.L      D1,D2
01EC'  2200                    81+         MOVE.L      D0,D1
01EE'  2079 0000 0000          81+         MOVE.L      _DOSBase,A0
01F4'  4EA8 0100               81+         JSR         _LVOVDU(A0)
01F8'  4E75                    81+         RTS
                               82
                               83          END



No errors found in this Assembly
