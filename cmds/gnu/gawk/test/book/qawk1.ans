# qawk - awk relational database query processor

BEGIN { readrel("relfile") }
/./   { doquery($0) }

function readrel(f) {
    while (getline <f > 0)   # parse relfile
        if ($0 ~ /^[A-Za-z]+ *:/) {     # name:
            gsub(/[^A-Za-z]+/, "", $0)  # remove all but name
            relname[++nrel] = $0
        } else if ($0 ~ /^[ \t]*!/)     # !command...
            cmd[nrel, ++ncmd[nrel]] = substr($0,index($0,"!")+1)
        else if ($0 ~ /^[ \t]*[A-Za-z]+[ \t]*$/)  # attribute
            attr[nrel, $1] = ++nattr[nrel]
        else if ($0 !~ /^[ \t]*$/)      # not white space
            print "bad line in relfile:", $0
}
function doquery(s,   i,j,x) {
    for (i in qattr)  # clean up for next query
        delete qattr[i]
    query = s    # put $names in query into qattr, without $
    while (match(s, /\$[A-Za-z]+/)) {
        qattr[substr(s, RSTART+1, RLENGTH-1)] = 1
        s = substr(s, RSTART+RLENGTH+1)
    }
    for (i = 1; i <= nrel && !subset(qattr, attr, i); ) 
        i++
    if (i > nrel)     # didn't find a table with all attributes
        missing(qattr)
    else {            # table i contains attributes in query
        for (j in qattr)   # create awk program
            gsub("\\$" j, "$" attr[i,j], query)
        if (!exists[i] && ncmd[i] > 0) {
            for (j = 1; j <= ncmd[i]; j++)
                x = x cmd[i, j] "\n"
            print "executing\n" x  # for debugging
            if (system(x) != 0) { # create table i
                    print "command failed, query skipped\n", x
                    return
               }
            exists[i]++
        }
        awkcmd = sprintf("awk -F'\t' '%s' %s", query, relname[i])
        printf("query: %s\n", awkcmd)   # for debugging
        system(awkcmd)
    }
}
function subset(q, a, r,   i) {  # is q a subset of a[r]?
    for (i in q)
        if (!((r,i) in a))
            return 0
    return 1
}
function missing(x,     i) {
    print "no table contains all of the following attributes:"
    for (i in x)
        print i
}
