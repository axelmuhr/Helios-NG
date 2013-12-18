	TTL	Created by "buildtime"	> timedate.s

old_opt	SETA	{OPT}
	OPT	(opt_off)

		GBLL	timedate_s
timedate_s	SETL	{TRUE}

	; Time and Date variables
	GBLA	MakeTime	; secs since 1st Jan 1970
	GBLS	MakeDate	; full ANSI time and date
	GBLS	MakeDay		; full weekday name
	GBLS	MakeMDay	; month day
	GBLS	MakeMonth	; full month name
	GBLS	MakeYear	; full year number
	GBLS	MakeClock	; 24hour clock

MakeTime	SETA	&28FD6FD8
MakeDate	SETS	"Thu Oct 17 12:11:20 1991"
MakeDay		SETS	"Thursday"
MakeMDay	SETS	"17"
MakeMonth	SETS	"October"
MakeYear	SETS	"1991"
MakeClock	SETS	"12:11"

	OPT	(old_opt)
	END
