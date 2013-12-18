/* ncc bug test prog */
/* peephole opti. causes problems here */

#include <helios.h>

void ifabsent(int fred);

struct keyentry { char *key; void (*fn)(); word arg1; } keytab[] =
{
	{ "ifabsent",	ifabsent,	1	},
	{ 0, 		0, 		0 	}
};

int main(void)
{
	struct keyentry *k = keytab;
	void (*f)() = k->fn;

	IOdebug("before fnptr call");
#ifdef BUG
	f(k->arg1);	/* bang */
#else
	f(1);		/* kosha */
#endif

	IOdebug("after fnptr call");
}

void ifabsent(int fred)
{
	IOdebug("Entered ifabsent");
}
