
#define stdio_flag 1

#define INCLUDE(x) #ifndef x_flag \
#include <x.h> \
#endif

INCLUDE (stdio)

extern fred();

static fred(a,b,c)
int a,b,c;
{
   a=b+c;
   c = getchar();
}
