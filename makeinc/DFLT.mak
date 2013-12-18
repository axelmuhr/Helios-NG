# HPROC = target processor type
HPROC = TRAN

# HHOST = host processor for make
HHOST = SUN4

# HSRC = pathname of system root directory
HSRC = /giga/HeliosRoot/Helios

# HPROD = destination for installed production binaries
HPROD = /giga/HeliosRoot/Production/$(HPROC)

# RSRC = optional remote equivalent of HSRC
#RSRC	= /hsrc

# Indicate that we are cross-compiling
DEFINES := -D__CROSSCOMP
