/^doctor/ { p = 1; next }
p == 1
/^$/      { p = 0; next }
