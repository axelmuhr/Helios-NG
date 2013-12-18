.ll 6.0i
.po 1.25i
.ps +1
.vs +1
.rs
.sp 7
.ps +4
.ce
Changes from Version 2.3 to Version 2.4
.ps -4
.sp 1
.ti +5n
This note briefly describes the changes made to InterViews since
the 2.3 release that became available in early March 1988.
The two major changes are the use of customization information
as in the X Toolkit and a C++ rewrite of idraw, the drawing editor,
which was originally written in Modula-2.
Several new classes have been added, some existing classes have been
enhanced, and a number of bugs have been fixed.
.sp 1
.ti +5n
PropertySheet is a hierarchy of user-customizable attributes.
Painter attributes, such as font and colors, are configured automatically
based on the name of an interactor instance or class.
An instance name may be passed as the first argument
to any interactor constructor.
If no attributes are defined for a specific class or instance,
they are inherited from the interactor's parent.
Thus, it is no longer necessary or appropriate to pass a painter
to an interactor constructor.
For backward compatibility, the painter-based constructors are still
present in the library.
However, they are not documented and should be not be used in new code.
.sp 1
.ti +5n
Because an interactor's output painter attributes
may depend on user preferences that are inherited from ancestors,
interactor constructors cannot assume that output
has been set up correctly.
We have added a new virtual operation on interactors, called Reconfig,
that should perform any necessary operations using output.
Some interactors compute their shape based on their output font
(e.g., if they contain text), so scenes should compute
their shapes in their Reconfig operations.
.sp 1
.ti +5n
Tray is a new scene of interactors that overlap, tile, or
have other constraints on their alignment relative to each other.
One interactor serves as the background for the tray.
Dialog is a new monoscene that handles input for a dialog box.
Message is a simple interactor that contains a line of text.
.sp 1
.ti +5n
StringPool, StringTable, and Table are three new classes
that provide simple table management.
StringPool manages an area of character storage;
StringTable uses a pool to implement a hash table for strings.
Table implements a hash table for integers or arbitrary pointers.
.sp 1
.ti +5n
Painter::Text can now optionally take coordinates
that specify the lower-left corner of the text,
eliminating for the most part the need to use Painter::MoveTo.
.sp 1
.ti +5n
A new library is included that classes for interprocess communication.
The library, called ``liballegro'', is the runtime support
a set of programming environment tools that are collectively
called Allegro.  We expect to distribute these tools
with InterViews as they become usable.
.sp 1
.ti +5n
There have also been several internal changes to the implementation.
We have reorganized the source so that all the
window system dependent routines are isolated in two files.
The source makefiles now all use relative pathnames for includes;
there are no more symbolic links.
