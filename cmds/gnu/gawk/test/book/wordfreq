# wordfreq - print number of occurrences of each word
#   input:  text
#   output: number-word pairs sorted by number

    { gsub(/[.,:;!?(){}]/, "")    # remove punctuation
      for (i = 1; i <= NF; i++)
          count[$i]++
    }
END { for (w in count)
          print count[w], w | "sort -rn"
    }
