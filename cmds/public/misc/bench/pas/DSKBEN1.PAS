
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