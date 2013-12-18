/^[+-]?[0-9][0-9]?[0-9]?(,[0-9][0-9][0-9])*$/ {
        gsub(/,/, "")
        sum += $0
        next
}
      { print "bad format:", $0 }
END   { print sum }
