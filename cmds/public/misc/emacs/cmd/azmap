;	AZMAP.CMD:	MicroEMACS Macro file
;			to translate an AZTEC C .SYM map file into
;			the form needed to read it into ADDSYMS
;			to be used with Pheonix's PFIX plus (C) debugger
;
;			written 6/5/86 by Daniel Lawrence

beginning-of-file
newline
previous-line
replace-string "~n" "~n 0000:"
beginning-of-line
kill-to-end-of-line
newline
newline
newline
beginning-of-file
newline
insert-string "  Address         Publics by Name"
newline
set-mark
end-of-file
case-region-upper
beginning-of-file
replace-string "_~n" "~n"

