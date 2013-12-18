/*******************************************************************************
*
*	Loadpac : Main Program.
*	Date 	: July 1990.
*
*
*	Copyright Perihelion Software 1990.
*
*	Programmed by
*	     James Dowdells
*       
*	Installs software from disk. On MSDOS systems, disk contains at least
*	the directory install with installation scripts therein.
*	On UNIX type systems, assume that the disk contains a tar archive.
*	The files in the archive must ALWAYS be in {/helios/}tmp/loadpac
*
*******************************************************************************/
#ifdef __TRAN
static char *rcsid = "$Header: /dsl/HeliosARM/Working/RTNucleus/cmds/com/loadpac.c,v 1.1.1.1 1994/05/10 09:20:24 nickc Exp $";
#endif

#include <attrib.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <helios.h>
#include <nonansi.h>
#include <posix.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslib.h>
#include <unistd.h>


#define CLEAR putchar(0x0c)
#define COMMANDLINE curpos(1, Commandline)
#define MAXLINE 80
#define BELL	7
#define ESC	27
#define SLOW	1
#define FAST	2

#define PC


struct instalables
{
char name[100];
char version[10];
char date[100];
int size;
int loaded;
}; 


struct installfiles
{
	char name[100];
	char version[10];
	int size;
};

struct Lines
{
	char Letters[MAXLINE];
	int LineLength;
	int EndOfWord;
};

struct Words
{
	char Letters[MAXLINE];
	int WordLength;
};



FILE *InputStr;
FILE *stream;
char diskname[256];
char drivename[256];
char entry[256];
int Commandline;
int Counting;
int CurrentInTableSize;
int CurrentTableSize;
int Length;
int NoOfFiles;
int NoOfStand;
int NoOnDisk;
int Width;
int device = 0;
int msdos = 0;
int tar_read = 0;
struct Lines aline;
struct Words aword;
struct instalables intable[100];
struct instalables table[100];
struct installfiles ListFiles[100];
struct installfiles TempListFiles[100];

char *BANNER = "		Loadpac Software Management V2.0\n\n";
char *OPTION1 = "   (1)	Change disk drive name. (Currently : %s)\n";
char *OPTION2 = "   (2)	List installed software.\n";
char *OPTION3 = "   (3)	List software on a disk.\n";
char *OPTION4 = "   (4)	Install new software.\n";
char *OPTION5 = "   (5)	Deinstall software.\n";

#define DISK_ERROR	"\n%s\n%s %s\n",			\
		"FUBAR!!! Error reading loadpac disk",	\
		"Please insert correct disk in drive", diskname

int raw(void (*)());
int doareyousure(void);
int doinput(int which);
int getdata(int choice,char* name,char* version);
int getdata2(int choice,char* name,char* version);
int getline(void);
int getlist(void);
int getword(void);
int main(void);
int makeintable(void);
int makenum(char *strnum);
int putintable(int NoLines);
int read_tar(int howmuch);
int search(char* name);
int setuptable(void);
void PutTopMenu(void);
void TopMenu(void);
void clearcommand(void);
void clearline(int number);
void clearlines(int number);
void curpos(int x, int y);
void dochangedisk(void);
void dodisk(char *disc);
void dodiskinstall(int choice, int single);
void dohelppage(void);
void doinstall(void);
void donoise(void);
void doquit(void);
void douninstall(int choice);
void handlemany(int choice);
void init(void );
void lerror(char *str);
void makediskpath(char *diskpath, char *name);
void makepath(char *result, char *dir, char *file);
void mpause(void);
void processline(void);
void putheading (void);
void putheading2 (void);
void putinslist(int CurLine);
void putinstalled(void);
void putname(char *name, int maxlength);
void readline(char *buffer, int ofst);
void removepackage(void);
#if 0
void setmedia(void);
#endif
void updatetable (void);

int main(void)
{

	init();

	setuptable();      	/* init table of packages */
	makeintable();		/* init table of packages installed */
	TopMenu();		/* Handle the installation/removal */
}



void PutTopMenu(void)
{
	CLEAR;
	printf(BANNER);
	printf(OPTION1, drivename);
	printf(OPTION2);
	printf(OPTION3);
	printf(OPTION4);
	printf(OPTION5);

}

