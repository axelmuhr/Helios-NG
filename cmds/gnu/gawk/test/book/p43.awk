BEGIN	{ FS = OFS = "\t" }
	{ $5 = 1000 * $3 / $2; print }
