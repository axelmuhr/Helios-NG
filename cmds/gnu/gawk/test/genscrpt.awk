# This is generating sripts for a shell running on Atari ST which
# inherited file structure from MessyDOS.  Modify to your needs
# and other computers

# This gawk program generates gulam script to generate TeX pk-fonts
# from Metafont programs.  Edit file fontdata.txt to your needs.
# Run as 'gawk -f genfonts.awk fontdata.txt > script.g'
#
# From a similar Bourne shell script which was posted 
# in comp.text.tex by Ken Yap (ken@cs.rochester.edu)
#
# Michal Jaegermann (ntomczak@ualtavm.bitnet, ntomczak@vm.ucs.ualberta.ca)
# November, 1990
#

# function to print script lines

function gencommands(fname, mag, mode, dpi,    res, subdir)
{
# Rm is my private alias which removes files without asking any questions

    res = int(dpi * exp(log(1.2) * mag) + 0.5)
    subdir = res "pk"
    printf "if { -e %s } == 0\n\tmkdir %s\nendif\n", subdir, subdir
    printf "mf '\\mode=%s; \\mag=magstep%s; \\batchmode; \\input %s'\n", mode, mag, fname
    if (0 != mag)
	printf "Rm %s.tfm\n", fname
    else
	printf "mv %s.tfm tfm\\\n", fname 
    printf "gftopk %s.%sgf %s\\%s.pk\n", fname, res, subdir, fname
    printf "Rm %s.%sgf\n", fname, res
    print "#"
}

# adjust to your needs - you may also set these variables on a command line
BEGIN {
    mode = "epson"
    dpi  = 240
    print "#"
    print "# gulam script to generate pk-fonts created by gawk"
    print "# it assumes that subdirectory .\\tfm already exists"
    print "#"
}
{
    if ("@" == $1 && 2 == NF && $2 == "fonts") {
	fnum = 0
	while (getline > 0 && "@" != $1) {
	    fnum++
	    fonts[fnum] = $1
	}
	mnum = 0
	while ($2 != "magstep") {
	    if (getline <= 0)
		break;
	}
	if ("magstep" == $2) {
	    getline
	    mnum = split ($0, mags)
	}
	if (mnum > 0) {
	    for (i = 1; i <= fnum; i++)
		for (j = 1; j <= mnum; j++)
		    gencommands(fonts[i],mags[j],mode,dpi)
	}
    }
}
