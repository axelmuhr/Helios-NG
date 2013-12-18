
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
