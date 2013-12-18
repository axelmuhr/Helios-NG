/* 

B4)
The problem with automatic array initialisation is as follows:

GIVEN:
*/

int main(char *trace, int null)
{
	int fred[] = {10,20,30,40};
	char harry[5] = {'a','b','c', '\n', 0};

#if 0
	char *c;

	c = harry;
	while((*trace++ = *c++) != 0);/*nullstat*/

	*trace++=0xff;
#endif
	return 1;
}

/*
YOU GET:

ncc -I.,/helios/include -DARM -c ncctest.c
Norcroft Helios ARM C vsn 3.00/331b [Jun  6 1989]
"ncctest.c", line 6: Warning: '&' unnecessary for function or array 'fred'
"ncctest.c", line 6: Warning: '&' unnecessary for function or array '<Anon1_at_line_6>'
"ncctest.c", line 7: Warning: '&' unnecessary for function or array 'harry'
"ncctest.c", line 7: Warning: '&' unnecessary for function or array '<Anon1_at_line_7>'
ncctest.c: 4 warnings, 0 errors, 0 serious errors

The initialisation looks ok doesn't it!? This problem also occurs under
unix C. Under unix the strings actually do get initialised, but under
Helios/ARM C they don't. Making fred and harry statics removes the warnings.
*/

/* now we get:
"bug4.c", line 20: Warning: variable 'fred' declared but not used
"bug4.c", line 20: Warning: variable 'null' declared but not used
"bug4.c", line 20: Fatal internal error:
		Unable to find another data symbol at 16

***************** FATAL ERROR ***********************
*/
