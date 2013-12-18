# form4 - format countries data for tbl input

BEGIN  {
    FS = ":"; OFS = "\t"; date = "January 1, 1988"
    print ".TS\ncenter;"
    print "l c s s s r s\nl\nl l c s c s c\nl l c c c c c."
    printf("%s\t%s\t%s\n\n", "Report No. 3",
        "POPULATION, AREA, POPULATION DENSITY", date)
    print "CONTINENT", "COUNTRY", "POPULATION",
          "AREA", "POP. DEN."
    print "", "", "Millions", "Pct. of", "Thousands",
          "Pct. of", "People per"
    print "", "", "of People", "Total", "of Sq. Mi.",
          "Total", "Sq. Mi."
    print "\t\t_\t_\t_\t_\t_"
    print ".T&\nl l n n n n n."
}

{    if ($1 != prev) {  # new continent
        if (NR > 1)
            totalprint()
        prev = $1
        poptot = $8;  poppct = $4
        areatot = $9; areapct = $6
    } else {            # next entry for current continent
        $1 = ""
        poppct += $4; areapct += $6
    }
    printf("%s\t%s\t%d\t%.1f\t%d\t%.1f\t%.1f\n",
        $1, $2, $3, $4, $5, $6, $7)
    gpop += $3;  gpoppct += $4
    garea += $5; gareapct += $6
}

END {
    totalprint()
    print ".T&\nl s n n n n n."
    printf("GRAND TOTAL\t%d\t%.1f\t%d\t%.1f\n",
        gpop, gpoppct, garea, gareapct)
    print "", "=", "=", "=", "=", "="
    print ".TE"
}

function totalprint() {    # print totals for previous continent
    print ".T&\nl s n n n n n."
    print "", "_", "_", "_", "_", "_"
    printf("   TOTAL for %s\t%d\t%.1f\t%d\t%.1f\n",
        prev, poptot, poppct, areatot, areapct)
    print "", "=", "=", "=", "=", "="
    print ".T&\nl l n n n n n."
}
