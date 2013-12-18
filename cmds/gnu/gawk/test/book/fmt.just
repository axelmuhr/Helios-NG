# fmt.just - formatter with right justification

BEGIN { blanks = sprintf("%60s", " ") }
/./   { for (i = 1; i <= NF; i++) addword($i) }
/^$/  { printline("no"); print "" }
END   { printline("no") }

function addword(w) {
    if (cnt + size + length(w) > 60)
        printline("yes")
    line[++cnt] = w
    size += length(w)
}

function printline(f,    i, nb, nsp, holes) {
    if (f == "no" || cnt == 1) {
        for (i = 1; i <= cnt; i++)
            printf("%s%s", line[i], i < cnt ? " " : "\n")
    } else if (cnt > 1) {
        dir = 1 - dir        # alternate side for extra blanks
        nb = 60 - size       # number of blanks needed
        holes = cnt - 1      # holes
        for (i = 1; holes > 0; i++) {
            nsp = int((nb-dir) / holes) + dir
            printf("%s%s", line[i], substr(blanks, 1, nsp))
            nb -= nsp
            holes--
        }
        print line[cnt]
    }
    size = cnt = 0
}
