	TTL	Created by "buildtime"	> timedate.s

old_opt	SETA	{OPT}
	OPT	(opt_off)

	; Time and Date variables
	GBLA	MakeTime	; secs since 1st Jan 1970
	GBLS	MakeDate	; full ANSI time and date
	GBLS	MakeDay		; full weekday name
	GBLS	MakeMDay	; month day
	GBLS	MakeMonth	; full month name
	GBLS	MakeYear	; full year number
	GBLS	MakeClock	; 24hour clock

MakeTime	SETA	&271C3FF5
MakeDate	SETS	"Wed Oct 17 12:26:45 1990"
MakeDay		SETS	"Wednesday"
MakeMDay	SETS	"17"
MakeMonth	SETS	"October"
MakeYear	SETS	"1990"
MakeClock	SETS	"12:26"

	OPT	(old_opt)
	END
