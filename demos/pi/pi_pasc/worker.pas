PROGRAM worker(input, out);

FUNCTION _read(hand:integer; place:integer; amount:integer):integer;EXTERNAL;
FUNCTION _write(hand:integer;place:integer;amount:integer):integer;EXTERNAL;

	{ the evaluation routine }
FUNCTION eval(position, workers, intervals : integer):longreal;
VAR
	first, current, last : integer;
	width, sum, tmp : longreal;

BEGIN
	sum	:= 0.0D0;
	width	:= 1.0D0 / (workers * intervals);
	first	:= position * intervals;
	last	:= first + intervals;

	for current := first to (last - 1) do
	BEGIN
		tmp := (0.5D0 + current) * width;
		sum := sum + width * (4.0D0 / (1.0D0 + tmp * tmp));
	END;

	eval := sum;
END;

	{ the main routine }
VAR	position, number_workers, intervals, junk, temp:integer;
	total, sum:longreal;

BEGIN
	junk := _read( 0, addr(position), 4);
	temp := position + 1;
	junk := _write(1, addr(temp), 4);

	junk := _read( 0, addr(number_workers), 4);
	junk := _write(1, addr(number_workers), 4);

	junk := _read( 0, addr(intervals), 4);
	junk := _write(1, addr(intervals), 4);

	sum := eval(position, number_workers, intervals);

	junk  := _read( 0, addr(total), 8);
	total := total + sum;
	junk  := _write(1, addr(total), 8);
END.
