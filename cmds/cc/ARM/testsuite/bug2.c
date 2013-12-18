/*

B2)
I have found another problem with the compiler, its one that I can work around,
so no immediate action is required.

The problem is to do with extern declarations within function bodies. - They
dont work. I have not looked into this problem to deeply, as I have other
things to do at the moment, briefly:
*/
#include <attrib.h>

int main()
{
int x;

extern Attributes ostate;

x = ostate.Time;

return x;
}

/*
generates:

 patch arm data transfer instruction (0x5997000) with datamodule of ostate
 patch arm data transfer instruction (0x5977000) with datasymb of ostate

rather than:

 patch arm data transfer instruction (0x5997000) with datamodule of ostate
 patch arm data processing instruction (0x2877000) with lsb datasymb of ostate
 patch arm data processing instruction (0x2877000) with rest datasymb of ostate
*/