void TopMenu(void)
{	
	char c;
	int i;

	forever
	{
		PutTopMenu();
		forever
		{
			COMMANDLINE;
			printf("Select (1 2 3 4 5) for desired option or 'q' to quit : ");
			fflush(stdout);
			c = toupper(fgetc(InputStr));
			if (c >= '1' && c <= '5')
				break;
			else if (c == 'Q')
				doquit();
			else 
			{
				donoise();
				clearcommand();
			}	
		}	

		CLEAR;
		printf(BANNER);

		switch(c) {

		  case '1':
		     printf(OPTION1, drivename);
		     dochangedisk();
		     break;

		  case '2':
		     printf(OPTION2);
		     if (CurrentInTableSize < 1)
			     printf("\nNothing Installed.\n");
		     else 
			putintable(5);
		     mpause();
		     break;

		  case '3':
		     printf(OPTION3);
		     printf("\nPlease insert the disk you wish to inspect.\n");
		     printf("Press any key when ready : ");
		     fflush(stdout);
		     c = fgetc(InputStr);

		     tar_read = 0;
		     if(!read_tar(FAST))
			break;
			
		     if ((i = getlist()) > 0) {
		        printf("\nInstallable programs on this disk are : \n");
		        putinslist(9);
		        mpause();
		     }
		     else if (! i)
		     {
			printf(DISK_ERROR);
			mpause();
		     }
		     break;
			
		  case '4':
		     doinstall();
     		     break;

		  case '5':
		     printf(OPTION5);
		     removepackage();
		  
		}
	}
}


void removepackage(void)

/* Handle removal of packages */

{
	int choice;
	if (CurrentInTableSize == 0)
	{
		printf("\nNothing installed.\n");
		mpause();
	}
	else
	{
		forever
		{	
			CLEAR;
			printf(BANNER);
			printf(OPTION5);
			putintable(7);
			choice = doinput(0);
			if (choice == -1)
				break;
			else
				douninstall(choice -1);
		}
	}
}

void doinstall(void)

/* Handle the installation of packages */

{
	int choice;
	
	forever
	{
		CLEAR;
		printf(BANNER);
		printf(OPTION4);
		putinstalled();
		choice = doinput(1);
		if (choice  == -1)
			break;
		if (choice != 0) 
		{
			if (choice > CurrentTableSize)
				handlemany(choice - 1);
			else
				dodiskinstall(choice-1, 1);
		}
	}
}

void handlemany(int choice)

/* Handle the installation  packages on a disk which may be unknown */

{
	char c, name[100], version[100];
	int selection, start, end, isloaded, i, size;
		
	clearcommand();
	COMMANDLINE;
	printf("\nPlease insert the disk you wish to install\n");		
	printf("Press any key to proceed : ");
	fflush(stdout);
	c = fgetc(InputStr);

	tar_read = 0;
	if(!read_tar(SLOW))
		return;

	if (!getlist())
	{
		CLEAR;
		printf(DISK_ERROR);
		mpause();
	}
	else
	if (NoOnDisk > 0)
	{
		CLEAR;
		printf("\nApplications on the disk are : \n");
		putinslist(3);
		forever
		{
			clearcommand();
			COMMANDLINE;
			printf("\nPress RETURN to go back, enter number corresponding to application you\n");
			printf("wish to install or enter 'all' to install all applications in list : ");
			fflush(stdout);	
			readline(entry,0);
			if (!entry[0])
				return;
			if (strcmp(entry, "all") == 0)
			{
				start = 0;
				end = NoOnDisk - 1;
				break;
			}
			selection = makenum(entry);
			if ((selection >= 1) && (selection <= NoOnDisk))
				{
				start = end = selection - 1;
				break;
				}
			else
				donoise();
		}
		
		for (i = start ; i <= end ; i++)
			{
			isloaded = search(ListFiles[i].name);
			if (isloaded < 0)
				{

				strcpy(table[CurrentTableSize].name, ListFiles[i].name);
				CurrentTableSize++;

				size = getdata(choice, name, version);
				if (size != -1)
					{
					strcpy(table[choice].version, version);
					table[choice].size = size;
					table[choice].loaded = 0;
					dodiskinstall(choice++, 0);
					}
				}
			else dodiskinstall(isloaded, 0);		
			}
	}
}

int search(char* name)

/* search installed table for the given name */

{
	int i;	
	for (i = 0 ; i < CurrentTableSize; i++)
		if (strcmp(table[i].name, name) == 0)
			return i;
	return -1;	
}

#if 0

