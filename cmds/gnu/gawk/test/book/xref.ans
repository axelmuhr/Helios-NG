/^\.#/ { printf("{ gsub(/%s/, \"%d\") }\n", $2, ++count[$1])
         if (saw[$2])
             print NR ": redefinition of", $2, "from line", saw[$2]
         saw[$2] = NR
       }
END    { printf("!/^[.]#/\n") }
