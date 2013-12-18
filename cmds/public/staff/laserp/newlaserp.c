/* "laserp" version 2.0 by Chris Joyce (orig. 28/7/89) */

/*
 *
 * laserp laserp laserp laserp laserp laserp laserp laserp laserp laserp laserp
 * laserp laserp laserp laserp laserp laserp laserp laserp laserp laserp laserp
 * laserp laserp laserp laserp laserp laserp laserp laserp laserp laserp laserp
 * laserp laserp laserp laserp                      laserp laserp laserp laserp
 * laserp laserp laserp laserp  Into The Night...   laserp laserp laserp laserp
 * laserp laserp laserp laserp                      laserp laserp laserp laserp
 * laserp laserp laserp laserp laserp laserp laserp laserp laserp laserp laserp
 * laserp laserp laserp laserp laserp laserp laserp laserp laserp laserp laserp
 * laserp laserp laserp laserp laserp laserp laserp laserp laserp laserp laserp
 *
 */


#include <stdio.h>
#include <sys/time.h>

#define 	V_PAGE_LGTH      93 	
#define 	H_PAGE_LGTH	 66
#define		LNS_PER_IN	  8
#define		TOP_MGN		  2
#define		LEFT_MGN	  1 

#define 	V_TEXT_LGTH	 84
#define		H_TEXT_LGTH	 57 

#define		COLUMN_WIDTH	 86
#define		V_PAGE_WIDTH	125
#define		H_PAGE_WIDTH	183

#define		COLUMN_SPACE	 10

#define		LHS		  1
#define		RHS		  2

#define 	TAB		  8
#define 	TAB_SIZE(x)	(TAB-((x)%TAB))


char Page[V_TEXT_LGTH][H_PAGE_WIDTH];

int linenumber;
int pagenumber;
int pagewidth;
int textlength;

char *filename;

int state = 0;

int opt_2;     /* for two columns, sideways... */
int opt_l;     /* for no line numbers...       */
int opt_w;     /* for wide, sideways..         */
int opt_t;     /* for no title...              */



main(argc,argv)
int argc;
char *argv[];
{
	int k=1;
	char *cp;
	FILE *fp;


	cp = argv[1];
        while( *cp++ == '-' ){ 
                while ( *cp != NULL ) {			switch (*cp) {
				case 't':	opt_t++;	break;
				case 'l':	opt_l++;	break;
				case 'w':	opt_w++;	break;
				case '2':	opt_2++;	break;
				default:	usage();
			}
			cp++;	
		}
		k++;
		cp = argv[k];
	}	
	
	if (k == argc || (opt_2 && opt_w))  /* maybe user needs help... */

	             usage(); 


	while ( k < argc ){
		if ( ( fp = fopen(argv[k],"r") ) == NULL) {
			fprintf(stderr, "Can't open file: %s\n", argv[k]);
			exit(1);
		}
		else {	
			filename = argv[k];

			init_printer();
			init_page();
			print(fp);

			fclose(fp);
		}
		k++;
	}
}

init_printer()
{

  	/*   reset    */
	printf("\033E");		

	if (opt_2 || opt_w) {

                /* landscape orientation */
		printf("\033&l1O");  		  

                /* landscape page length */
		printf("\033&l%dP", H_PAGE_LGTH);  

                /* landscape font */
		printf("\033(s16.66H");            

	}

	else {

		/* portrait  page length */
		printf("\033&l%dP", V_PAGE_LGTH);  

		/* font selection */
		printf("\033(s16.66H");

	}


	/* left margin */ 
        printf("\033&a%dL", LEFT_MGN); 	
	 

	/* lines per inch */
	printf("\033&l%dD", LNS_PER_IN);

	/* top margin */
	printf("\033&l%dE", TOP_MGN);
	
	/* lines of text per page */
	if(opt_2) {
		printf("\033&l%dF", H_TEXT_LGTH+3);  /* add 3 for title */
	}
	else{
		printf("\033&l%dF", V_TEXT_LGTH+3);
	}

	return(1);
}

init_page()
{
	pagenumber = 1;
	linenumber = 1;
	state = 0;

	if (opt_2 || opt_w) {

               /* landscape (horizontal) */

		textlength = H_TEXT_LGTH;
		if (opt_2) pagewidth = COLUMN_WIDTH;
		else       pagewidth = H_PAGE_WIDTH;
	}

	else {

               /*   portrait (vertical)   */
		
		textlength = V_TEXT_LGTH;
		pagewidth =  V_PAGE_WIDTH;
	}

	return(1);
}

print(file)
FILE *file;
{

	while (state != EOF) {
		
		state = fill_page(file, LHS);

		if (opt_2 && state != EOF) {

			fill_page(file, RHS);
		}
		state = print_page();
	}

	return(EOF);
}
	

