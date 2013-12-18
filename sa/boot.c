
#include <sysinfo.h>
#include <chanio.h>

void boot(int link)
{
	word *image = (word *)SysBase;
	byte *boot = (byte *)RTOA(image[3]);
	word imagesize = image[0];
	word bootsize = (byte *)RTOA(image[0]) - boot;

	link_out_byte(link,bootsize);
	link_out_data(link,boot,bootsize);

	tin_(ldtimer_()+20);
	
	link_out_byte(link,4);
	link_out_data(link,image,imagesize);
}