void setmedia(void)
{
	char reply;

	device = 1;
		
	forever
	{
		clearcommand();
		COMMANDLINE;
		printf("Select distribution media (1 disk, 2 tape) : ");
		fflush(stdout);
		reply = fgetc(InputStr);
		switch(reply)
		{
		case '1':
			device = 0;
			return;
		case '2':
			return;
		default: 
			donoise();
		}
	}
}

#endif

void dodiskinstall(int choice, int single)

/* do the installation from a disk */

{
	char name[100], aname[10], version[100], command[512];
	int size, disk = 1, i;
	FILE *fd;
	
	clearcommand();
	COMMANDLINE;

	if (single)
		{
		printf("\nPlease insert Loadpac disk no %d", disk);
		printf(" for %s into disk drive.\n", table[choice].name);
		tar_read = 0;
		}
	else
		printf("\nReady to proceed with installation of %s.\n",
						table[choice].name);
	printf("Press any key to proceed : ");
	fflush(stdout);
	fgetc(InputStr);

	if( ! read_tar(SLOW))
		return;

#if 0
	setmedia();
#endif
	CLEAR;

	if ((size = getdata(choice, name, version)) == -1)
	{
		printf(DISK_ERROR);
	}
	else
	{
        printf ("table[choice].name = \"%s\", name = \"%s\"\n", 
		    table[choice].name, name);
	    
	if ((strcmp(name, table[choice].name) != 0) && (choice < CurrentTableSize))
		printf("\n*** Loadpac: not disk for %s\n", table[choice].name);
	else
	{
	  char c = 'N';
	  
		if (table[choice].loaded == 1)
		{
			forever
			{
				clearcommand();
				COMMANDLINE;
				printf("\n%s is already installed.\n", table[choice].name);
				printf("Do you wish to reinstall %s (y/n) : ", table[choice].name);
				fflush(stdout);
				c = toupper(fgetc(InputStr));
				if (c == 'Y' || c == 'N')
					break;
				donoise();
			}
		}
		if ((table[choice].loaded == 0) || (c == 'Y'))
		{
			CLEAR;
			strcpy(table[choice].version, version);
			table[choice].size = size;

			printf("\nLoading the application %s ...\n",
						table[choice].name);
			fflush(stdout);
			for (i = 0; i <4; i++)
				aname[i] = table[choice].name[i];
			aname[4] = 0;
			
			/*move install script to harddrive*/
			sprintf(command, "%s/install/%s.ins",
						diskname, aname);

			if((fd = fopen(command, "r")) != (FILE *) NULL)
				{
				fclose(fd);

				sprintf(command,
					"cp %s/install/%s.ins /helios/system",
						diskname, aname);
				if(system(command))
					lerror("copy error, install aborted");

				else {

				/* execute install script*/
			
				sprintf(command,
					"/helios/system/%s.ins 1 %s %s",aname, 
					diskname, device ? "tape" : "disk");
				raw(RemoveAttribute);
				if(system(command))
					lerror("installation failed");
			
				else {
					table[choice].loaded  = 1;
					printf("\n%s installed.\n",
						 table[choice].name);
					updatetable();
					makeintable();
				}
				raw(AddAttribute);
				}
				}
			else
				lerror("file open error, install aborted");
			fflush(stdout);
		}
		else lerror("installation aborted");
	}
	}
	printf("\nAny key to continue - ");
	fflush(stdout);
	fgetc(InputStr);
}


void updatetable (void)
/* updates file storing info on packages */
{
	int i;
	
	stream = fopen("/helios/system/in", "w");
	fprintf(stream,"STANDARD\n");
	for (i = 0; i < CurrentTableSize ; i++)
	{
		if (i == NoOfStand)
			fprintf(stream,"NONSTANDARD\n");
		fprintf(stream, "%s ", table[i].loaded ? "L" : "U");
		fprintf(stream,"%s ",table[i].name);
		if (table[i].size != 0)
			fprintf(stream,"%s %d", table[i].version, table[i].size);
		fprintf(stream,"\n");
	}		
	fclose(stream);
}



int getdata(int choice,char* name,char* version)
{

	char path[512];
	char aname[10];
	int i;
	int size;
		
	for (i = 0; i <4; i++) /* cuts program name down to first 4 letters */
 		aname[i] = table[choice].name[i];
	aname[i] = 0;

	sprintf(path, "%s/install/%s.ins", diskname, aname);

	stream = fopen(path, "r");
	if (stream == NULL)
	{
		printf ("Failed to open %s\n", path);
	    
		return -1;
	}
	else
	{	
		fscanf(stream, " # %s %s %d", name , version, &size); 
		fclose(stream);
		return size;
	}
}


