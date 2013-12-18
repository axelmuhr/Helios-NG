Listing 1.  Source code for primary benchmark program, BEN1.PAS.

PROGRAM Ben1;
USES CRT, DOS;
CONST MAX = 100000;
VAR i, j, k : LONGINT;
    Hr, Min, Sec, Sec100 : WORD;
    Time1 : LONGINT;
    ch : CHAR;
BEGIN
    WHILE 2 > 1 DO BEGIN
        GetTime(Hr, Min, Sec, Sec100);
        Time1 := LONGINT(Sec + ((Hr * 60) + Min * 60));
        FOR i := 1 TO MAX DO BEGIN
            j := i + 1;
            k := 2 * j;
            DEC(j);
            k := j div 2;
        END;
        WRITE(^G);
        GetTime(Hr, Min, Sec, Sec100);
        WRITE('TIme = ',LONGINT(Sec + ((Hr * 60) + Min * 60)) - Time1);
        WRITELN(' sec');
        IF Keypressed THEN HALT;
    END;
END.
________________________________________________________________________

Listing 2.  Source code for secondary benchmark program, BEN2.PAS.

PROGRAM Ben2;
USES CRT;
CONST MAX = 100000;

VAR i, j, k, n : LONGINT;
    ch : CHAR;
BEGIN
    n := 0;
    WHILE 2 > 1 DO BEGIN
        FOR i := 1 TO MAX DO BEGIN
            j := i + 1;
            k := 2 * j;
            DEC(j);
            k := j div 2;
            writeln(i);
            IF Keypressed THEN HALT;
        END;
        INC(n);
        writeln('Finished loop # ',n);
        IF n > MAXINT THEN n := 0;
    END;
END.
________________________________________________________________________

Listing 3.  Source code for disk benchmark program, DSKBEN1.PAS.

PROGRAM Disk_Benchmark_1;
{ This benchmark is used to test concurrent systems }
{ version 1.0                              3/22/88  }
{ by Namir Clement Shammas                          }
Uses CRT, DOS;
VAR Ch : CHAR;
    Hr, Min, Sec, Sec100 : WORD;
    Time1 : LONGINT;
    Line, Filename : STRING[80];
    FileVar : TEXT;
BEGIN
    ClrScr;
    FIlename := ParamStr(1);
    REPEAT
        Assign(FileVar, FILENAME);
        Reset(FileVar);
        GetTime(Hr, Min, Sec, Sec100);
        Time1 := LONGINT(Sec + ((Hr * 60) + Min * 60));
        WHILE NOT EOF(FileVar) DO
            READLN(FileVar, Line);
        Close(FileVar);
        GetTime(Hr, Min, Sec, Sec100);
        WRITE('Time = ',LONGINT(Sec + ((Hr * 60) + Min * 60)) - Time1);
        WRITELN(' sec');
        IF Keypressed THEN
            IF UpCase(ReadKey) = 'Q' THEN HALT;
    UNTIL 1 < 0;
END.
