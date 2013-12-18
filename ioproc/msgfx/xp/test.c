#include <stdio.h>
#include "graph.h"

int main()
{	int x;
	x = _setvideomode(_TEXTBW40  );
	printf("_TEXTBW40 = %d\n",x);
	x = _setvideomode(_TEXTC40   );
	printf("_TEXTC40 = %d\n",x);
	x = _setvideomode(_TEXTBW80  );
	printf("_TEXTBW80 = %d\n",x);
	x = _setvideomode(_TEXTC80   );
	printf("_TEXTC80 = %d\n",x);
	x = _setvideomode(_MRES4COLOR  );
	printf("_MRES4COLOR = %d\n",x);
	x = _setvideomode(_MRESNOCOLOR );
	printf("_MRESNOCOLOR = %d\n",x);
	x = _setvideomode(_HRESBW      );
	printf("_HRESBW = %d\n",x);
	x = _setvideomode(_TEXTMONO    );
	printf("_TEXTMONO = %d\n",x);
	x = _setvideomode(_HERCMONO    );
	printf("_HERCMONO = %d\n",x);
	x = _setvideomode(_MRES16COLOR );
	printf("_MRES16COLOR = %d\n",x);
	x = _setvideomode(_HRES16COLOR );
	printf("_HRES16COLOR = %d\n",x);
	x = _setvideomode(_ERESNOCOLOR );
	printf("_ERESNOCOLOR = %d\n",x);
	x = _setvideomode(_ERESCOLOR   );
	printf("_ERESCOLOR = %d\n",x);
	x = _setvideomode(_VRES2COLOR  );
	printf("_VRES2COLOR = %d\n",x);
	x = _setvideomode(_VRES16COLOR );
	printf("_VRES16COLOR = %d\n",x);
	x = _setvideomode(_MRES256COLOR);
	printf("_MRES256COLOR = %d\n",x);
}
