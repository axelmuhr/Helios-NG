PROGRAM control(input, out);

FUNCTION _read(hand:integer; place:integer; amount:integer):integer;EXTERNAL;
FUNCTION _write(hand:integer;place:integer;amount:integer):integer;EXTERNAL;

VAR	number_workers, intervals, junk:integer;
	total:longreal;

BEGIN
	number_workers := 0;
	junk := _write(5, addr(number_workers), 4);
	junk := _read( 4, addr(number_workers), 4);

	writeln('Pi controller : the number of workers is ', number_workers);

	junk := _write(5, addr(number_workers), 4);
	junk := _read( 4, addr(number_workers), 4);

	write('Please enter the number of intervals per worker : ');
	readln(intervals);
	write('Evaluating a total of ');
	writeln(number_workers * intervals,' intervals');

	junk := _write(5, addr(intervals), 4);
	junk :=_read( 4, addr(intervals), 4);

	total := 0.0D0;
	junk  := _write(5, addr(total), 8);
	junk  := _read( 4, addr(total), 8);

	writeln('Calculated value of pi is ', total:16:14);
END.
