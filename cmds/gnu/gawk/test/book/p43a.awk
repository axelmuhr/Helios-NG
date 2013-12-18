{ print ($1 != 0 ? 1/$1 : "$1 is zero, line " NR) }
