.ll 6.0i
.po 1.25i
.ps +1
.vs +1
.rs
.sp 7
.ps +6
.ce
Preface
.ps -6
.sp 1
.ti +5n
The InterViews system supports the design and implementation
of applications with graphical user interfaces.
The basic model of InterViews is \fIobject-oriented\fP,
with C++ as the implementation language.
This manual describes InterViews version 2.5,
which runs as library code on top of the X window system
(the library can be compiled to run on either X10 or X11).
If you are not running X already,
you should bring it up before trying to use
the programs and library routines documented here.
The manual also assumes familiarity with C++.
.sp 1
.ti +5n
The manual is organized into four sections:
commands, the basic library (libInterViews),
a structured graphics library (libgraphic), and
a structured text library (libtext).
See the 2.5 release notes for more information
about the distribution.
.sp 1
.ti +5n
A number of people have contributed
to the design and implementation of InterViews.
Paul Calder designed the boxes and glue model
of combining interactive objects, wrote many of the commands, and
implemented the structured text library.
John Vlissides wrote the original version of
the drawing editor, \fIidraw\fP(1), and the structured graphics library.
Both Paul and John have also contributed
to the design and implementation of the base library.
John Interrante wrote the current version of \fIidraw\fP and
added the ICCCM window manager protocol support to libInterViews.
Craig Dunwoody participated in the early design of InterViews.
We are grateful to the growing InterViews user community
for their encouragement and support.
.sp 1
.ti +5n
This work has been supported
by the Quantum project through a gift from Digital Equipment Corporation.
Special thanks go to Ira Machefsky of Digital for his efforts
in making this support possible.
.sp 2
.in +30n
.nf
Mark A. Linton
Stanford University
May 29, 1989
.fi
.in -30n
