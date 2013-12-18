.ll 6.0i
.po 1.25i
.ps +1
.vs +1
.rs
.sp 7
.ps +4
.ce
Changes from Version 2.2 to Version 2.3
.ps -4
.sp 1
.ti +5n
This note briefly describes the changes made to InterViews since
the 2.2 release that became available in early January 1988.
A number of the changes are bug fixes for X11, including
the Bitmap class (which didn't work at all on X11),
the painter CopyArea operation, and
the font Index operation.
.sp 1
.ti +5n
Viewport(3I) is a new scene that contains a single component.
Unlike MonoScene(2I), which gives its component exactly
as much space as the monoscene has, a viewport allocates a component
exactly as much space as it wants (determined from the shape).
This space will be clipped to the viewport size.
The viewport maintains a perspective to adjust the portion of the component
that is actually displayed.
.sp 1
.ti +5n
Adjuster(3I) and Panner(3I) are both views of a perspective.
An adjuster is a button that scrolls a fixed amount when pushed.
A panner allows two-dimensional scrolling and zooming.
.sp 1
.ti +5n
The Frame(3I) interface has changed slightly.
Frames now have a sensor that listens for on/off events.
The frame constructor therefore no longer takes a sensor as a parameter.
Frame::Handle operation calls a virtual Highlight with true
or false for an on or off event.
This organization simplies the implementation of subclasses.
A new frame subclass, BorderFrame, draws the frame using a gray pattern
when highlighting is off and a solid pattern when highlighting is on.
.sp 1
.ti +5n
The Scene(2I) Change operation has been simplified.
It now only takes an interactor; the semantics are that the interactor
is a child of the scene and has changed its shape.
A new Propagate operation can specify whether shape changes
are propagated up the scene hierarchy.
A new Move operation is used to change an interactor's position.
.sp 1
.ti +5n
Several new operations have been added to the base interactor class.
UnRead puts an event back on the input queue as if it were never read.
GetRelative translates coordinates to be relative to another interactor.
The MapX, MapY, and Event::GetRelative operations have been removed.
Reshape is a virtual that requests an interactor modify its shape.
GetComponents returns a dynamically-allocated array of interactor pointers
for the components within an interactor.
It is only meaningful for scenes.
.sp 1
.ti +5n
Three new patterns are now defined by the external variables
``lightgray'', ``gray'', and ``darkgray''.
These patterns corresponding to filling one-eighth, one-half, and seven-eighths
of an area, respectively.
.sp 1
.ti +5n
Damage(3I) is a new class in \fIlibgraphic\fP that performs
minimal screen update after a modification is made
to a structured graphic object.
.sp 1
.ti +5n
To make InterViews more portable,
we have replaced all appearances of \fIclass.operation\fP with
\fIclass::operation\fP, declared constants using ``static const'', and
added conditional compilation code for HP/UX (HP's version of Unix).
We have also changed the makefiles so that they do not use
the VPATH macro.