void douninstall(int choice)
{
	char command[30], aname[5], c;
	int t, i;
	FILE *fd;
	
	/* make path name of script */
	for (i = 0; i <4; i++)
		aname[i] = intable[choice].name[i];
	aname[4] = '\0';

	sprintf(command, "/helios/system/%s.ins", aname);

	if((fd = fopen(command, "r")) != (FILE *) NULL)
		{
		fclose(fd);
		strcat(command, " 0");

		forever
		{
			printf("\nAre you sure (y/n) : ");
			fflush(stdout);
			c = toupper(fgetc(InputStr));
			if (c == 'Y' || c == 'N')
				break;
			else 
			{
				clearline(0);
				donoise();
			}
		}
		if (c == 'Y')
		{
			CLEAR;
			printf("\nRemoving %s ...\n",intable[choice].name);
			raw(RemoveAttribute);
			if(system(command))
				lerror("removal failed"); 
			else {
			for (t = 0; t < CurrentTableSize; t++)
				if (strcmp(table[t].name,intable[choice].name)
					 == 0) table[t].loaded = 0;
			printf("\n%s removed.\n",intable[choice].name);
			updatetable(); /* update table of installed software */
			makeintable(); /* update table of current software */
			}
			raw(AddAttribute);
		}
	}
	else
		lerror("file open error, removal aborted");
	printf("\nAny key to continue - ");
	fflush(stdout);
	fgetc(InputStr);
}

void clearcommand(void)
{
	int i;

	for(i = Commandline ; i <= Length ; i++)
		clearline(i);
}


void clearline(int number)
{
	if(number)
		curpos(1,number);
	printf("\r                                                                             \r");
}

void clearlines(int number)
{
	while(number--)
	printf("\r                                                                             \n");
}


void init(void )
{
	Attributes attr;

#define CTERMID_WORKS
#ifdef CTERMID_WORKS
	char *instr;
#ifdef __HELIOS
	char *ctermid ( char* );
#endif
	instr = (char *) (malloc ( L_ctermid * sizeof (char)));
#ifdef __HELIOS
	instr = ctermid ( instr );
#endif
	InputStr = fopen ( instr , "r" );
#else
	InputStr = stdin;
#endif

	setvbuf(InputStr,NULL,_IONBF,0);
	
#ifdef __HELIOS
	if( ! raw(AddAttribute))
		exit(1);
#else
	system( "stty -echo" );
	system( "stty raw" );
#endif

	if (GetAttributes( fdstream( fileno( stdout ) ), &attr ) < 0)
	{
		exit( 3 );
	}

	Width = (attr.Time > 80 ? 80 : attr.Time);
	Length = (attr.Min > 25 ? 25 : attr.Min);

	if(Width < 70 || Length < 18)
		{
		fprintf(stderr,
		    "Loadpac : Window too small (%d x %d)\n", Width, Length);
		exit(4);
		}

	if(! isatty(fileno(stdout)))
		{
		fprintf(stderr,
		    "Loadpac : No output redirection\n");
		exit(5);
		}

	Commandline = Length - 4;

#ifdef PC
		msdos = 1;
		strcpy(diskname,"/a");
		strcpy(drivename,"/a");
#else
		strcpy(diskname,"/helios/tmp/loadpac");
		strcpy(drivename,"/files/dev/rfd0c");
		tar_read = 0;
#endif
	
	signal( SIGINT, SIG_IGN);
}	

void readline(char *buffer, int ofst)
{
	char c, *p;

	p = buffer + ofst;

	while((c = fgetc(InputStr)) != '\n'&& c != '\r')
		{
		if(c >= ' ' && c <= 126)
			*p++ = c;
		if((c == 127 || c == 8) && p != buffer)
			{
			printf(" \b");
			fflush(stdout);
			p--;
			}
		}
	*p = 0;
}

int getlist(void)

