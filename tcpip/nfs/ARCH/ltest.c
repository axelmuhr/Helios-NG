
#include <syslib.h>


int main()
{
	Object *o;
	
	o = Locate(NULL,"/orion/nick/helios");
IOdebug("%O",o);

	o = Locate(o,"posix");
IOdebug("%O",o);	

	o = Locate(o,"../queue.h");
IOdebug("%O",o);	
}
