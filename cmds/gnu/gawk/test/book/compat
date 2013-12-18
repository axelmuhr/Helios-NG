# compat - check if awk program uses new built-in names

BEGIN { asplit("close system atan2 sin cos rand srand " \
               "match sub gsub", fcns)
        asplit("ARGC ARGV FNR RSTART RLENGTH SUBSEP", vars)
        asplit("do delete function return", keys)
      }

      { line = $0 }

/"/   { gsub(/"([^"]|\\")*"/, "", line) }     # remove strings,
/\//  { gsub(/\/([^\/]|\\\/)+\//, "", line) } # reg exprs,
/#/   { sub(/#.*/, "", line) }                # and comments

      { n = split(line, x, "[^A-Za-z0-9_]+")  # into words
        for (i = 1; i <= n; i++) {
            if (x[i] in fcns)	
                warn(x[i] " is now a built-in function")
            if (x[i] in vars)
                warn(x[i] " is now a built-in variable")
            if (x[i] in keys)
                warn(x[i] " is now a keyword")
        }
      }

function asplit(str, arr) {  # make an assoc array from str
    n = split(str, temp)
    for (i = 1; i <= n; i++)
        arr[temp[i]]++
    return n
}

function warn(s) {
    sub(/^[ \t]*/, "")
    printf("file %s, line %d: %s\n\t%s\n", FILENAME, FNR, s, $0)
}