/* Gets the list of installable programs on a disk */
{
	DIR *thedir;
	struct dirent *adir;	
	char target[5], name[30], diskpath[276], version[30];
	int count = 3, found = 1, i;

	NoOfFiles = 0;

	sprintf(diskpath, "%s/install", diskname);
	strcpy(target, ".ins");

	if ((thedir = opendir(diskpath)) != NULL )
		{
		forever
			{
			adir = readdir(thedir);
			if (adir == NULL) 
				break;
			else
				{
				found = 1;
				count = strlen(adir -> d_name) - 1;
				for (i = 3; i >=0; i--)
					if (adir ->d_name[count--] != target[i])
						found = 0;
				if (found)
					{ 	
					for(i=0;i<(strlen(adir->d_name)-4);i++)
					    TempListFiles[NoOfFiles].name[i] =
							adir->d_name[i];
					NoOfFiles++;
					}
				}	
			}
		for (i = 0; i < NoOfFiles; i++)
			{
			ListFiles[i].size = getdata2(i,name, version);
			strcpy(ListFiles[i].name,name);
			strcpy(ListFiles[i].version,version);
			}
		NoOnDisk = NoOfFiles;
		return 1;
		}
	return 0;	
}


int getdata2(int choice,char* name,char* version)

/* Gets info for a file in the list of files on disk */

{

	char path[276];
	char aname[10];
	int i;
	int size;
		
	for (i = 0; i <4; i++)
		aname[i] = TempListFiles[choice].name[i];
	aname[4] = 0 ;
	sprintf(path, "%s/install/%s.ins", diskname, aname);

	stream = fopen(path, "r");
	fscanf(stream, " # %s %s %d", name , version, &size);
	fclose(stream);
	return size;
}


int setuptable(void)

/* make the table of installable software */

{	
	Counting = 0;
	NoOfStand = 0;
	CurrentTableSize = 0;
	stream = fopen("/helios/system/in", "r");	
	if (stream == NULL)
		{
		printf("ERROR: can't find /helios/system/in file");
		printf("\nAny key to continue - ");
		fflush(stdout);
		fgetc(InputStr);
		}
	else
	{
		while (!getline())
	 		processline();
		fclose(stream);
	}
	return CurrentTableSize;
}

int getline(void)

/* gets a line of text from a file */

{
	int achar;
	int count = 0;
	achar = fgetc(stream);
	while ((achar != '\n') && (count < MAXLINE) && (achar != EOF))
	{
		if(achar != '\r')
		{
			aline.Letters[count] = achar;
			count = count + 1;
		}
		achar = fgetc(stream);
	}
	aline.Letters[count] = '\0';
	if (achar == EOF)
		return 1;
	aline.LineLength = count - 1;
	aline.EndOfWord = 0;	
	return 0;
}


void processline(void)

/* processes a line of text */

{

	(void) getword();	
	if (aword.Letters[0] != '#')
	{	
		if (strcmp(aword.Letters,"STANDARD") == 0)
			Counting = 1;
		else if (strcmp(aword.Letters, "NONSTANDARD") == 0)
			Counting = 0;
		else if ((aword.Letters[0] == 'U') || (aword.Letters[0] == 'L'))
			{
			if (Counting == 1) 
				NoOfStand++;
			table[CurrentTableSize].loaded = 1;
			if (aword.Letters[0] == 'U')
				table[CurrentTableSize].loaded = 0;
			if (getword())
			  strcpy(table[CurrentTableSize].name, aword.Letters);

			strcpy(table[CurrentTableSize].version, " ");
			if (getword())
			  strcpy(table[CurrentTableSize].version,
							aword.Letters);

			table[CurrentTableSize].size = 0;
			if (getword())
				table[CurrentTableSize].size =
					makenum(aword.Letters);
			CurrentTableSize++;
			}	
	}
}


int getword(void)

/* gets a word from a 'line' of text */

{
	int count;
	int pos = 0;
	
	count = aline.EndOfWord;
	while ((count < MAXLINE) && (aline.Letters[count] == ' ') && (count <= aline.LineLength))	
		count++; /*find start of first word*/
	
	while ((count < MAXLINE) && (aline.Letters[count] != ' ') && (count <= aline.LineLength))
		aword.Letters[pos++] = aline.Letters[count++];
	aline.EndOfWord = count;
	aword.Letters[pos] = '\0';
	aword.WordLength = pos;

	return pos;
}


int makeintable(void)

/* makes a table of software already installed */

