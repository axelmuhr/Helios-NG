	set %try 0
	set %sum 0
	insert-string "Rolls: "
*nextry
	set %roll &rnd 100
	!if &greater $curcol 70
		newline
		insert-string "       "
	!endif
	set %sum &add %sum %roll
	insert-string &cat %roll " "
	update-screen
	set %try &add %try 1
	!if &less %try 300
		!goto nextry
	!endif

	newline
	newline
	insert-string &cat &cat "Sum of rolls: " %sum "~n"
	insert-string &cat &cat "Average roll: " &div %sum 300 "~n"
	update-screen
	unmark-buffer
