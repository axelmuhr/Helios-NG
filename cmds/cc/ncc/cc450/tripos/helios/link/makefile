ncc = [ echo "Compiling %1"
   failat 1
   c:nc -ps1 -dM68K -dNOPOSIX -dNORCROFT -j,%,nc:include/ %1 %
]

sym.o      : sym.c      [<ncc>]
objed.o    : objed.c    [<ncc>]
module.o   : module.c   [<ncc>]
mem.o      : mem.c      [<ncc>]
scancode.o : scancode.c [<ncc>]
link.o     : link.c     [<ncc>]
readfile.o : readfile.c [<ncc>]
vm.o       : vm.c       [<ncc>]
genimage.o : genimage.c [<ncc>]

link : sym.o objed.o module.o mem.o scancode.o
       link.o readfile.o vm.o genimage.o
[  echo "Linking"
   blink with linklnk to link
]
