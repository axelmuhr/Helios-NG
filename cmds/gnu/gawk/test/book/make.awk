# make - maintain dependencies

BEGIN {
    while (getline <"makefile" > 0)
        if ($0 ~ /^[A-Za-z]/) {  #  $1: $2 $3 ...
            sub(/:/, "")
            if (++names[nm = $1] > 1)
                error(nm " is multiply defined")
            for (i = 2; i <= NF; i++) # remember targets
                slist[nm, ++scnt[nm]] = $i
        } else if ($0 ~ /^\t/)        # remember cmd for
            cmd[nm] = cmd[nm] $0 "\n" #   current name
        else if (NF > 0)
            error("illegal line in makefile: " $0)
    ages()      # compute initial ages
    if (ARGV[1] in names) {
        if (update(ARGV[1]) == 0)
            print ARGV[1] " is up to date"
    } else
        error(ARGV[1] " is not in makefile")
}

function ages(      f,n,t) {
    for (t = 1; ("ls -t" | getline f) > 0; t++)
        age[f] = t   # all existing files get an age
    close("ls -t")
    for (n in names)
        if (!(n in age))   # if n has not been created
            age[n] = 9999  # make n really old
}
function update(n,   changed,i,s) {
    if (!(n in age)) error(n " does not exist")
    if (!(n in names)) return 0
    changed = 0
    visited[n] = 1
    for (i = 1; i <= scnt[n]; i++) {
        if (visited[s = slist[n, i]] == 0) update(s)
        else if (visited[s] == 1)
            error(s " and " n " are circularly defined")
        if (age[s] <= age[n]) changed++
    }
    visited[n] = 2
    if (changed || scnt[n] == 0) {
        printf("%s", cmd[n])
        system(cmd[n])  # execute cmd associated with n
        ages()          # recompute all ages
        age[n] = 0      # make n very new
        return 1
    }
    return 0
}
function error(s) { print "error: " s; exit }
