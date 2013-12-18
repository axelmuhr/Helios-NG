.nr PO 1.25in
.DA "29 May 1989"
.TL
.ps +6
Release Notes for InterViews 2.5
.ps -6
.PP
The InterViews distribution is split into two main subdirectories,
one named \fIiv\fP, which contains InterViews source and associated files, and
one named \fIg++\fP, which contains the GNU C++ compiler.
The InterViews libraries and applications do not require g++;
you can use a different C++ compiler if you wish.
The top-level Makefile will try to build g++.
If you do not want to use g++, then chdir to \fIiv\fP and
make from there.
.PP
The file \fIiv/README\fP describes how to build InterViews.
Under \fIiv/src\fP, the directory \fIInterViews\fP contains include files,
\fIbin\fP contains applications (each one in its own subdirectory),
\fIlibInterViews\fP contains the base library classes,
\fIlibgraphic\fP contains the structured graphics library, and
\fIlibtext\fP contains the structured text library.
.PP
The \fIsted\fP demo text editor will not link on non-BSD platforms
because the regular expression class (Regexp) is only implemented
using the BSD library routines (\fIre_comp\fP and \fIre_exec\fP).
It should be straightforward to port it to use the System V
library routines, but we did not have time to do so before the release.
.PP
Three publicly available applications are included with InterViews
because they are necessary for the configuration and operation of InterViews.
\fIImake\fP and \fImakedepend\fP come from the X11R3 core
distribution, with post-R3 improvements from Jim Fulton at MIT.
The \fItwm\fP window manager is from Tom LaStrange at Hewlett-Packard
in Fort Collins, Colorado.
.PP
The directory \fIiv/man\fP contains troff sources for the
InterViews release notes and manual pages.
The directory \fIiv/doc\fP contains LaTeX and PostScript
for three papers and a one-page overview of InterViews.
The \fIcomposition\fP subdirectory contains
the paper titled ``Composing User Interfaces with InterViews''
that was published in the February 1989 issue of \fIComputer\fP.
The \fIlibgraphic\fP subdirectory contains
the paper titled ``Applying Object-Oriented Design to Structured Graphics''
that was published in the 1988 C++ Conference proceedings.
The \fItoolkit\fP subdirectory contains
the paper titled ``InterViews: A C++ Graphical Interface Toolkit''
that is a revised version of the paper published
in the 1987 C++ Conference proceedings.
.PP
Please address questions or comments about InterViews to
.in +5n
.nf
.sp 1
Mark Linton
Center for Integrated Systems, Room 213
Stanford University
Stanford, CA 94305
.sp 1
Internet: linton@interviews.stanford.edu