{
	int i;
	int count = 0;
	
	for (i = 0; i < CurrentTableSize; i++)
	{
		if (table[i].loaded == 1)
		{
			strcpy(intable[count].name,table[i].name);
			strcpy(intable[count].version,table[i].version);
			intable[count].size = table[i].size;	
			count++;
		}
	}
	CurrentInTableSize = count;
	return count;
}



void dochangedisk(void)
{
	curpos(1,5);
	if(msdos)
		{
		printf("	Select 1 for drive /a\n");
		printf("	Select 2 for drive /b\n");
		printf("	Select 3 for other device name.\n\n");
		printf("  Press any other key to go back or 'q' to quit\n");
		printf("Make Selection : ");
		fflush(stdout);
		switch(toupper(fgetc(InputStr)))
			{
			case 'Q':
				doquit();
	
			case '1':
				dodisk("/a");
				return;

			case '2':
				dodisk("/b");

			default:
				return;

			case '3':
				break;
			}
		}

	printf("\nEnter name of disk drive : ");
	fflush(stdout);
	readline(entry,0);
	dodisk(entry);
}	

int doareyousure(void)
{
	char c;

	forever
	{
		printf("Are you sure (y/n) : ");
		fflush(stdout);
		c = toupper(fgetc(InputStr));
		if (c == 'Y')
			return 1;
		else if (c == 'N')
			return 0;
		else 
		{
			clearline(0);
			donoise();
		}
	}
}

void dodisk(char *disc)
{
	if(doareyousure)
	{
		strcpy(drivename, disc);
		if(msdos)
			strcpy(diskname, disc);
		else
			tar_read = 0;
		printf("\nDisk drive changed to %s\n", disc);
	}
	else
		printf("\nDisc drive unchanged\n");
}

int makenum(char *strnum)

/* Convert a string into a number */

{
	int result = 0;
	
	while(isdigit(*strnum))
		result = result*10 + (*strnum++ - '0');
	return result;
}


void donoise(void)

/* Make bell noise */

{
	putchar(BELL);
}

void putinstalled(void)

/* prints list of base and optional packages on screen */

{
	
	int i;
	int TotalSize = 0;
	int Selection = 1;
	int NoLines = 10;
	char *type = "base";
	
	putchar('\n');
	putheading2();	 	
	for (i = 0 ; i < CurrentTableSize ; i++)
	{
		if (NoLines == Commandline)
		{
			printf("\nHit any key to continue list: ");
			fflush(stdout);
			fgetc(InputStr);
			NoLines = 9;
			CLEAR;
			printf(BANNER);
			if(i != NoOfStand)
				putheading2();
		}
		if(i == NoOfStand)
			type = "optional";
		printf("  %d	",Selection);
		putname((table[i].loaded ? "Yes" : "No"), 8);
     		putname(table[i].name, 23);
     		putname(table[i].version, 10);
     		if (table[i].size)
			printf("%4d K",table[i].size);
		printf("\t  %s\n", type);
     		if (table[i].loaded != 0)
			TotalSize += table[i].size;
      		Selection++;
		NoLines++;
	}
	
	printf("\nEnter %d to install unlisted software\n", Selection); 

	printf("Total space used is %d Kbytes.\n", TotalSize);

	return; 
}

int putintable(int CurLine)

/* Prints table of installed software on screen */

{
	int i;
	int TotalSize = 0;
	int Selection = 1;
 	
 	putheading();
	for (i = 0 ; i < CurrentInTableSize ; i++)
	{
		if (CurLine == Commandline)
		{
			printf("\nHit any key to continue : ");
			fflush(stdout);
			fgetc(InputStr);
			CurLine = 6;
			CLEAR;
			printf(BANNER);
			putheading();
		}
		printf("  %d	",Selection);
     		putname(intable[i].name, 22);
     		putname(intable[i].version, 10);
           	printf("%4d K\n",intable[i].size);     	
     		TotalSize = TotalSize + intable[i].size;
      		Selection++;
		CurLine++;
	}
	putchar('\n');
	return Selection;	
}

void putinslist(int CurLine)

/* prints the list of installable packages on screen */

{
	int i;
	for ( i = 0; i < NoOfFiles; i ++)
	{
		if (CurLine == Commandline)
		{
			printf("\nHit any key to continue : ");
			fflush(stdout);
			fgetc(InputStr);
			CurLine = 6;
			CLEAR;
		}
		printf("   %d	", i+1);
		putname(ListFiles[i].name,22);
		putchar('V');
		putname(ListFiles[i].version,10);
		printf("	%4d Kbytes\n",ListFiles[i].size);
	}
	putchar('\n');		
}

