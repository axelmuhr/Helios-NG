
#rel = "sampldrv 6.01b"

odir = obj\\

ldir = lst\\

inc = ..\include\\

diagdir = ..\diag\\

asmdir = ..\asmtools\\

#infodir = ..\getinfo\\

infodir = ..\sampldrv\\

compiler = cl
assembler = masm
cbasic = /AC /DSAMPLDRV /Zl /c /Fo$(odir) /I$(inc) /I$(infodir) /I.

#stackflags = /Gs
#debug = /Ddebug
#files = /Fc$(ldir)
#compflags = $(cbasic) $(debug) $(files) /Zi /Zd /Od
compflags = $(cbasic) $(debug) $(files) /Ox

asmflags = /DSMALL /DEXETYPE /I$(inc) /I.

defhdr = sampldrv.h

default : sampldrv.exe

$(odir)sampldrv.obj:	sampldrv.c
	$(compiler) $(compflags) sampldrv.c

$(odir)intrface.obj:	intrface.c
	$(compiler) $(compflags) intrface.c

$(odir)sdrvvars.obj:	sdrvvars.c
	$(compiler) $(compflags) sdrvvars.c

$(odir)dm_tools.obj:	$(diagdir)dm_tools.c
	$(compiler) $(compflags) $(diagdir)dm_tools.c

$(odir)sr_tools.obj:	$(diagdir)sr_tools.c
	$(compiler) $(compflags) $(diagdir)sr_tools.c

$(odir)sr_comn.obj:	$(diagdir)sr_comn.c
	$(compiler) $(compflags) $(diagdir)sr_comn.c

$(odir)nstk_chk.obj:	$(diagdir)nstk_chk.c
	$(compiler) $(compflags) $(stackflags) $(diagdir)nstk_chk.c

$(odir)nstk_com.obj:	$(diagdir)nstk_com.c
	$(compiler) $(compflags) $(stackflags) $(diagdir)nstk_com.c

$(odir)nstkdiag.obj:	$(diagdir)nstkdiag.c
	$(compiler) $(compflags) $(stackflags) $(diagdir)nstkdiag.c

$(odir)common.obj:	$(diagdir)common.c
	$(compiler) $(compflags) $(diagdir)common.c

$(odir)cvars.obj:	$(diagdir)cvars.c
	$(compiler) $(compflags) $(diagdir)cvars.c

$(odir)eth_data.obj:	$(diagdir)eth_data.c
	$(compiler) $(compflags) $(diagdir)eth_data.c

$(odir)params.obj:	$(diagdir)params.c
	$(compiler) $(compflags) $(diagdir)params.c

$(odir)spec8013.obj:	$(diagdir)spec8013.c
	$(compiler) $(compflags) $(stackflags) $(diagdir)spec8013.c

$(odir)batch.obj:	$(diagdir)batch.c
	$(compiler) $(compflags) $(diagdir)batch.c

$(odir)setup.obj:	$(diagdir)setup.c
	$(compiler) $(compflags) $(diagdir)setup.c

    #assembler stuff
$(odir)diagmove.obj: $(asmdir)diagmove.asm
	$(assembler) $(asmflags) $(asmdir)diagmove.asm $(odir)diagmove.obj;

$(odir)fast8013.obj: $(asmdir)fast8013.asm
	$(assembler) $(asmflags) $(asmdir)fast8013.asm $(odir)fast8013.obj;

$(odir)cgetcnfg.obj: $(infodir)cgetcnfg.asm
	$(assembler) $(asmflags) $(infodir)cgetcnfg.asm $(odir)cgetcnfg.obj;

$(odir)inp_outp.obj: $(asmdir)inp_outp.asm
	$(assembler) $(asmflags) $(asmdir)inp_outp.asm $(odir)inp_outp.obj;

$(odir)int_help.obj: $(asmdir)int_help.asm
	$(assembler) $(asmflags) $(asmdir)int_help.asm $(odir)int_help.obj;

$(odir)real_int.obj: $(asmdir)real_int.asm
	$(assembler) $(asmflags) $(asmdir)real_int.asm $(odir)real_int.obj;

$(odir)pc_ints.obj: $(asmdir)pc_ints.asm
	$(assembler) $(asmflags) $(asmdir)pc_ints.asm $(odir)pc_ints.obj;

$(odir)micro_ch.obj: $(asmdir)micro_ch.asm
	$(assembler) $(asmflags) $(asmdir)micro_ch.asm $(odir)micro_ch.obj;

$(odir)sysinfo.obj: $(asmdir)sysinfo.asm
	$(assembler) $(asmflags) $(asmdir)sysinfo.asm $(odir)sysinfo.obj;

# main dependency stmnt
sampldrv.exe: \
$(odir)sampldrv.obj $(odir)intrface.obj \
$(odir)sdrvvars.obj $(odir)sr_comn.obj \
$(odir)dm_tools.obj $(odir)sr_tools.obj \
$(odir)nstk_chk.obj $(odir)nstk_com.obj $(odir)nstkdiag.obj \
$(odir)common.obj $(odir)cvars.obj $(odir)eth_data.obj \
$(odir)batch.obj $(odir)setup.obj \
$(odir)params.obj $(odir)spec8013.obj \
$(odir)diagmove.obj $(odir)fast8013.obj $(odir)inp_outp.obj \
$(odir)cgetcnfg.obj \
$(odir)int_help.obj $(odir)real_int.obj $(odir)pc_ints.obj \
$(odir)micro_ch.obj $(odir)sysinfo.obj sampldrv.lnk
	link @sampldrv.lnk
