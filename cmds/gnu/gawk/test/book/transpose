# transpose - input and output suitable for graph
#   input:  data and specification of a graph
#   output: data and specification for the transposed graph

BEGIN {
    number = "^[-+]?([0-9]+[.]?[0-9]*|[.][0-9]+)" \
                            "([eE][-+]?[0-9]+)?$"
}
$1 == "bottom" && $2 == "ticks" {     # ticks for x-axis
    $1 = "left"
    print
    next
}
$1 == "left" && $2 == "ticks" {       # ticks for y-axis
    $1 = "bottom"
    print
    next
}
$1 == "range" {                       # xmin ymin xmax ymax
    print $1, $3, $2, $5, $4
    next
}
$1 == "height" { $1 = "width"; print; next }
$1 == "width"  { $1 = "height"; print; next }
$1 ~ number && $2 ~ number  { nd++; print $2, $1, $3; next }
$1 ~ number && $2 !~ number { # single number:
    nd++                      #   count data points
    print $1, nd, $2          #   fill in both x and y
    next
}
{ print }
