# Project:   linkpod


# Toolflags:
CCflags = -c -depend !Depend -throwback -IC:
Linkflags = -o $@
ObjAsmflags = -depend !Depend -ThrowBack -Stamp -quit -CloseExec
CMHGflags = 
LibFileflags = -c -o $@
Squeezeflags = -o $@
AAsmflags = -depend !Depend -ThrowBack -stamp -quit -CloseExec -To $@ -From


# Final targets:
@.lomon:   @.s.lomon 
        aasm $(aasmflags) @.s.lomon
@.execload:   @.o.loader @.o.copier 
        link $(linkflags) @.o.loader @.o.copier 


# User-editable dependencies:


# Static dependencies:
@.o.loader:   @.c.loader
        cc $(ccflags) -o @.o.loader @.c.loader 
@.o.copier:   @.s.copier
        objasm $(objasmflags) -from @.s.copier -to @.o.copier


# Dynamic dependencies:
