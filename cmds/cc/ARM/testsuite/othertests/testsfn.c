/* following static causes problems */

typedef void (* VoidFnPtr)();

static void callme(void);
void doit(VoidFnPtr vfp);

int main()
{
	doit(callme);
	printf("Going to callme?");
	callme();
}

void doit(VoidFnPtr vfp)
{
	vfp();
}

static void callme(void)
{
	printf("do something...\n");
}

