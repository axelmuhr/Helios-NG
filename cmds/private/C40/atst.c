/* quick hack to test output of non aligned data */
/* PAB 23/6/92 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int sz;
	char *fred = "abcdefghijklmnopqrstuvwxyz"
			"abcdefghijklmnopqrstuvwxyz"
			"abcdefghijklmnopqrstuvwxyz"
			"abcdefghijklmnopqrstuvwxyz"

			"abcdefghijklmnopqrstuvwxyz"
			"abcdefghijklmnopqrstuvwxyz"
			"abcdefghijklmnopqrstuvwxyz"
			"abcdefghijklmnopqrstuvwxyz"

			"abcdefghijklmnopqrstuvwxyz"
			"abcdefghijklmnopqrstuvwxyz"
			"abcdefghijklmnopqrstuvwxyz"
			"abcdefghijklmnopqrstuvwxyz";

		/*    012345678901234567890123456789 */

	for (sz = 1 ;sz <= 300; sz++) {
		write(1, &fred[0], sz);
		write(1, "\n", 1);
		write(1, &fred[1], sz);
		write(1, "\n", 1);
		write(1, &fred[2], sz);
		write(1, "\n", 1);
		write(1, &fred[3], sz);
		write(1, "\n", 1);
	}
}

/* end of align test */
