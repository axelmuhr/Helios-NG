.nr PO 1.25in
.DA "29 May 1989"
.TL
.ps +6
Changes from Version 2.4 to Version 2.5
.ps -6
.SH
.ps +2
Overview
.ps -2
.PP
This document describes the changes made to InterViews since the
release of Version 2.4 in September 1988, which was
included in the X11R3 distribution from MIT.
The major changes in 2.5 are the inclusion of g++, the GNU C++ compiler;
conformance to the window manager and session manager protocols
defined by the X11 Inter-Client Communication Conventions Manual (ICCCM);
support for raster images in both immediate and structured graphics;
new classes for text editing; and reliance on the X11 build tools
\fIimake\fP and \fImakedepend\fP.
Also, the communications classes that interface to Unix IPC have been
incorporated into the base library; \fIliballegro\fP no longer exists
as a separate directory.
.PP
For the most part, changes are upward compatible with Version 2.4.  A
notable exception is the elimination of the virtual \fIDelete\fP
function used in conjunction with destructors.  Although AT&T cfront
1.2 generates working code when it encounters this function in 2.4,
the usage is not legal C++ and does not work with g++ or other C++
compilers.
.SH
.ps +2
g++ compiler
.ps -2
.PP
Version 2.5 includes a copy of g++\ 1.35.1-.  InterViews also works
with AT&T cfront 1.2 and should be very close to working with cfront
2.0 and Oregon C++.  At Stanford we use AT&T cfront 1.2 on DECstation
3100s and g++ on Sun-4s.
.PP
To allow usage in different compilation environments,
a complete set of header files
that interface to C library functions is provided
under the \fIStd\fP subdirectory.
The base library also includes an implementation of the C++ stream class,
enabling InterViews applications to be built with
either AT&T cfront or g++.
InterViews includes a CC compiler driver command
that can call either g++ or cfront.
.PP
Because none of the InterViews code depends on libg++
(the GNU C++ library),
an application written with InterViews and compiled with g++ (or cfront)
is not considered to be derived from GNU software.
Thus, there continue to be no licensing restrictions
on InterViews-based applications.
.PP
Many InterViews 2.4 classes had destructors that called a virtual
Delete function to delete storage for objects whose derived type had
been lost.  Using InterViews with g++ uncovered the fact that the C++
language does not permit a destructor to call a subclass virtual
function; AT&T cfront 1.2 simply does this incorrectly.  Only by
switching to virtual destructors could we guarantee that storage for
such objects would be freed.
.PP
The change to virtual destructors will cause three incompatibilities
with existing code.  First, you must replace virtual Delete functions
you have written with virtual destructors.  Second, Rubberbands no
longer automatically erase themselves when deleted, since Erase is
virtual.  Thus if you use rubberbands, you must now erase them
explicitly before deleting them.  Third, AT&T cfront 1.2 does not
generate code to check for a nil pointer value when deleting an object
with a virtual destructor.  Deleting an object through such a pointer
will require dereferencing nil, which will crash the program on many
architectures.  If you use AT&T cfront 1.2 and have code that deletes
a pointer to an object with a virtual destructor, your code should
check for nil before deleting the object.
.SH
.ps +2
ICCCM compliance
.ps -2
.PP
Inter-Client Communication Conventions Manual
by David Rosenthal (dshr@sun.com)
defines conventions for X applications in
the following areas: selection mechanism, cut-buffers, window manager,
session manager, and manipulation of shared workstation resources.
The InterViews library follows the conventions
described in sections 4 and 5 for
window manager support (except for the properties WM_COLORMAP_WINDOWS
and WM_PROTOCOLS) and for session manager support.
.PP
The ICCCM requires each top-level interactor to specify certain
properties of the window it occupies.  The set of properties depends
on the purpose of the top-level interactor.  To specify this purpose,
programmers can use one of five new methods defined by World:
InsertIcon, InsertPopup, InsertTransient, InsertToplevel, and
InsertApplication.  The World manual page describes these methods
in more detail.  The original Insert method is retained and is
equivalent to InsertToplevel.  Only programs that explicitly insert
popup menus will break if they continue to use Insert
(they should use InsertPopup).
.PP
The ICCCM also specifies conventions (not yet completely followed by
most window managers) by which an interactor may inform a window
manager how to display its icon and when to iconify or deiconify
itself.  The following methods were added to Interactor to let
instances set or get their icon attributes: IconName, IconInteractor,
IconGeometry, IconBitmap, IconMask, and StartIconic.  The old Iconify
and DeIconify methods have been reimplemented to follow the ICCCM
conventions.  InterViews includes a window manager, twm, that has been
patched to recognize the conventions followed by Iconify and
DeIconify.
.SH
.ps +2
New graphics features
.ps -2
.IP Bitmap
.br
The Bitmap class no longer defines a Draw member
function.  Bitmaps are now used solely to store a binary image.  They
can be drawn with the new painter member function Stencil, which also
allows the image to be masked by a second bitmap.  Bitmaps can also be
used to define window manager icons, to construct fill patterns, and
to construct cursors.
.IP Brush
.br
Dashed Brushes are now fully supported, and a new Brush
constructor has been added to allow more general specification of
brush characteristics.
.IP Color
.br
A new function, DisplayIntensities, has been added to the
Color class to return the actual RGB intensities used to display the
color.  The Intensities function now returns the RGB values used to
specify the Color.
.IP Cursor
.br
New constructors have been added to the Cursor class to allow cursors
to be created from a Bitmap, a Font character, or a standard cursor
identifier.
.IP Raster
.br
A new class, Raster, stores a full-color image.
Rasters are drawn with Painter's new RasterRect member function.
.IP Painter
.br
Painters now use their current transformation matrix for all
operations.  In particular, text, bitmaps, and rasters are now drawn
accurately for arbitrary transformations.  Several new functions have
been added to the class: Stencil draws bitmaps, RasterRect draws
rasters, and SetStyle specifies styling to apply when drawing text.
Valid text styles include underlining, boldface, and reversed colors.
The functions that draw curved lines have been renamed to avoid
confusion: Arc has been renamed Curve, and ArcTo has been renamed
CurveTo.  Finally, Painter now reference counts its graphics
resources.
.IP RasterRect and Stencil
Two new subclasses of Graphic have been added to libgraphic to support
bitmaps and rasters in structured graphics: the Stencil graphic
subclass draws a bitmap, while the RasterRect subclass draws a Raster.
.SH
.ps +2
Text editing
.ps -2
.PP
The base library provides a new set of text editing classes.  The old
text editing classes have been kept in libtext for backward
compatibility, but the new ones are simpler and more powerful.
.IP Regexp
.br
A new class, Regexp, supports regular expression
searching and matching in text strings.  Regexp understands the same
pattern syntax as \fIed\fP or \fIgrep\fP.
.IP StringEditor
.br
A new interactor subclass, StringEditor, replaces the old class
StringEdit.  StringEditor has a similar interface but differs in two
main aspects: the status of an editing operation is now reported via a
ButtonState, and the function that returns the current value of the
string no longer allocates storage for the result.  StringEditor does
not yet support cut-and-paste editing; this support will be added when
general support for selections is added to InterViews.
.IP TextBuffer
.br
A new class, TextBuffer, supports common editing, searching, and
positioning operations for unstructured text.
The name of the old TextBuffer class has been changed to OldTextBuffer.
.IP TextDisplay
.br
A new class, TextDisplay, displays multi-line unstructured text and
preforms efficient screen updates following changes to the text.
TextDisplay is useful in interactor subclasses that display changeable
text.
.IP TextEditor
.br
A new interactor subclass, TextEditor, replaces the
old class TextViewer.
TextEditor provides an interactive
interface to a TextBuffer and supports a range of scrolling and
selecting operations.
.SH
.ps +2
Other new features
.ps -2
.IP Interactor
The virtual functions Activate and Deactivate are now called
when an interactor's canvas is mapped and unmapped, respectively.
This is useful, for example, to recognize when an interactor
has been iconified or deiconified.
The default functions do nothing.
.IP Scene
.br
Two new virtual functions have been added to the Scene
class to allow Scene subclasses to control the raising and lowering of
their components: DoRaise is called by Raise, and DoLower is called by
Lower.  Also, the UserPlace function now takes two additional arguments.
.IP World
.br
A new function, SetHint, has been added to the World class.  SetHint
provides a system-independent interface to system-dependent behavior.
Currently supported hints control rubberbanding behavior and the
drawing of transformed text.
.SH
.ps +2
Applications
.ps -2
.IP genclass
A new program, genclass, generates C++ source from template files
using simple string substitution.  In InterViews it is used to
instantiate classes with type parameters.
This program will not be needed when C++ provides
parameterized classes.
.IP graphics
.br
The graphics demo uses
new Stencil and RasterRect structured graphics
objects and allows the user to scale and rotate objects.
.IP idraw
.br
Idraw now displays transformed text correctly, and the user can
specify grid spacing in points.
.IP sted
.br
The sted demo text editor has been rewritten to use the new
TextEditor, TextBuffer, and StringEditor classes.  It is not a
complete text editor, but it can serve as an example of how a
practical one might be built.
.SH
.ps +2
Configuration management
.ps -2
.PP
The X11 configuration management tools \fIimake\fP and \fImakedepend\fP
have replaced the \fIdomake\fP, \fIgenmake\fP, and \fImkdepend\fP
shell scripts to make the installation process easier.
Imake runs the C preprocessor on a
template file in a configuration directory.  The template file
includes several configuration files to define build parameters and
rules.  The template file also includes the Imakefile in the current
working directory.  Imake writes out the result as the new Makefile.
Makedepend reads the source files, generates Makefile dependencies
between the object files and the source files used to build them, and
adds those dependencies to the Makefile.
.PP
The config and config/InterViews directories, which are new to
InterViews 2.5, contain the platform-specific .cf files and InterViews
template and rules files.  The file config/InterViews/README describes
how to configure InterViews for your system before building it.
