BEGIN {
    expects["aa"] = "bb"
    expects["cc"] = "dd"
    expects["ee"] = "ff"
}
/^(aa|cc|ee)/ {
    if (p != "")
        print "line", NR, ": expected " p
    p = expects[substr($0, 1, 2)]
}
/^(bb|dd|ff)/ {
    x = substr($0, 1, 2)
    if (p != x) {
        print "line", NR, ": saw " x
        if (p)
            print ", expected", p
    }
    p = ""
}
END {
    if (p != "")
        print "at end, missing", p
}
