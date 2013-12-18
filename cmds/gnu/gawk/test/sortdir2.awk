#
# use as in 'gawk -f sortdir2.awk > outs'
#
BEGIN {
	cmd1 = "ls -l"
	cmd2 = "sort -n +3"
	while ((cmd1 | getline) > 0)
		print $0 | cmd2
	close (cmd1)
	close (cmd2)
}
