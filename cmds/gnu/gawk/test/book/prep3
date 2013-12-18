# prep3 - prepare countries data for form3

BEGIN  { FS = "\t" }
pass == 1 {
    area[$4] += $2
    areatot += $2
    pop[$4] += $3
    poptot += $3
}
pass == 2 {
    den = 1000*$3/$2
    printf("%s:%s:%s:%f:%d:%f:%f:%d:%d\n",
        $4, $1, $3, 100*$3/poptot, $2, 100*$2/areatot,
        den, pop[$4], area[$4]) | "sort -t: +0 -1 +6rn"
}
