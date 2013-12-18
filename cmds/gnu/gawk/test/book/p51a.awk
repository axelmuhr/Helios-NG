{ print max($1,max($2,$3)) }  # print maximum of $1, $2, $3

function max(m, n) {
    return m > n ? m : n
}
