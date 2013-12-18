# ix.format - remove key, restore size and font commands
#   input:  sort key tab string tab num num ...
#   output: troff format, ready to print

BEGIN { FS = "\t" }

{   gsub(/ /, ", ", $3)        # commas between page numbers
    gsub(/\[/, "\\f(CW", $2)   # set constant-width font
    gsub(/\]/, "\\fP", $2)     # restore previous font
    print ".XX"                # user-definable command
    printf("%s  %s\n", $2, $3) # actual index entry
}
