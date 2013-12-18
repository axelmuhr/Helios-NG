#include <procname.h>
 
void test2 (char *CallDummy)
{
	IOdebug ("this is %s, called by %s", ThisFunc (CallDummy), CalledBy (CallDummy));
}

void test1 ()
{
	char *Dummy;
	
	test2 (Dummy);
}

int main ()
{
	test1 ();
}


