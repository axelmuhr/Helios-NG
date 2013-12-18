# ix.genkey - generate sort key to force ordering
#   input:  string tab num num ...
#   output: sort key tab string tab num num ...

BEGIN { FS = OFS = "\t" }

{   gsub(/~/, " ", $1)       # tildes now become blanks
    key = $1
    # remove troff size and font change commands from key
    gsub(/\\f.|\\f\(..|\\s[-+][0-9]/, "", key)
    # keep blanks, letters, digits only
    gsub(/[^a-zA-Z0-9 ]+/, "", key)
    if (key ~ /^[^a-zA-Z]/)  # force nonalpha to sort first
        key = " " key        # by prefixing a blank
    print key, $1, $2
} 
