# fmt - format
#    input:  text
#    output: text formatted into lines of <= 60 characters

/./  { for (i = 1; i <= NF; i++) addword($i) }
/^$/ { printline(); print "" }
END  { printline() }

function addword(w) {
    if (length(line) + length(w) > 60)
        printline()
    line = line " " w
}

function printline() {
    if (length(line) > 0) {
        print substr(line, 2)   # removes leading blank
        line = ""
    }
}
