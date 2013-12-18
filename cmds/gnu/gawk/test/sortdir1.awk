#
# A demo of using your own temporary files with gawk
# Use as in 'gawk -f sortdir1.awk > outs'
#
# !!! WILL CREATE (OR OVERWRITE) FILE ls.tmp  -- WATCH OUT !!!
#
BEGIN {
	cmd1 = "ls -l"
	cmd2 = "sort -n +3"
	tmpfile = "ls.tmp"
	while ((cmd1 | getline) > 0)
		print $0 > tmpfile
	close (cmd1)
	close (tmpfile)
	while ((getline < tmpfile) > 0)
		print $0 | cmd2
	close (cmd2)
	close (tmpfile)
}
