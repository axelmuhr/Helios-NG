#include "gexec.h"

void fred(void)
{
	ExecRoot *xroot = ExecRoot();

	xroot->Timer += 1000;

	xroot->SliceTime--;
}

