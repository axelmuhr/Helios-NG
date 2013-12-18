MODULE Control;

  FROM SYSTEM IMPORT
    ADR;
  FROM InOut IMPORT
    WriteString, WriteLn, WriteInt, ReadInt, Done;
  FROM RealInOut IMPORT
    WriteReal;
  FROM POSIX IMPORT
    read, write;

  VAR
    NumberWorkers, Intervals, junk: INTEGER;
    Total: REAL;

BEGIN
  NumberWorkers := 0;
  junk := write(5, ADR(NumberWorkers), SIZE(NumberWorkers));
  junk := read (4, ADR(NumberWorkers), SIZE(NumberWorkers));

  WriteString('Pi controller : the number of workers is ');
  WriteInt(NumberWorkers, 0); WriteLn;

  junk := write(5, ADR(NumberWorkers), SIZE(NumberWorkers));
  junk := read (4, ADR(NumberWorkers), SIZE(NumberWorkers));

  REPEAT
    WriteString('Please enter the number of intervals per worker : ');
    ReadInt(Intervals);
  UNTIL Done;
  WriteString('Evaluating a total of ');
  WriteInt(NumberWorkers * Intervals, 0);
  WriteString(' intervals'); WriteLn;

  junk := write(5, ADR(Intervals), SIZE(Intervals));
  junk := read (4, ADR(Intervals), SIZE(Intervals));

  Total := 0.0;
  junk  := write(5, ADR(Total), SIZE(Total));
  junk  := read (4, ADR(Total), SIZE(Total));

  WriteString('Calculated value of pi is ');
  WriteReal(Total, 17); WriteLn;
END Control.
