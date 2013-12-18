#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/textutil/RCS/pr.c,v 1.3 1994/05/12 13:39:57 nickc Exp $";
#endif

#include <stdio.h>
#include <time.h>

#define LINESIZE 132
#define USAGE "pr [<file list>]"

void do_ln(char *line);
void put_ln(char *line);
void form_feed(void);

FILE *input, *output;
int ln_cnt, page_no;
int tabsize = 4, lines_per_page = 60;
char *fname;
time_t timer;

int main(int argc, char *argv[])
{
	int i, returncode = 0;
	char line[LINESIZE];
	output = stdout;
	for(i=1; i<argc; ++i){
		if(argv[i][0] == '-'){
			switch(argv[i][1]){
				default:
					fprintf(stderr, USAGE);
					return(1);
			}
		}
		else
			break;
	}
	timer = time(NULL);	/* Get current time */

	if (i==argc) {		/* pr stdin */
		fname = "stdin";
		page_no = 1;
		while(fgets(line, LINESIZE, stdin) != NULL){
			do_ln(line);
		}
		form_feed();
		return 0;
	}

	/* Files given */
	for(; i<argc; ++i){
		input = fopen(argv[i], "r");
		if(input == NULL){
			fprintf(stderr, "Cannot open file %s\n", argv[i]);
			returncode = 1;
			continue;
		}
		fname = argv[i];
		page_no = 1;
		while(fgets(line, LINESIZE, input) != NULL){
			do_ln(line);
		}
		form_feed();
	}
	return returncode;
}

void do_ln(char *line)
{
	char buf[LINESIZE], *p = buf;
	int i;
	while(*line){
		if(*line == '\t'){
			for(i=0; i<tabsize; ++i)
				*p++ = ' ';
			++line;
		}
		else
			*p++ = *line++;
	}
	*p = '\0';
	put_ln(buf);
}

void put_ln(char *line)
{
	if(ln_cnt == 0)
	{	/* put header */
		fprintf(output, "\n%s %s Page %d\n\n", ctime(&timer), 
			fname, page_no);
		ln_cnt = lines_per_page;
	}
	fputs(line, output);
	--ln_cnt;
	if(ln_cnt == 0)
	{	/* put footer */
		fputs("\n\n\n", output);
		++page_no;
	}
}

void form_feed()
{
	while(ln_cnt)
		put_ln("\n");
}

