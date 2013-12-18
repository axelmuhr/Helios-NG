# build file for MG using Alcyon C and the GULAM shell
# execute this from the directory sys\atari 
#
#
# First make sure that there are stubs for the include files at top level 
# 
cd ..\..  
if { -e chrdef.h } == 0
    echo '#include "sys\atari\chrdef.h"' > chrdef.h
    endif
if { -e sysdef.h } == 0
    echo '#include "sys\atari\sysdef.h"' > sysdef.h
    endif
if { -e ttydef.h } == 0
    echo '#include "sys\atari\ttydef.h"' > ttydef.h
    endif
if { -e varargs.h } == 0
    echo '#include "sys\atari\varargs.h"' > varargs.h
    endif
#
#
# Recompile all the top-level files
#
cc basic
cc buffer
cc dir
cc dired
cc display
cc echo
cc extend
cc file
cc help
cc kbd
cc keymap
cc line
cc macro
cc main
cc match
cc modes
cc paragrap
cc random
cc re_searc
cc regex
cc region
cc search
cc version
cc window
cc word
#
#
# Now do all of the system-specific ones
#
cd sys\atari
cc alloc
cc cinfo
cc diredsup
cc fileio
cc misc
cc term
cc ttyio
as68 -l -u -s c: gemstart.s
as68 -l -u -s c: getn.s
#
#
# Now do the link
#
aln -c mglink.inp
