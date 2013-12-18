
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
