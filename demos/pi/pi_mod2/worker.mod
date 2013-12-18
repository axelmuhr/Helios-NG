MODULE Worker;

  FROM SYSTEM IMPORT
    ADR;
  FROM POSIX IMPORT
    read, write;

  PROCEDURE Eval(position, workers, intervals: INTEGER): REAL;
    VAR first, current, last: INTEGER;
        width, sum, tmp: REAL;
  BEGIN
    sum := 0.0;
    width := 1.0 / FLOAT(workers * intervals);
    first := position * intervals;
    last := first + intervals;

    FOR current := first TO (last - 1) DO
      tmp := (0.5 + FLOAT(current)) * width;
      sum := sum + width * (4.0 / (1.0 + tmp * tmp));
    END;
    RETURN sum;
  END Eval;

  (* the main routine *)

  VAR
    Position, NumberWorkers, Intervals, junk, temp: INTEGER;
    Total, Sum: REAL;

BEGIN
  junk := read (0, ADR(Position), SIZE(Position));
  temp := Position + 1;
  junk := write(1, ADR(temp), SIZE(temp));

  junk := read (0, ADR(NumberWorkers), SIZE(NumberWorkers));
  junk := write(1, ADR(NumberWorkers), SIZE(NumberWorkers));

  junk := read (0, ADR(Intervals), SIZE(Intervals));
  junk := write(1, ADR(Intervals), SIZE(Intervals));

  Sum := Eval(Position, NumberWorkers, Intervals);

  junk  := read (0, ADR(Total), SIZE(Total));
  Total := Total + Sum;
  junk  := write(1, ADR(Total), SIZE(Total));
END Worker.
