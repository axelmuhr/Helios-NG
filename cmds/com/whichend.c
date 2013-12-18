#include <stdio.h>

int main(int argc, char **argv)
{
	union {
		long i;
		char c[4];
	} sextest;

	sextest.i = 0x03020100;

	printf("This machine is ");

	if (sextest.c[0] == 0x03)
		printf("big endian / bytesex odd / Most Significant Byte First\n");
	else
		printf("little endian / bytesex even / Least Significant Byte First\n");

	printf("\nunion{int i; char c[4]} u;\n\n"
		"u.i = 0x03020100;\n" \
		"/* is equivalent to: */\n" \
		"u.c[0] = %#x;\nu.c[1] = %#x;\nu.c[2] = %#x;\nu.c[3] = %#x;\n", \
		sextest.c[0], sextest.c[1], sextest.c[2], sextest.c[3] \
	);
	return 0;
}

