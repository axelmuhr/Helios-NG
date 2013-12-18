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
Version 2.4
.ps -2
.sp 1
.ti +5n
This distribution contains five directories: \fIbin\fP, \fIlib\fP,
\fIsrc\fP, \fIman\fP, and \fIdoc\fP.
As a side effect of making the system,
the \fIbin\fP and \fIlib\fP directories
will be further subdivided according to target machine and
window system (e.g., X10 or X11).
The \fIsrc\fP directory is divided into subdirectories
for include files, each of the libraries, and a subdirectory
for commands (\fIbin\fP).
The general user interface classes are in \fIlibInterViews\fP,
the structured graphics classes are in \fIlibgraphic\fP, and
text and structured text classes are in \fIlibtext\fP.
A library of classes for interprocess communication are in \fIliballegro\fP.
Each command is in its own subdirectory under \fIbin\fP.
.sp 1
.ti +5n
The binaries for a source directory are built in a subdirectory
named by the target machine.
The X environment variable determines
the target window system for building a binary.
The top-level build script sets X to X11.
The \fIlibInterViews\fP directory also contains subdirectories
identified by target machine and window system,
e.g., the subdirectory
named ``SUN3.X11'' contains the object files for a Sun-3
running X11.
.sp 1
.ti +5n
The \fIbin/all\fP directory contains an executable shell script named
``genmake'' that constructs ``Makefile'' for a binary subdirectory
using a generic makefile named ``genMakefile''.
The generic makefile contains definitions and targets, but no
dependencies.
For example, ``genmake\ VAX'' creates ``VAX/Makefile''
with the header file dependencies.
The ``genmake'' script uses the script ``mkdepend'' to generate
the dependencies; this script is also in \fIbin/all\fP.
Because of Ultrix differences, a special version of ``mkdepend''
is in \fIbin/VAX\fP.
.sp 1
.ti +5n
The \fIsrc\fP directory contains a makefile to build
all the libraries and commands from scratch.
The makefile uses a shell script ``domake'' that sets
its path to include the appropriate \fIbin\fP subdirectories.
The first thing ``domake'' does is figure out the target machine,
which it does by compiling and executing ``cpu.c''.  If your target machine
is other than those listed in ``cpu.c'', you should edit the file and add
the appropriate #ifdef and printf.
You may also need to modify the istat and idraw genMakefiles
for system-dependent external libraries.
Once ``cpu.c'' is correct, you can build all the libraries and commands
with ``make install''.
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
