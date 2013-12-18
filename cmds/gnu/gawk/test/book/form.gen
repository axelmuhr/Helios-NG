# form.gen - generate form letters
#   input:  prototype file letter.text; data lines
#   output: one form letter per data line

BEGIN {
    FS = ":"
    while (getline <"letter.text" > 0) # read form letter
        form[++n] = $0
}

{   for (i = 1; i <= n; i++) { # read data lines
        temp = form[i]         # each line generates a letter
        for (j = 1; j <= NF; j++)
            gsub("#" j, $j, temp)
        print temp
    }
}