void putname(char *name, int maxlength)
{
	printf("%-*s", maxlength, name);
}



void putheading (void)
{
	printf("\n	Name                  Version   Size\n ");
        printf("	----                  -------   ----\n");
}

void putheading2 (void)
{
printf("\tLoaded  Name                   Version   Size\t  Type\n ");
printf("\t------  ----                   -------   ----\t  ----\n");
}



int doinput(int which)
{
	int num;
	char c;
	
	forever
	{
		clearcommand();
		COMMANDLINE;
		if (which == 1)
		{	
			printf("\nSelect '?' for help, RETURN to return to main menu\n");  
			printf("       'q' to quit, or enter number of package you wish to load : ");
		}
		else 
		{
			printf("\nPress RETURN to return to main menu, 'q' to quit,\n");  
			printf("       or enter number of package you wish to remove : ");
		}
		fflush(stdout);
		switch(c = toupper(fgetc(InputStr)))
			{
			case '\n':
			case '\r':
				return -1;

			case 'Q':
				doquit();

			case '?':
				if(which)
					{
					dohelppage();
					return 0;
					}
				break;
			default:
				entry[0] = c;
				readline(entry,1);
				num = makenum(entry);
				if(num > 0 && num <=
				(which ? CurrentTableSize+1 : CurrentInTableSize))
					return num;
				donoise();
			}
	}
}




void mpause(void)
{
	forever
	{
		printf("Press RETURN to go back or 'q' to quit : ");
		fflush(stdout);
		switch(toupper(fgetc(InputStr)))
			{
			case 'Q':
				doquit();

			case '\n':
			case '\r':
				return;

			default:
				clearline(0);
				donoise();
		}
	}
}


void dohelppage(void)
{

	CLEAR;
	
	printf("\nInstallation Help\n");
	printf("------------------\n");
	printf("Entering RETURN returns you to the main menu.\n\n");
	printf("Entering 'q' terminates the installer program.\n\n");
	printf("Entering a number corresponding to the numbers listed next to\n");
	printf("the list of software will tell the program that you wish to install\n");
	printf("the software package corresponding to this number.\n");
	printf("You will then be given various prompts explaining what to do.\n");
	printf("Following these prompts will lead to the selected software package\n");
	printf("being installed.\n\n");
	printf("Press any key to go back : ");
	fflush(stdout);
	fgetc(InputStr);
}

void doquit(void)
{
	char command[30];
	printf("\nExiting ....");
	fflush(stdout);
	if(!msdos)
		{
		sprintf(command, "rm -rf %s", diskname);
		system(command);
		}
	printf("\n\n");
	exit(0);
}

void curpos(int x, int y)
{
	putchar(ESC);
	putchar('[');
	putchar((y/10)+'0');
	putchar((y%10)+'0');
	putchar(';');
	putchar((x/10)+'0');
	putchar((x%10)+'0');
	putchar('H');
	fflush(stdout);
}

void lerror(char *str)
{
	fprintf(stderr, "\n*** Loadpac: %s\n", str);
	fflush(stderr);
}

int read_tar(int howmuch)
{
	FILE *fd;
	char command[256];

	if(msdos || tar_read)
		return 1;

	CLEAR;
	printf("Reading archive from %s\n", drivename);
	sprintf(command, "rm -rf %s", diskname);
	system(command);

	if(howmuch == SLOW)
		sprintf(command, "(cd /helios ; tar xf %s tmp/loadpac)",
						drivename); 
	else
		sprintf(command, "(cd /helios ; tar xf %s tmp/loadpac/install)",
						drivename); 
	system(command);

	if((fd = fopen(diskname, "rb")) == (FILE *) NULL)
		{
		lerror("not loadpac format disc");
		printf("\nAny key to continue - ");
		fflush(stdout);
		fgetc(InputStr);
		return 0;
		}
	fclose(fd);
	CLEAR;
	tar_read = 1;
	return 1;
}

int raw(void (*doit)())
{
	Attributes attr;

	if (GetAttributes( fdstream( fileno( InputStr ) ), &attr ) < 0)
	{
		return FALSE;
	}
	
	doit(&attr, ConsoleRawInput);
		
	if (SetAttributes( fdstream( fileno( InputStr ) ), &attr ) < 0)
	{
		return FALSE;
	}
	
	return TRUE;
}
