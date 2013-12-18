.ll 6.0i
.po 1.25i
.ps +1
.vs +1
.rs
.sp 7
.ps +4
.ce
Changes from Version 2.1 to Version 2.2
.ps -4
.sp 1
.ti +5n
This note briefly describes the changes made to InterViews since
the 2.1 release that went out with the X11R1 distribution in September.
Most importantly, a large number of problems have been fixed for X11.
InterViews was originally written for X10 and ported using
a preliminary release of X11.  We are now using X11 regularly and
are running InterViews applications on top of it.  We also continue to use
X10 on some systems.
.sp 1
.ti +5n
The second important part of this 2.2 release is a new library
of classes to manage structured text (called \fIlibtext\fP),
described in the Text(3I) manual page.
The library also includes some classes for interactive text manipulation,
including StringEdit(3I) and TextViewer(3I).
The \fIpages\fP program demonstrates some of the facilities
of this library.
.sp 1
.ti +5n
The InterViews library contains three new classes.
Deck(3I) is a subclass of scene that contains
interactors (conceptually) stacked on top of each other.
A deck is useful to implement ``paging'' through a document, or
for alternate tool pallets or menu entries.
Bitmap(2I) is a primitive for manipulating bitmap data.
It can be used to draw an image created with the X bitmap program
onto an interactor's canvas.
ReqErr(2I) is a primitive for catching errors from X.
.sp 1
.ti +5n
The Workstation class has been eliminated.
All its functionality is now in the World class.
.sp 1
.ti +5n
For a variety of reasons, the name of the base structured graphics class
was changed from Picture to Graphic, and Picture is now
the name of the subclass of Graphic that represents a group
of objects.
A new class, GraphicBlock, can be used to define an interactor
that contains structured graphics.
The \fIgraphics\fP program demonstrates how to use the
structured graphics library to define persistent graphical objects.
.sp 1
.ti +5n
The scaled digital clock program, \fIdclock\fP has been rewritten.
It previously used an old interface, which was not distributed
as part of InterViews 2.1.
The old interface (known as libInterViews1.0)
is now only used by \fIidraw\fP, and
the source code for it is contained in this distribution under
the \fIidraw\fP source subdirectory.
The \fIishell\fP command is no longer distributed.
.sp 1
.ti +5n
We are currently working on several applications on top of InterViews,
including a rewrite of \fIidraw\fP in C++ (using \fIlibgraphic\fP),
a structure editor, and a window-oriented debugger.
We are also working on a library for general support
of drawing systems.
