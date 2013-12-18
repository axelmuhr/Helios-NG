;Create a block of DATA statements for a BASIC program
; within MicroEMACS 3.8

	insert-string "1000 DATA "
	set %linenum 1000

*nxtin
	update-screen		;make sure we see the changes
	set %data @"Next number[0 to end]: "
	!if &equal %data 0
		!goto finish
	!endif

	!if &greater $curcol 60
		2 delete-previous-character
		newline
		set %linenum &add %linenum 10
		insert-string &cat %linenum " DATA "
	!endif

	insert-string &cat %data ", "
	!goto nxtin

*finish

	2 delete-previous-character
	newline


