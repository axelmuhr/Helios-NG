#include "hepcdefs.h"
#include "hepcio.h"

main(argc, argv)
int argc; char **argv;
{
	int fd = open("/dev/hepc0", 0);

	if(fd < 0)
		printf("failed to open\n");
	if(ioctl(fd, HEPC_DEBUG, atoi(argv[1])) != 0)
		printf("failed to ioctl debug\n");
	if(ioctl(fd, HEPC_RESET) != 0)
		printf("failed to ioctl\n");
	if(ioctl(fd, HEPC_DEBUG, 2) != 0)
		printf("failed to ioctl 2\n");
	if(ioctl(fd, HEPC_DEBUG, 3) != 0)
		printf("failed to ioctl 3\n");
}
