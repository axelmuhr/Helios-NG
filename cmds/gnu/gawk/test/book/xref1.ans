/^\.#/ { s[$2] = ++count[$1]; next }
       { for (i in s)
             gsub(i, s[i])
         print
       }
