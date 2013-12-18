function daynum(y, m, d,    days, i, n) {   # 1 == Jan 1, 1901
    split("31 28 31 30 31 30 31 31 30 31 30 31", days)
    # 365 days a year, plus one for each leap year
    n = (y-1901) * 365 + int((y-1901)/4)
    if (y % 4 == 0) # leap year from 1901 to 2099
        days[2]++
    for (i = 1; i < m; i++)
        n += days[i]
    return n + d
}
    { print daynum($1, $2, $3) }
