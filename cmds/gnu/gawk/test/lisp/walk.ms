.\" troff -ms
.\" ...if no CW font available, change CW to B globally
.de ZB
.DS
.ft CW
.nf
..
.de ZE
.DE
.ft R
.fi
.br
..
.TL 
A LISP interpreter in Awk
.AU
Roger Rohrbach
.AI
1592 Union St.,  #94
San Francisco,  CA 94123
.ND
January 3,  1989
.AB ABSTRACT
.PP
This note describes a simple interpreter for the LISP programming
language,  written in \fBawk\fR.  It provides
intrinsic versions of the basic functions on s-expressions,  and many others
written in LISP.  It is compatible with the commonly
available version of \fBawk\fR that is
supplied with most UNIX systems.  The interpreter serves to illustrate
the use of \fBawk\fR for prototyping or implementing language
translators,  as well as providing a simple example of LISP
implementation techniques.
.AE
.SH
Intrinsic functions.
.PP
The interpreter has thirteen built-in functions.
These include the five elementary functions on s-expressions defined
by McCarthy [1];  some conditional expression operators;  an
assignment operator,  and some functions to control the evaluation process.
.PP
The intrinsic functions are summarized below.
Familiarity with existing LISP systems is assumed in the
descriptions of these functions.
.IP "\f(CW(car \fIl\fP)\fR"
.br
Returns the first element of the list \fIl\fR.
An error occurs if \fIl\fR is atomic.
.IP "\f(CW(cdr \fIl\fP)\fR"
.br
Returns the remainder of the list \fIl\fR,  \fIi.e.\fR,
the sublist containing the second through the last
elements.  If \fIl\fR has only one element,  \fBnil\fR is
returned.  \fBcdr\fR is undefined on atoms.
.IP "\f(CW(cons \fIe l\fP)\fR"
.br
Constructs a new list whose \fBcar\fR is \fIe\fR
and whose \fBcdr\fR is \fIl\fR.
.IP "\f(CW(eq \fIx y\fP)\fR"
.br
Returns \fBt\fR if \fIx\fR and \fIy\fR
are the same LISP object,  \fIi.e.\fR,  are either atomic and
have the same print name,  or evaluate to the same
list cell.  Otherwise,  \fBnil\fR is returned.
.IP "\f(CW(atom \fIs\fP)\fR"
.br
Returns \fBt\fR if \fIs\fR is an atom,  otherwise \fBnil\fR.
.IP "\f(CW(set \fIx y\fP)\fR"
.br
Assigns the value \fIy\fR to \fIx\fR and returns \fIy\fR.
\fIx\fR must be atomic,  and may not be a constant
or name an intrinsic function.
.IP "\f(CW(eval \fIs\fP)\fR"
.br
Evaluates \fIs\fR and returns the result.
.IP "\f(CW(error \fIs\fP)\fR"
.br
Halts evaluation and returns \fBnil\fR.
The atom \fIs\fR is printed.
.IP "\f(CW(quote \fIs\fP)\fR"
.br
Returns \fIs\fR,  unevaluated.  The form
.ZB
\'\fIexpr\fP
.ZE
is an abbreviation for
.ZB
(quote \fIexpr\fP)
.ZE
.IP "\f(CW(cond (\fIp1 \f(CW[\fIe1\f(CW]) \fI...\fP (\fIpN \f(CW[\fIeN\f(CW]))\fR"
.br
Evaluates each \fIp\fR from left to right.  If any
evaluate to a value other than \fBnil\fR,  the
corresponding \fIe\fR is evaluated and the result is returned.
If there is no corresponding \fIe\fR,  the value of the \fIp\fR
itself is returned instead.
If no \fIp\fR has a non-null value,  \fBnil\fR is returned.
.IP "\f(CW(and \fIe1 ...  eN\fP)\fR"
.br
Evaluates each \fIe\fR and returns \fBnil\fR if any evaluate to \fBnil\fR.
Otherwise  the value of the last \fIe\fR is returned.
.IP "\f(CW(or \fIe1 ...  eN\fP)\fR"
.br
Evaluates each \fIe\fR and returns the first whose
value is non-null.  If no such \fIe\fR is found,  \fBnil\fR is returned.
.IP "\f(CW(list \fIe1 ...  eN\fP)\fR"
.br
Constructs a new list with elements \fIe1 ...  eN\fR.
Equivalent to
.br
\f(CW(cons \fIe1\fP (cons \fI...\fP (cons \fIeN\fP nil)\fR.
.SH
Lambda functions.
.PP
The following functions are written in LISP and are
defined in the file \fBwalk.w\fR.  Most of
these are commonly supplied with LISP systems.
.IP "\f(CW(cadr \fIs\fP)\fR"
.IP "\f(CW(cddr \fIs\fP)\fR"
.IP "\f(CW(caar \fIs\fP)\fR"
.IP "\f(CW(cdar \fIs\fP)\fR"
.IP "\f(CW(cadar \fIs\fP)\fR"
.IP "\f(CW(caddr \fIs\fP)\fR"
.IP "\f(CW(cddar \fIs\fP)\fR"
.IP "\f(CW(cdadr \fIs\fP)\fR"
.br
These correspond to various compositions of
\fBcar\fR and \fBcdr\fR,  \fIe.g.\fR,
.br
\f(CW(cadr \fIs\fP)\fR \(-> \f(CW(car (cdr \fIs\fP))\fR.
.IP "\f(CW(null \fIs\fP)\fR"
.br
Equivalent to \f(CW(eq \fIs\fP nil)\fR.
.IP "\f(CW(not \fIs\fP)\fR"
.br
Same as \fBnull\fR.
.IP "\f(CW(ff \fIs\fP)\fR"
.br
Returns the first atomic symbol in \fIs\fR.
.IP "\f(CW(subst \fIx y z\fP)\fR"
.br
Substitutes \fIx\fR for all occurrences of the atom \fIy\fR in \fIz\fR.
\fIx\fR and \fIz\fR are arbitrary s-expressions.
.IP "\f(CW(equal \fIx y\fP)\fR"
.br
Returns \fBt\fR if \fIx\fR and \fIy\fR are
the same s-expression,  otherwise \fBnil\fR.
.IP "\f(CW(append \fIx y\fP)\fR"
.br
Creates a new list containing the elements of x and y,
which must both be lists.
.IP "\f(CW(member \fIx y\fP)\fR"
.br
Returns \fBt\fR if \fIx\fR is an element of the list \fIy\fR,
otherwise \fBnil\fR.
.IP "\f(CW(pair \fIx y\fP)\fR"
.br
Pairs each element of the lists \fIx\fR and \fIy\fR,
and returns a list of the resulting pairs.  The number
of pairs in the result will equal the length of the
shorter of the two input lists.
.IP "\f(CW(assoc \fIx y\fP)\fR"
.br
Association list selector function.
\fIy\fR is a list of the
form \f(CW((\fIu1\fP \fIv1\fP) \fI...\fP (\fIuN\fP \fIvN\fP))\fR
where the \fIu\fR's are atomic.  If \fIx\fR is
one of these,  the corresponding pair \f(CW(\fIu\fP \fIv\fP)\fR
is returned,  otherwise \fBnil\fR.
.IP "\f(CW(sublis \fIx y\fP)\fR"
.br
\fIx\fR is an association list.
Substitutes the values in \fIx\fR for the keys in \fIy\fR.
.IP "\f(CW(last \fIl\fP)\fR"
.br
Returns the last element of \fIl\fR.
.IP "\f(CW(reverse \fIl\fP)\fR"
.br
Returns a list that contains the elements in \fIl\fR,
in reverse order.
.IP "\f(CW(remove \fIe l\fP)\fR"
.br
Returns a copy of \fIl\fR with all
occurrences of the element \fIe\fR removed.
.IP "\f(CW(succ \fIx y\fP)\fR"
.br
Returns the element that immediately follows the atom \fIx\fR
in the list \fIy\fR.  If \fIx\fR does not occur in \fIy\fR
or is the last element,  \fBnil\fR is returned.
.IP "\f(CW(pred \fIx y\fP)\fR"
.br
Returns the element that immediately precedes the atom \fIx\fR
in the list \fIy\fR.  If \fIx\fR does not occur in \fIy\fR
or is the first element,  \fBnil\fR is returned.
.IP "\f(CW(before \fIx y\fP)\fR"
.br
Returns the list of elements occurring before y in x.
If \fIy\fR does not occur in \fIx\fR
or is the first element,  \fBnil\fR is returned.
.IP "\f(CW(after \fIx y\fP)\fR"
.br
Returns the list of elements occurring after y in x.
If \fIy\fR does not occur in \fIx\fR
or is the last element,  \fBnil\fR is returned.
.IP "\f(CW(plist \fIx\fP)\fR"
.br
Returns the property list for the atom \fIx\fR.
.IP "\f(CW(get \fIx i\fP)\fR"
.br
Returns the value stored on \fIx\fR's property list
under the indicator \fIi\fR.
.IP "\f(CW(putprop \fIx v i\fP)\fR"
.br
Stores the value \fIv\fR on \fIx\fR's property list under
the indicator \fIi\fR.
.IP "\f(CW(remprop \fIx i\fP)\fR"
.br
Remove the indicator \fIi\fR
and any associated value from \fIx\fR's property list.
.IP "\f(CW(mapcar \fIf l\fP)\fR"
.br
Applies the function \fIf\fR to each element of \fIl\fR and returns
the list of results.
.IP "\f(CW(apply \fIf args\fP)\fR"
.br
Calls \fIf\fR with the arguments \fIargs\fR,  \fIe.g.\fR,
.ZB
(apply 'cons '(a (b)))
.ZE
is equivalent to
.ZB
(cons 'a '(b))
.ZE
.SH
Syntactic conventions.
.LP
Atoms take the following forms:
.IP "\fIRegular identifiers\fR"
.br
Atoms matching the regular expression
.br
.sp
.nf
    \f(CW[_A-Za-z][-A-Za-z_0-9]*\fR
.fi
.sp
The initial value of an identifier is \fBnil\fR.
.IP "\fIIntegers\fR"
.br
Atoms matching the regular expression \f(CW[0-9][0-9]*\fR.
Integers are constants,  \fIi.e.\fR,  evaluate to themselves.
.IP "\fIWeird atoms\fR"
.br
Identifiers matching the regular expression \f(CW".*"\fR.  Weird
atoms are not constants.
.LP
A semicolon introduces a comment,  which continues for the rest
of the line.
.SH
Usage.
.LP
The command for running the interpreter is
.ZB
walk [ \fIfiles\fP ]
.ZE
on BSD UNIX and derivative systems,  or
.ZB
awk -f walk [ \fIfiles\fP ]
.ZE
on UNIX System V.
The file name \f(CW\-\fR represents the standard input.
This can be omitted if no other files are being read in,  or
if the interpreter is being run non-interactively.
.PP
Normally,  the interpreter is used interactively,  augmented
with the functions defined in \fBwalk.w\fR,  and,  perhaps,  other files.
The command line to use for this purpose is
.ZB
walk walk.w [ \fIother files\fP ] p -
.ZE
The interpreter will first read \fBwalk.w\fR,  printing
the results of evaluating the function definitions
therein.  Then it will read \fBp\fR.  This file
contains no LISP definitions;  the interpreter recognizes
it by name and prints a prompt to signal the user that all
the prerequisite files have been read and that the interpreter
is waiting for input.  (This is the only way to get \fBawk\fR
to do this;  this can be hidden from the user
with a shell program that invokes the interpreter if desired.)
Thereafter,  it will evaluate expressions typed in by
the user,  printing a prompt after each one.
Normally the prompt is \f(CW\->\fR;  the first character of the
prompt changes when appropriate to an integer that represents the number
of unmatched left parentheses read in so far.
.PP
The interpreter exits when it encounters the end of its last input file.
If this file is the standard input,  the number of LISP objects
created is reported.
.PP
Several files defining auxiliary functions are provided.
.SH
Implementation.
.PP
So that it can run on any UNIX system,  The
LISP interpreter has been written using the UNIX V7 version
of \fBawk\fR,  which
predates the version described in \fIThe Awk Programming Language\fR [2].
The only complex data type provided by this language is the array.
Data that in C might be stored in structures is
represented,  therefore, using
multiple arrays,  one for each field.  For example,  the C code
.ZB
	p = allocate_cell();
	p->car = s;
	p->cdr = NIL;
.ZE
can be approximated with:
.ZB
	p = ++cell;
	car[p] = s;
	cdr[p] = nil;
.ZE
Lists (using nested array references)
and stacks are also simulated with arrays.  The most important data
structures are explained in the program and in the following description.
.PP
As is usual for LISP implementations,  the interpreter is constructed as a
loop that reads an s-expression,  evaluates it,  and prints the
result.  The reader collects an s-expression,  reading multiple
input lines if necessary.  Like the other two phases of the
interpreter,  this is a recursive procedure and
in \fBawk\fR this must be managed explicitly.
When an s-expression is read,  its internal representation in
list structure is formed using the stack \fBread[]\fR.  Atoms and
\fBcons\fR operators are pushed onto the read stack and periodically
`reduced' or replaced
with list cells when a complete list has been read;  the reader returns
an atom or list on the top of the stack.
The reader must
be able to return an s-expression in the middle of an input line,  so the
entire interpreter is enclosed in a loop that allows the
current input line to be completely scanned before the next
input record is read.  The general outline is:
.ZB
BEGIN {
.ft I
	initialize interpreter 
	say hello if interactive
.ft R
.ft CW
}

{
.ft I
	initialize reader variables
.ft R
.ft CW

	while (\fIchars left on this line\fR\f(CW)
	{
.ft I
		read
.ft R
.ft CW

		if (\fIhave read an s-expression\fR\f(CW)
		{
.ft I
			eval

			print
.ft R
.ft CW
		}
	}

.ft I
	prompt if interactive
.ft R
.ft CW
}

END {
.ft I
	say goodbye if interactive
	exit
.ft R
.ft CW
}
.ZE
.PP
The evaluator maintains two stacks,  one for input and one for output.
The result returned by the reader is copied onto the input stack
(\fBeval[]\fR),  and evaluated according to the usual LISP rules.
Evaluated s-expressions are placed on the output stack,  \fBarg[]\fR.
When an intrinsic function that takes evaluated arguments appears on
the top of the evaluation stack,  its arguments are popped from the
argument stack.  Functions (like \fBcond\fR) that take unevaluated
arguments are handled as special forms before their arguments have been
pushed onto \fBeval[]\fR.  The arguments are handled differently
depending on the
semantics of the function.  Lambda (user-defined) functions are evaluated
by temporarily binding the formal parameters in the function
definition to the results of evaluating the actual arguments with which
the function was called,  and then evaluating the body of the function.
Temporary bindings only are kept on a special
pushdown list (the \fIalist\fR).  Atoms have a global value that is
stored separately;  this keeps the alist small.
.LP
The evaluation procedure is sketched below:
.ZB
.ft I
atom?
.ft CW
	lambda
.ft R
		restore previous environment (lambda function
		body has been evaluated already)
.ft I
	constant?
.ft R
		return
.ft I
	bound?
.ft R
		look up local value
.ft I
	otherwise
.ft R
		return global value

.ft I
intrinsic function?
.ft R
	apply to already evaluated arguments

.ft I
lambda function?
.ft R
	bind formal parameters to already evaluated arguments
	evaluate function body

.ft I
form?
	intrinsic function application?
.ft R
.ft CW
		quote
.ft R
			return unevaluated argument
.ft CW
		cond
		and
		or
.ft R
			begin evaluating arguments according to operator semantics
.ft CW
		list
.ft R
			expand to repeated applications of cons
.ft I
	other?
.ft R
		push function variable,  arguments
	
.ft I
lambda function application?
.ft R
	push lambda function,  body
.ft I
other?
.ft R
	eval \fBcar\fR,  \fBcdr\fR
.ZE
.PP
When the evaluation stack is emptied,  the result
is popped from the argument stack and printed.  A stack is again
used to manage recursion.
.SH
Conclusion.
.PP
The goal of writing a small LISP interpreter and extending it
in LISP has been realized.  Though it was not my original intention,  it
would be easy to incorporate the
LISP functions as intrinsics,  and many other extensions (such as
numeric functions) could be made,  in which case the interpreter
might fulfill more than a pedagogic function.
Even so,  it can be used as is for an introduction to LISP programming
and implementation concepts.  I hope it also inspires more of us to
learn how to program in \fBawk\fR!
.SH
References.
.IP "[1]"
.br
McCarthy, J.  Recursive Functions of Symbolic Expressions
and their Computation by Machine,  Part
I.  \fIComm. ACM\fR,  3,  4,  pp. 185-195
April 1960
.IP "[2]"
.br
Aho, A.,  Weinberger,  P.,  & Kernighan,  B.W.  \fIThe
Awk Programming Language\fR.  Addison-Wesley,  Reading,  MA 1988
