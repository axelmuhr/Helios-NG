/*

B5)
The following declarations:

*/

static double T, E1[5];
int J;

int main(int argc, char **argv)
{
return 0;
}

/*
cause the compiler to crash:

Norcroft Helios ARM C vsn 3.00/332 [Aug 16 1989]
"#whet.c", line 4: Fatal internal error: Data seg generation confused

If, however, you declare the int before the double, all is well.
*/

