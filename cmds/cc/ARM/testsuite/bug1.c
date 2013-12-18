/* 

B1)
A second bug (I can work around this one) has come to light with the latest
compiler. It is to do with the -Zr option and statics allocated within
functions. Given the code fragment:
*/

char fred[10];
static char fred2[10];

int main()
{
	static char stata[10];

	printf(fred);
	printf(fred2);
	printf(stata);
}

/*
The code output correctly references 'fred' and 'fred2' as '_fred' and
'_fred2'. However 'stata' is referenced as '.stata'. We expect it to be
referenced as '_stata'. Again this bug can be put on the back burner until the
next iteration.
*/

