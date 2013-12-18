awk '{ printf("%6.2f  %s\n", $2 * $3, $0) }' emp.data | sort
