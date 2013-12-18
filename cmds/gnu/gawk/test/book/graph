# graph - processor for a graph-drawing language
#   input:  data and specification of a graph
#   output: data plotted in specified area

BEGIN {                # set frame dimensions...
    ht = 24; wid = 80  # height and width
    ox = 6; oy = 2     # offset for x and y axes
    number = "^[-+]?([0-9]+[.]?[0-9]*|[.][0-9]+)" \
                            "([eE][-+]?[0-9]+)?$"
}
$1 == "label" {                       # for bottom
    sub(/^ *label */, "")
    botlab = $0
    next
}
$1 == "bottom" && $2 == "ticks" {     # ticks for x-axis
    for (i = 3; i <= NF; i++) bticks[++nb] = $i
    next
}
$1 == "left" && $2 == "ticks" {       # ticks for y-axis
    for (i = 3; i <= NF; i++) lticks[++nl] = $i
    next
}
$1 == "range" {                       # xmin ymin xmax ymax
    xmin = $2; ymin = $3; xmax = $4; ymax = $5
    next
}
$1 == "height" { ht = $2; next }
$1 == "width"  { wid = $2; next }
$1 ~ number && $2 ~ number {          # pair of numbers
    nd++    # count number of data points
    x[nd] = $1; y[nd] = $2
    ch[nd] = $3    # optional plotting character
    next
}
$1 ~ number && $2 !~ number {         # single number
    nd++    # count number of data points
    x[nd] = nd; y[nd] = $1; ch[nd] = $2
    next
}
END {    # draw graph
    if (xmin == "") {         # no range was given
        xmin = xmax = x[1]    # so compute it
        ymin = ymax = y[1]
        for (i = 2; i <= nd; i++) {
            if (x[i] < xmin) xmin = x[i]
            if (x[i] > xmax) xmax = x[i]
            if (y[i] < ymin) ymin = y[i]
            if (y[i] > ymax) ymax = y[i]
        }
    }
    frame(); ticks(); label(); data(); draw()
}
function frame() {        # create frame for graph
    for (i = ox; i < wid; i++) plot(i, oy, "-")     # bottom
    for (i = ox; i < wid; i++) plot(i, ht-1, "-")   # top
    for (i = oy; i < ht; i++) plot(ox, i, "|")      # left
    for (i = oy; i < ht; i++) plot(wid-1, i, "|")   # right
}
function ticks(    i) {   # create tick marks for both axes
    for (i = 1; i <= nb; i++) {
        plot(xscale(bticks[i]), oy, "|")
        splot(xscale(bticks[i])-1, 1, bticks[i])
    }
    for (i = 1; i <= nl; i++) {
        plot(ox, yscale(lticks[i]), "-")
        splot(0, yscale(lticks[i]), lticks[i])
    }
}
function label() {        # center label under x-axis
    splot(int((wid + ox - length(botlab))/2), 0, botlab)
}
function data(    i) {    # create data points
    for (i = 1; i <= nd; i++)
        plot(xscale(x[i]),yscale(y[i]),ch[i]=="" ? "*" : ch[i])
}
function draw(    i, j) { # print graph from array
    for (i = ht-1; i >= 0; i--) {
        for (j = 0; j < wid; j++)
            printf((j,i) in array ? array[j,i] : " ")
        printf("\n")
    }
}
function xscale(x) {      # scale x-value
    return int((x-xmin)/(xmax-xmin) * (wid-1-ox) + ox + 0.5)
}
function yscale(y) {      # scale y-value
    return int((y-ymin)/(ymax-ymin) * (ht-1-oy) + oy + 0.5)
}
function plot(x, y, c) {  # put character c in array
    array[x,y] = c
}
function splot(x, y, s,    i, n) { # put string s in array
    n = length(s)
    for (i = 0; i < n; i++)
        array[x+i, y] = substr(s, i+1, 1)
}
