.\" $Header: /dsl/HeliosRoot/Helios/cmds/public/stevie/RCS/stevie.mm,v 1.1 1993/08/06 15:17:14 nickc Exp $
.\"
.\" Documentation for STEVIE. Process with nroff using the mm macros.
.\"
.nr Hu 1
.SA 1
.TL
STEVIE - An Aspiring VI Clone
.sp
User Reference - 3.69a
.AU "Tony Andrews"
.AF ""
.MT 4
.PH "'STEVIE''User Reference'"
.PF "''- \\\\nP -''"
.H 1 "Overview"
STEVIE is an editor designed to mimic the interface of the UNIX
editor 'vi'. The name (ST Editor for VI Enthusiasts) comes from the fact that
the editor was first written for the Atari ST. The current version also supports
UNIX, Minix (ST), MS-DOS, and OS/2, but I've left
the name intact for now.
.P
This program is the result of many late nights of hacking over the last
couple of years.
The first version was written by Tim Thompson and posted
to USENET. From there, I reworked the data structures completely, added
LOTS of features, and generally improved the overall performance in the
process.
.P
I've labelled STEVIE an 'aspiring' vi clone as a warning to those who
may expect too much. On the whole, the editor is pretty complete.
Nearly all of the visual mode commands are supported.
And several of the more important 'ex' commands are supported as well.
I've tried hard to
capture the feel of vi by getting the little things right.
Making lines
wrap correctly, supporting true operators, and even getting the cursor to
land on the right place for tabs are all a pain, but really help make
the editor feel right.
I've tried to resist the temptation to deviate from the behavior
of vi, even where I disagree with the original design.
.P
The biggest problem remaining has to do with the fact that the edit buffer
is maintained entirely in memory, limiting the size of files that can
be edited in some environments.
Other missing features include named buffers and macros.
Performance is generally reasonable, although the screen update code
could be more efficient.
This is generally only visible on fairly slow systems.
.P
STEVIE may be freely distributed. The source isn't copyrighted or
restricted in any way. If you pass the program along, please include all
the documentation and, if practical, the source as well. I'm not fanatical
about this, but I tried to make STEVIE fairly portable and I'd like to
see as many people have access to the source as possible.
.P
The remainder of this document describes the operation of the editor.
This is intended as a reference for users already familiar with the real
vi editor.
.H 1 "Starting the Editor"
The following command line forms are supported:
.VL 20
.LI "stevie [file ...]"
Edit the specified file(s)
.LI "stevie -t tag"
Start at the location of the given tag
.LI "stevie + file"
Edit file starting at end
.LI "stevie +n file"
Edit file starting a line number 'n'
.LI "stevie +/pat file"
Edit file starting at pattern 'pat'
.LE
.P
If multiple files are given on the command line (using the first form),
the ":n" command goes to the next file, ":N" goes backward in the list,
and ":rew" can be used to rewind back to the start of the file list.
.H 1 "Set Command Options"
The ":set" command works as usual to set parameters. Each parameter has
a long and an abbreviated name, either of which may be used. Boolean
parameters are set as in:
.sp
.ti +5
set showmatch
.sp
or cleared by:
.sp
.ti +5
set noshowmatch
.sp
Numeric parameters are set as in:
.sp
.ti +5
set scroll=5
.sp
Several parameters may be set with a single command:
.sp
.ti +5
set novb sm report=1
.P
To see the status of all parameters use ":set all". Typing ":set" with
no arguments will show only those parameters that have been changed.
The supported parameters, their names, abbreviations, defaults,
and descriptions are shown below:
.VL 12
.LI autoindent
Short: ai, Default: noai, Type: Boolean
.br
When in insert mode, start new lines at the same column as the prior
line. Unlike vi, you can backspace over the indentation.
.LI backup
Short: bk, Default: nobk, Type: Boolean
.br
Leave a backup on file writes.
(During file writes, a backup is always kept for
safety anyway, until the write is completed.
At that point, the 'backup' parameter
determines whether the backup file is deleted.)
.LI color
Short: co, Default: system-dependent, Type: Numeric
.br
Change the screen color (video attributes).
There is a system-dependent mapping between numbers and colors.
(For instance, in MSDOS we use the BIOS-type attributes for
CGA or monochrome screens in alphanumeric mode.)
.LI errorbells
Short: eb, Default: noeb, Type: Boolean
.br
Ring bell when error messages are shown.
.LI ignorecase
Short: ic, Default: noic, Type: Boolean
.br
Ignore case in string searches.
.LI lines
Short: lines, Default: lines=25, Type: Numeric
.br
Number of physical lines on the screen. The default value
and possible settings depend on the host machine.
The default value is 25 on most systems.
.LI list
Short: list, Default: nolist, Type: Boolean
.br
Show tabs and newlines graphically.
.LI modelines
Short: ml, Default: noml, Type: Boolean
.br
Enable processing of modelines in files.
.LI number
Short: nu, Default: nonu, Type: Boolean
.br
Display lines on the screen with their line numbers.
.LI report
Short: report, Default: report=5, Type: Numeric
.br
Minimum number of lines to report operations on.
.LI return
Short: cr, Default: cr, Type: Boolean
.br
End lines with cr-lf when writing files.
.LI scroll
Short: scroll, Default: scroll=12, Type: Numeric
.br
Number of lines to scroll for ^D & ^U.
.LI showmatch
Short: sm, Default: nosm, Type: Boolean
.br
When a ), }, or ] is typed, show the matching (, {, or [ if
it's on the current screen by moving the cursor there briefly.
.LI showmode
Short: mo, Default: nomo, Type: Boolean
.br
Show on status line when in insert mode.
.LI tabstop
Short: ts, Default: ts=8, Type: Numeric
.br
Number of spaces in a tab.
.LI tagstack
Short: tg, Default: notg, Type: Boolean
.br
If set, the tag command :ta is "stacked";
that is, the place from which it was called is pushed onto a stack.
A new command ":untag" goes back to that place, and pops the tag stack.
.LI terse
Short: terse, Default: noterse, Type: Boolean
.br
This option is currently ignored.
It is provided only for compatibility with vi.
.LI tildeop
Short: to, Default: noto, Type: Boolean
.br
If set, tilde is an operator. Otherwise, tilde acts as normal.
.LI wrapscan
Short: ws, Default: ws, Type: Boolean
.br
String searches wrap around the ends of the file.
.LI vbell
Short: vb, Default: vb, Type: Boolean
.br
Use a visual bell, if possible. (novb for audible bell)
.LE
.P
The EXINIT environment variable can be used to modify the default values
on startup as in:
.sp
.ti +5
setenv EXINIT="set sm ts=4"
.P
In environments (e.g. OS/2 or TOS) where lines are normally terminated by
CR-LF, the 'return' parameter allows files to be written with only a LF
terminator (if the parameter is cleared).
This parameter is ignored on UNIX systems.
.P
The 'lines' parameter tells the editor how many lines there are on the screen.
This is useful on systems like the ST (or OS/2 machines with an EGA adapter)
where various screen resolutions may be
used. By using the 'lines' parameter, different screen sizes can be easily
handled.
.H 1 "Colon Commands"
Several of the normal 'vi' colon commands are supported by STEVIE.
Some commands may be preceded by a
line range specification.
For commands that accept a range of lines,
the following address forms are supported:
.DS 1
addr
addr + number
addr - number
.DE
where 'addr' may be one of the following:
.DS 1
a line number
a mark (as in 'a or 'b)
\'.' (the current line)
\'$' (the last line)
.DE
.P
An address range of "%" is accepted as an abbreviation of "1,$".
.H 2 "Mode Lines"
Mode lines are a little-known, but often useful, feature of vi.
To use this feature, special strings are placed in the first or
last five lines in a file.
When the file is edited, these strings are detected and processed
as though typed as a colon command.
One instance where this can be useful is to set the "tabstop"
parameter on a per-file basis.
The following are examples of mode lines:
.DS 1
vi:set ts=4 noai:
ex:45:
.DE
.P
Mode lines are characterized by the string "vi" or "ex" followed
by a command surrounded by colons. Other text may appear on the
line, and multiple mode lines may be present. No guarantee is
made regarding the order in which multiple mode lines will be
processed.
.P
The processing of mode lines is enabled by setting the "ml"
parameter. This should be done in the "EXINIT" environment
variable, so that mode line processing is enabled as soon
as the editor begins.
By default, mode lines are disabled for security reasons.
.H 2 "The Global Command"
A limited form of the global command is supported, accepting the
following command form:
.DS 1
g/pattern/X
.DE
where X may be either 'd' or 'p' to delete or print lines that match
the given pattern.
If a line range is given, only those lines are checked for a match
with the pattern.
If no range is given, all lines are checked.
.P
If the trailing command character is omitted, 'p' is assumed.
In this case, the trailing slash is also optional.
The current version of the editor does not support the undo operation
following the deletion of lines with the global command.
.H 2 "The Substitute Command"
The substitute command provides a powerful mechanism for making more
complex substitutions than can be done directly from visual mode.
The general form of the command is:
.DS 1
s/pattern/replacement/g
.DE
Each line in the given range (or the current line, if no range was
given) is scanned for the given regular expression.
When found, the string that matched the pattern is replaced with
the given replacement string.
If the replacement string is null, each matching pattern string is
deleted.
.P
The trailing 'g' is optional and, if present, indicates that multiple
occurrences of 'pattern' on a line should all be replaced.
.P
Some special sequences are recognized in the replacement string. The
ampersand character is replaced by the entire pattern that was matched.
For example, the following command could be used to put all occurrences
of 'foo' or 'bar' within double quotes:
.DS 1
1,$s/foo|bar/"&"/g
.DE
.P
The special sequence "\\n" where 'n' is a digit from 1 to 9, is replaced
by the string the matched the corresponding parenthesized expression in
the pattern. The following command could be used to swap the first two
parameters in calls to the C function "foo":
.DS 1
1,$s/foo\\\\(([^,]*),([^,]*),/foo(\\\\2,\\\\1,/g
.DE
.P
Like the global command, substitutions can't be undone with this
version of the editor.
.H 2 "File Manipulation Commands"
The following table shows the supported file manipulation commands as
well as some other 'ex' commands that aren't described elsewhere:
.DS CB
:w		write the current file
:wq		write and quit
:x		write (if necessary) and quit
ZZ		same as ":x"

:e file		edit the named file
:e!		re-edit the current file, discarding changes
:e #		edit the alternate file

:w file		write the buffer to the named file
:x,yw file	write lines x through y to the named file
:r file		read the named file into the buffer

:n		edit the next file
:N		edit the previous file
:rew		rewind the file list

:f		show the current file name
:f name		change the current file name
:x=		show the line number of address 'x'

:ta tag		go to the named tag
^]		like ":ta" using the current word as the tag
:untag		go back to place from which ":ta" was called

:help		display a command summary
:ve		show the version number

:sh		run an interactive shell
:!cmd		run a command
.DE
.P
The commands above work pretty much like they do in 'vi'. Most of the
commands support a '!' suffix (if appropriate) to discard any pending
changes.
.H 1 Help Facility
The ":help" command can also be invoked with the <HELP> key on the Atari
ST and the F1 key on MSDOS and OS/2 PCs.
This actually displays a pretty complete summary of the real vi with
unsupported features indicated appropriately.
Certain features controlled by "set" parameters will have varied displays
on the Help screens, to correspond to the current
setting of their parameters.
.P
The help screens (there are multiple screens) can be traversed
using "reasonable" keystrokes.  For instance, any key that would take you
"down" in the vi editor, will take you to the next help screen in
the help facility.  Similarly, "up" commands will take you to
the previous screen. 
The screens are numbered, and any numeric key will take you to
the corresponding screen.
The first screen (screen 0) is a table of contents for the
help screens.
.H 1 "String Searches"
String searches are supported, as in vi, accepting the usual regular
expression syntax. This was done using a modified form of
Henry Spencer's regular expression
library. I added code outside the library to support
the '\\<' and '\\>' extensions.
The parameter "ignorecase" can be set to ignore case in all string searches.
.H 1 "Operators"
The vi operators (d, c, y, !, <, and >) work as true operators.
The tilde command may also be used as an operator if the parameter "tildeop"
has been set. By default, this parameter is not set.
.H 1 "Tags"
Tags are implemented and a fairly simple version of 'ctags' is supplied
with the editor. The current version of ctags will find functions and
macros following a specific (but common) form.  See 'ctags.doc' for a
complete discussion.
.P
A non-standard but useful variant of tags is the "stacking" of calls to
the ':ta' command.
When tag-stacking is enabled (with ':set tg'), stevie remembers
(and stacks) the position from which any tag call is made.
In this mode, a new ':untag' command returns to the remembered position
and pops the stack.
Also while in the tagstack mode, the Ctrl-^ keystroke has a somewhat
different effect.  Before it switches to edit the alternate file
(its normal function), it checks to see if the tag stack is empty,
and does an ':untag' if not empty.
.H 1 "System-Specific Comments"
The following sections provide additional relevant information for the
systems to which STEVIE has been ported.
(WARNING: If you compile Stevie for, or port Stevie to,
your favorite environment, don't forget to edit ENV.H
to correspond to the target environment.)
.H 2 "Atari ST"
.H 3 "TOS"
The editor has been tested in all three resolutions, although low and
high res. are less tested than medium. The 50-line high res. mode can
be used by setting the 'lines' parameter to 50. Alternatively, the
environment variable 'LINES' can be set. The editor doesn't actively
set the number of lines on the screen. It just operates using the number
of lines it was told.
.P
The arrow keys, as well as the <INSERT>, <HELP>, and <UNDO> keys are
all mapped appropriately.
.H 3 "Minix"
The editor is pretty much the same under Minix, but many of the
keyboard mappings aren't yet supported.
.H 2 "UNIX"
The editor has been ported to UNIX System V release 3 as well as 4.2 BSD.
This was done
mainly to get some profiling data so I haven't put much effort into
doing the UNIX version right.
While the termcap routines are supported, the editor is still fairly
picky about the capabilities it wants and makes little effort to
do clever things with less intelligent terminals.
.H 2 "OS/2"
This port was done because the editor that comes with the OS/2 developer's
kit really stinks. Make sure 'ansi' mode is on (using the 'ansi' command).
The OS/2 console driver doesn't support insert/delete line, so STEVIE
bypasses the driver and makes the appropriate system calls directly.
This is all done in the system-specific part of the editor so the kludge
is at least localized.
.P
The arrow keys, page up/down and home/end all do what
you'd expect. The function keys are hard-coded to some useful macros until
I can get true support for macros into the editor. The current mappings
are:
.DS 1
F1    Help
F2    Next file (:n)             \h'|3.3i'Shift-F2  discard changes (:n!)
F3    Previous file (:N)         \h'|3.3i'Shift-F3  discard changes (:N!)
F4    Alternate file (:e #)      \h'|3.3i'Shift-F4  discard changes (:e! #)
F5    Rewind file list (:rew)    \h'|3.3i'Shift-F5  discard changes (:rew!)
F6    Next function (]])         \h'|3.3i'Shift-F6  Prev. function ([[)
F8    Global subst. (:1,$s/)
F9    Convert cdecl to English   \h'|3.3i'Shift-F9  Convert English to cdecl
F10   Save & quit (:x)           \h'|3.3i'Shift-F10 discard changes (:q!)
.DE
.P
Function keys F9 and Shift-F9 assume the presence of the cdecl program.
.H 2 "MSDOS"
STEVIE has been ported to MSDOS 3.3 using
(1) the Microsoft C compiler, version 5.1, and
(2) the Borland Turbo C compiler, version 2.0.
.P
The keyboard mappings are the same as for OS/2.
The only problem with the PC version is that the inefficiency of
the screen update code becomes painfully apparent on slower machines.
(In Rev 3.69a, there are a couple of ways in which screen I/O
is significantly speeded up.)
.P
There are two DOS versions:
one that uses an extended console driver,
and another that uses the BIOS directly.
The extended-console-driver version works well with "nansi.sys",
which is widely available shareware.
It does not work well with the "ANSI.SYS" driver included as part of
MSDOS (at least through the 3.30 version).
In order to be independent of the console driver,
you may compile with BIOS #defined;
this uses the BIOS as the exclusive screen I/O mechanism.
(Note that it is somewhat slower than nansi.sys.)
.P
The color settings are the video attributes for the alphanumeric modes
of the CGA and monochrome displays.
While these make most sense in binary or hex, they must be entered
in decimal.  The reference table below gives a few common values:
.DS 1
07    White on black          Initial default
23    White on blue
30    Bright yellow on blue   My favorite
.DE
.H 1 "Missing Features"
.AL
.LI
The ability to edit files larger than the available memory.
This isn't a problem on the machines I use, but it hits the
Minix-PC people pretty hard.
.LI
Macros with support for function keys.
.LI
More "set" options.
.LI
Many others...
.LE
.H 1 "Known Bugs and Problems"
.AL
.LI
The yank buffer uses statically allocated memory, so large yanks
will fail. If a delete spans an area larger than the yank buffer,
the program asks
for confirmation before proceeding. That way, if you were moving text,
you don't get screwed by the limited yank buffer. You just have to move
smaller chunks at a time. All the internal buffers (yank, redo, etc.)
need to be reworked to allocate memory dynamically. The 'undo' buffer
is now dynamically allocated, so any change can be undone.
.LI
If you stay in insert mode for a long time, the insert buffer can overflow.
The editor will print a message and dump you back into command mode.
.LI
The current version of the substitute and global commands
(i.e. ":s/foo/bar" or ":g/foo/d") can't
be undone.
This is due to the current design of the undo code.
To undo these
commands would generally involve unreasonable amounts of memory.
.LI
Several other less bothersome glitches...
.LE
.SK
.H 1 "Conclusion"
The editor has reached a pretty stable state, and performs well on
the systems I use it on, so I'm pretty much in maintenance mode now.
There's still plenty to be done; the screen update code is still pretty
inefficient and the yank/put code is still primitive.
I'm still interested in bug reports, and I do still add a new feature
from time to time, but the rate of change is way down now.
.P
I'd like to thank Tim Thompson for writing the original version of the
editor. His program was well structured and quite readable. Thanks for
giving me a good base to work with.
Thanks also to many users of STEVIE who have sent in their changes.
Many of the changes I've received aren't portable to all the systems
I support, but I'm working to get portable implementations integrated
into the editor where possible.
.P
If you're reading this file, but didn't get the source code for STEVIE,
it can be had by sending a disk with return postage to the address given
below. I can write disks for the Atari ST (SS or DS) or MSDOS (360K or
1.2M). Please be sure to include the return postage. I don't intend to
make money from this program, but I don't want to lose any either.
.DS 1
Tony Andrews		UUCP: onecom!wldrdg!tony
5902E Gunbarrel Ave.
Boulder, CO 80301
.DE
.DS 1
3.69a Additions from
Dave Tutelman           UUCP: pegasus.att.com!dmt
Larry Shurr (The BIOS implementation)
.DE
.SK
.HU "Character Function Summary"
The following list describes the meaning of each character that's used
by the editor. In some cases characters have meaning in both command and
insert mode; these are all described.
.SP 2
.VL 8
.LI ^@
The null character. Not used in any mode. This character may not
be present in the file, as is the case with vi.
.LI ^B
Backward one screen.
.LI ^D
Scroll the window down one half screen.
.LI ^E
Scroll the screen up one line.
.LI ^F
Forward one screen.
.LI ^G
Same as ":f" command. Displays file information.
.LI ^H
(Backspace) Moves cursor left one space in command mode.
In insert mode, erases the last character typed.
.LI ^J
Move the cursor down one line.
.LI ^L
Clear and redraw the screen.
.LI ^M
(Carriage return) Move to the first non-white character
in the next line. In insert mode, a carriage return opens a new
line for input.
.LI ^N
Move the cursor down a line.
.LI ^P
Move the cursor up a line.
.LI ^U
Scroll the window up one half screen.
.LI ^Y
Scroll the screen down one line.
.LI ^[
Escape cancels a pending command in command mode, and is used to
terminate insert mode.
.LI ^]
Moves to the tag whose name is given by the word in which the cursor
resides.
.LI ^`
Same as ":e #" if supported (system-dependent).
.LI SPACE
Move the cursor right on column.
.LI !
The filter operator always operates on a range of lines, passing the
lines as input to a program, and replacing them with the output of the
program. The shorthand command "!!" can be used to filter a number of
lines (specified by a preceding count). The command "!" is replaced
by the last command used, so "!!!<RETURN>" runs the given number of
lines through the last specified command.
.LI $
Move to the end of the current line.
.LI %
If the cursor rests on a paren '()', brace '{}', or bracket '[]',
move to the matching one.
.LI \'
Used to move the cursor to a previously marked position, as
in 'a or 'b. The cursor moves to the start of the marked line. The
special mark '' refers to the "previous context".
.LI +
Same as carriage return, in command mode.
.LI ,
Reverse of the last t, T, f, or F command.
.LI -
Move to the first non-white character in the previous line.
.LI .
Repeat the last edit command.
.LI /
Start of a forward string search command. String searches may be
optionally terminated with a closing slash. To search for a slash
use '\\/' in the search string.
.LI 0
Move to the start of the current line. Also used within counts.
.LI 1-9
Used to add 'count' prefixes to commands.
.LI :
Prefix character for "ex" commands.
.LI ;
Repeat last t, T, f, or F command.
.LI <
The 'left shift' operator.
.LI >
The 'right shift' operator.
.LI ?
Same as '/', but search backward.
.LI A
Append at the end of the current line.
.LI B
Backward one blank-delimited word.
.LI C
Change the rest of the current line.
.LI D
Delete the rest of the current line.
.LI E
End of the end of a blank-delimited word.
.LI F
Find a character backward on the current line.
.LI G
Go to the given line number (end of file, by default).
.LI H
Move to the first non-white char. on the top screen line.
.LI I
Insert before the first non-white char. on the current line.
.LI J
Join two lines.
.LI L
Move to the first non-white char. on the bottom screen line.
.LI M
Move to the first non-white char. on the middle screen line.
.LI N
Reverse the last string search.
.LI O
Open a new line above the current line, and start inserting.
.LI P
Put the yank/delete buffer before the current cursor position.
.LI R
Replace characters until an "escape" character is received.
Similar to insert mode, but replaces instead of inserting.
Typing a newline in replace mode is the same as in insert mode,
but replacing continues on the new line.
.LI T
Reverse search 'upto' the given character.
.LI U
Restore the current line to its state before you started changing it.
.LI W
Move forward one blank-delimited word.
.LI X
Delete one character before the cursor.
.LI Y
Yank the current line. Same as 'yy'.
.LI ZZ
Exit from the editor, saving changes if necessary.
.LI [[
Move backward one C function.
.LI ]]
Move forward one C function.
.LI ^
Move to the first non-white on the current line.
.LI `
Move to the given mark, as with '. The distinction between the two
commands is important when used with operators. I support the
difference correctly. If you don't know what I'm talking about,
don't worry, it won't matter to you.
.LI a
Append text after the cursor.
.LI b
Back one word.
.LI c
The change operator.
.LI d
The delete operator.
.LI e
Move to the end of a word.
.LI f
Find a character on the current line.
.LI h
Move left one column.
.LI i
Insert text before the cursor.
.LI j
Move down one line.
.LI k
Move up one line.
.LI l
Move right one column.
.LI m
Set a mark at the current position (e.g. ma or mb).
.LI n
Repeat the last string search.
.LI o
Open a new line and start inserting text.
.LI p
Put the yank/delete buffer after the cursor.
.LI r
Replace a character.
.LI s
Replace characters.
.LI t
Move forward 'upto' the given character on the current line.
.LI u
Undo the last edit.
.LI w
Move forward one word.
.LI x
Delete the character under the cursor.
.LI y
The yank operator.
.LI z
Redraw the screen with the current line at the top (zRETURN),
the middle (z.), or the bottom (z-).
.LI |
Move to the column given by the preceding count.
.LI ~
Invert the case of the current character (if alpha) and move to the right.
If the parameter "tildeop" is set, this command functions as an operator.
.LE
.de TX
.ce
STEVIE - User Guide
.sp
..
.TC
