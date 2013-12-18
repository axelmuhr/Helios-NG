/* Quick hack to test input of non aligned data */
/* Quickly hacked from atst */
/* PAB 1/8/92 */

#include <helios.h>

#if 1			/* posix version */

#include <stdio.h>
#include <stdlib.h>
#include <posix.h>
#include <fcntl.h>

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

	int	fd = open("/helios/lib/nucleus", O_RDONLY);

	for (sz = 1 ;sz <= 20; sz++) {
		fprintf(stderr, "Read sz %d\n", sz);
		read(fd, &fred[0], sz);
#if 1
		read(fd, &fred[1], sz);
		read(fd, &fred[2], sz);
		read(fd, &fred[3], sz);
#endif
	}
}

#else			/* syslib version */

#include <stdio.h>
#include <stdlib.h>
#include <posix.h>
#include <fcntl.h>
#include <syslib.h>

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

	Object *ob = Locate(NULL, "/helios/lib/nucleus");
	Stream *fd = Open(ob, NULL, O_ReadOnly);

	for (sz = 1 ;sz <= 20; sz++) {
		fprintf(stderr, "Read sz %d\n", sz);
		Read(fd, &fred[0], sz, -1);
#if 1
		Read(fd, &fred[1], sz, -1);
		Read(fd, &fred[2], sz, -1);
		Read(fd, &fred[3], sz, -1);
#endif
	}
}
#endif


/* end of align test */
