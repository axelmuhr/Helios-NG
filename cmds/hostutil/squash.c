

#include <stdio.h>

int main()
{
	int copying = 0;
	char line[100];
	while(gets(line) != NULL)
	{
		char *p = line;
		if( *p == '#') 
		{
			if( p[1] == '{' ) copying = 1;
			if( p[1] == '}' ) copying = 0;
			continue;
		}
		if( copying ) continue;
		while( *p == '\t' || *p == ' ' || *p == '\r' ) p++;
		if( *p == 0 ) continue;
		printf("%s\n",line);
	}

	return 0;
}
