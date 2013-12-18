BEGIN { free = -1; Blksize=512; Mbyte=1048576; CONST = Blksize / Mbyte; FS=")" }
{
  if (free == -1) {	# free is toggled every other line.
	split($1,fsptr,"("); FSYS=fsptr[1]
	split($2,freeptr," "); free=freeptr[2]+0
	FS=":";
	if( free == 0 && substr(freeptr[2],1,1) != "0" ) {
		free = -1; next
	}
	next
  }
  split($2,allocptr," "); alloc = allocptr[1]+0
  FS=")";
  if (alloc == 0) next;		# avoid division by zero
  TFREE= (free * CONST) - .005			# force rounding down.
  TALLOC= (alloc * CONST) - .005		# force rounding down.
  PCT=free * 100 / alloc
  if (TFREE < 0) TFREE=0
  if (TALLOC < 0) TALLOC=0
  if (length(FSYS) < 17)
    {
     printf ("%#16s:\tDisk space: %#6.2f MB of %#6.2f MB available (%#5.2f%%).\n", FSYS, TFREE, TALLOC, PCT)
    }
  else
    {
     printf ("%s\n", FSYS)
     printf ("\t\t:\tDisk space: %#6.2f MB of %#6.2f MB available (%#5.2f%%).\n", TFREE, TALLOC, PCT)
    }
  Cumfree += free; Cumalloc += alloc;
  free = -1	# reset flag/variable for next set of two lines
}
END	{
	CumPct=Cumfree * 100 / Cumalloc
	Cumfree= (Cumfree * CONST) - .005	# force rounding down.

	Cumalloc= (Cumalloc * CONST) - .005	# force rounding down.
	printf ("\n\t\t  Total Disk Space: %#6.2f MB of %#6.2f MB available (%#5.2f%%).\n", Cumfree, Cumalloc, CumPct)
}