fill_page(file, page_side)
FILE *file;
int page_side;
{
	static int overflow = 0;

	int i = 0;
	int spaces = 0;
	int line = 0, col = 0;
	int c=0;
	char lineno[7];

	for (line = 0; line < textlength; line++) {

	   if (opt_2) {

  	   	if (page_side == RHS) {
		   	col = COLUMN_WIDTH;
		   	pagewidth = H_PAGE_WIDTH;
		   	for (i=0;i<COLUMN_SPACE;i++) Page[line][col++] = ' ';
	   	}
		else if (page_side == LHS) {
			pagewidth = COLUMN_WIDTH;
		}
	   }

	   	
		
	   if (!opt_l && !overflow) {

			   if (linenumber > 9999) linenumber = 1;

			   sprintf(lineno, "%4d  ", linenumber++);
	   }

	   else if (overflow)    sprintf(lineno, "   >  ");
	   else                  sprintf(lineno, "      ");
		 
	   for (i=0;i<6;i++) {
	      		Page[line][col++] = lineno[i]; 
  	   }

	   while (col < pagewidth) {
			
		if ((c = getc(file)) == EOF) {
			
			Page[line][col++] = '\0';
			overflow = 0;
		        return(-1);
		}
		else if (c == '\n') {

		 	while (col < pagewidth) {
		 		Page[line][col++] = ' ';
		 	}
			break;
		}
		else if (c == '\t') {
                                                                      
			 if (page_side == RHS) spaces = TAB_SIZE(col-((12+COLUMN_SPACE)%TAB));
			 else                  spaces = TAB_SIZE(col-6);

			 for(i=0; i<spaces; i++) {			
			 	Page[line][col++] = ' ';
			 	if (col >= pagewidth) break;
		         }		
		}
			 
		else if (c == '\f') {
			Page[line][col++] = c;
			while (getc(file) != '\n');
			return(1);
		}
		else if (c == '\b') {

			i = COLUMN_WIDTH + COLUMN_SPACE + 6;

			if (col > 6 && !(opt_2 && (col == i)) ) {
				Page[line][col++] = c;
			}
		}
		else if (c >= ' ') {   /* leave out unwanted control chars */

			Page[line][col++] = c;

	        }
	    }
   	    if (c == '\n') overflow = 0;
	    else {
		  if ((c = getc(file)) != '\n') { 
			overflow = 1;
	   		ungetc(c, file);
		  }
	    }

	    col = 0;
	   
	}
	return(1);

}


print_page()
{

	int line,col;
	int start = 0;
	int width = pagewidth;
	int c = 0;
	int result = 1;
	int backspaces = 0;

	print_title_line();

	for (line=0; line < textlength; line++) {
		
		for (col = 0; col < width; col++) {
			
				
		        if (start == width) {
				printf("\r\n\f");
				return(1);
			}
			else while (col < start) { putchar(' '); col++; }

			if ((c = Page[line][col]) == '\0') {
				if (state == EOF || start != 0) return(EOF);
				else {
					result = EOF; 
					width  = COLUMN_WIDTH;
					break;
				}
			}
			else if (c == '\b') {
				putchar(c);
				backspaces++;
			}
			else if (c == '\f') {

				if (opt_2) {

				   putchar(' ');
		  	           if (col < COLUMN_WIDTH) {
					  start = COLUMN_WIDTH; 
				   }
				   else  {
					  width = COLUMN_WIDTH;
					  break;
				   }
			 	}
				else {
					printf("\r\n");
					putchar(c);
					return(1);
				}
			}		
			else putchar(c);
		}
		while (backspaces != 0) { putchar(' '); backspaces--;};

		if (result != EOF || line != textlength - 1) printf("\r\n");
	
	}
	
	return(result);
	
}

print_title_line()
{	
	if (!opt_t) {
		print_title(pagenumber++);
	
		if (opt_2 && state != EOF) {

			printf("%6s","");
			print_title(pagenumber++);
		}
	}

	printf("\r\n\r\n\r\n");

	return(1);
}

print_title(pageno)
int pageno;
{	
	int i,spaces,width;
	char *timenow();

/* format */

	if (opt_2) {
		spaces =  2;
		width = COLUMN_WIDTH - 3;	
	}
	else if (opt_w) {
		spaces = 50;
		width = pagewidth - 4;
	}
	else {
		spaces = 19;
		width = pagewidth - 4;
	}

/* now to print the title */

	printf("File:  ");
	
	  if ( strlen(filename) > 28 ) *(filename + 28) = '\0';

	  printf("%-30s", filename);

	  for (i=0; i<spaces; i++) putchar(' ');

	printf("%5s%3d", "Page ", pageno);

	  while ((30 + 5 + 3 + 24 + (spaces++)) < width) putchar(' ');	

	printf("%-24s", timenow());

}


char *timenow()
{
	char *the_time, *index;
	struct timeval *tp;

	tp = ((struct timeval*)malloc(sizeof(struct timeval)));

	gettimeofday(tp,0);

	the_time = ctime(&(tp->tv_sec));	

	index = the_time;
	index += 24;
	*index = '\0';	

	return(the_time);
}

usage()
{
	fprintf(stderr, "Usage: laserp [-2] [-l] [-t] [-w] file1 [file2] ...\n");

	fprintf(stderr, "-2 for two columns, sideways\n");
	fprintf(stderr, "-l for no line numbers\n");
	fprintf(stderr, "-t for no title\n");
	fprintf(stderr, "-w for wide, sideways\n");


	exit(1);
}

