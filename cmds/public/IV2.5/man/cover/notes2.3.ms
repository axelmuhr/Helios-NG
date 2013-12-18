.ll 6.0i
.po 1.25i
.ps +1
.vs +1
.rs
.sp 7
.ps +6
.ce
Release Notes
.sp 0.5
.ps -4
.ce
Version 2.3
.ps -2
.sp 1
.ti +5n
This distribution contains four directories: \fIbin\fP, \fIlib\fP,
\fIsrc\fP, and \fIman\fP.  The \fIbin\fP and \fIlib\fP directories
are further subdivided according to target machine and version of X.
We have binaries for VAX.X10, VAX.X11, and SUN3.X11.
The \fIsrc\fP directory is divided into five subdirectories:
\fIinclude\fP, \fIlib\fP, \fIlibgraphic\fP, \fIlibtext\fP, and \fIbin\fP.
All header files are under \fIinclude\fP.
The general library classes are in \fIlib\fP,
the structured graphics classes are in \fIlibgraphic\fP, and
text and structured text classes are in \fIlibtext\fP.
Each command is in its own subdirectory under \fIbin\fP.
We keep \fIinclude\fP files in ``/usr/include/InterViews'', but you
can make the library and programs with the header files
in a different place by creating a symbolic link
to the include directory named ``InterViews''.
.sp 1
.ti +5n
The binaries for a source directory are built in a subdirectory
named by the target machine and X version.
For example, the \fIlib\fP directory contains a subdirectory
named ``SUN3.X11'' for binaries that run on a Sun-3 accessing a display
running X11.
The \fIlibtext\fP and \fIlibgraphics\fP libraries, which are
not dependent on X, only need subdirectories for the target machine
(e.g., SUN3).
The \fIsrc\fP directory contains an executable shell script named
``genmake'' that constructs ``Makefile'' for a binary subdirectory
using a generic makefile named ``genMakefile''.
The generic makefile contains definitions and targets, but no
dependencies.
For example, ``genmake\ SUN3\ X11'' creates ``SUN3.X11/Makefile''
with the header file dependencies.
In the top \fIsrc\fP directory there is a makefile to build
all the libraries and commands from scratch.
.sp 1
.ti +5n
Please address any questions, bugs, or comments about InterViews to
.in +5n
.nf
.sp 1
Mark Linton
Center for Integrated Systems, Room 213
Stanford University
Stanford, CA 94305
.sp 1
Internet: linton@lurch.stanford.edu
UUCP:     shasta!linton, decwrl!linton, or ucbvax!linton
