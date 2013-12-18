/*	ETYPE:		Global function type definitions for
			MicroEMACS 3.9

                        written by Daniel Lawrence
                        based on code by Dave G. Conroy,
                        	Steve Wilhite and George Jones
*/

/*	Modifications:
	11-Sep-89	Mike Burrow (INMOS)	Added folding.
*/


/* ALL global fuction declarations */

BUFFER *PASCAL NEAR bfind();
BUFFER *PASCAL NEAR getdefb();
BUFFER *PASCAL NEAR getcbuf();
char *PASCAL NEAR bytecopy();
char *PASCAL NEAR complete();
char *PASCAL NEAR envval();
char *PASCAL NEAR fixnull();
char *PASCAL NEAR flook();
char *PASCAL NEAR funval();
char *PASCAL NEAR getctext();
char *PASCAL NEAR getfname();
char *PASCAL NEAR getkill();
char *PASCAL NEAR getreg();
char *PASCAL NEAR getval();
char *PASCAL NEAR gtenv();
char *PASCAL NEAR gtfilename();
char *PASCAL NEAR gtfun();
char *PASCAL NEAR gtusr();
char *PASCAL NEAR int_asc();
char *PASCAL NEAR ltos();
char *PASCAL NEAR makename();
char *PASCAL NEAR mklower();
char *PASCAL NEAR mkupper();
char *PASCAL NEAR namval();
char *PASCAL NEAR timeset();
char *PASCAL NEAR token();
char *PASCAL NEAR transbind();
char *PASCAL NEAR trimstr();
char *PASCAL NEAR xlat();
int (PASCAL NEAR *PASCAL NEAR fncmatch())();
int (PASCAL NEAR *PASCAL NEAR getname())();
int PASCAL NEAR absv();
int PASCAL NEAR amatch();
int PASCAL NEAR biteq();
int PASCAL NEAR boundry();
int PASCAL NEAR cclmake();
int PASCAL NEAR checknext();
int PASCAL NEAR desfunc();
int PASCAL NEAR desvars();
int PASCAL NEAR dispvar();
int PASCAL NEAR echochar();
int PASCAL NEAR eq();
int PASCAL NEAR ernd();
int PASCAL NEAR execkey();
int PASCAL NEAR fbound();
int PASCAL NEAR fexist();
int PASCAL NEAR fisearch();
int PASCAL NEAR getkey();
int PASCAL NEAR gettyp();
int PASCAL NEAR getwpos();
int PASCAL NEAR get_char();
int PASCAL NEAR indx();			/* MJB: 20-Sep-89 */
int PASCAL NEAR initlinelist();		/* MJB: 15-Sep-89 */
int PASCAL NEAR loffset();		/* MJB: 25-Sep-89 */
int PASCAL NEAR match_pat();
int PASCAL NEAR mceq();
int PASCAL NEAR mcscanner();
int PASCAL NEAR mcstr();
int PASCAL NEAR minleftmarg();		/* MJB: 17-Oct-89 */
int PASCAL NEAR nextch();
int PASCAL NEAR promptpattern();
int PASCAL NEAR pushline();		/* MJB: 15-Sep-89 */
int PASCAL NEAR readpattern();
int PASCAL NEAR reglines();
int PASCAL NEAR remmark();
int PASCAL NEAR replaces();
int PASCAL NEAR risearch();
int PASCAL NEAR scanmore();
int PASCAL NEAR scanner();
int PASCAL NEAR setfoldmarks();         /* BG:  11-Oct-89 */
int PASCAL NEAR setlower();
int PASCAL NEAR setupper();
int PASCAL NEAR setvar();
int PASCAL NEAR sindex();
int PASCAL NEAR stol();
int PASCAL NEAR svar();
int PASCAL NEAR tgetc();
int PASCAL NEAR tindx();		/* MJB: 26-Sep-89 */
int PASCAL NEAR uneat();
LINE *PASCAL NEAR lalloc();
LINE *PASCAL NEAR lback();		/* MJB: 14-Sep-89 */
LINE *PASCAL NEAR lforw();		/* MJB: 14-Sep-89 */
LINE *PASCAL NEAR mouseline();
LINE *PASCAL NEAR popline();		/* MJB: 15-Sep-89 */
PASCAL NEAR addline();
PASCAL NEAR adjustmode();
PASCAL NEAR anycb();
PASCAL NEAR apro();
PASCAL NEAR asc_int();
PASCAL NEAR backchar();
PASCAL NEAR backdel();
PASCAL NEAR backhunt();
PASCAL NEAR backline();
PASCAL NEAR backpage();
PASCAL NEAR backsearch();
PASCAL NEAR backword();
PASCAL NEAR bbackline();		/* MJB: 13-Oct-89 */
PASCAL NEAR bclear();
PASCAL NEAR betawarning();		/* MJB: 30-Oct-89 */
PASCAL NEAR bforwline();		/* MJB: 13-Oct-89 */
PASCAL NEAR bindtokey();
PASCAL NEAR bktoshell();
PASCAL NEAR buildlist();
PASCAL NEAR capword();
PASCAL NEAR cbuf();
PASCAL NEAR cbuf1();
PASCAL NEAR cbuf10();
PASCAL NEAR cbuf11();
PASCAL NEAR cbuf12();
PASCAL NEAR cbuf13();
PASCAL NEAR cbuf14();
PASCAL NEAR cbuf15();
PASCAL NEAR cbuf16();
PASCAL NEAR cbuf17();
PASCAL NEAR cbuf18();
PASCAL NEAR cbuf19();
PASCAL NEAR cbuf2();
PASCAL NEAR cbuf20();
PASCAL NEAR cbuf21();
PASCAL NEAR cbuf22();
PASCAL NEAR cbuf23();
PASCAL NEAR cbuf24();
PASCAL NEAR cbuf25();
PASCAL NEAR cbuf26();
PASCAL NEAR cbuf27();
PASCAL NEAR cbuf28();
PASCAL NEAR cbuf29();
PASCAL NEAR cbuf3();
PASCAL NEAR cbuf30();
PASCAL NEAR cbuf31();
PASCAL NEAR cbuf32();
PASCAL NEAR cbuf33();
PASCAL NEAR cbuf34();
PASCAL NEAR cbuf35();
PASCAL NEAR cbuf36();
PASCAL NEAR cbuf37();
PASCAL NEAR cbuf38();
PASCAL NEAR cbuf39();
PASCAL NEAR cbuf4();
PASCAL NEAR cbuf40();
PASCAL NEAR cbuf5();
PASCAL NEAR cbuf6();
PASCAL NEAR cbuf7();
PASCAL NEAR cbuf8();
PASCAL NEAR cbuf9();
PASCAL NEAR cex();
PASCAL NEAR cinsert();
PASCAL NEAR clean();
PASCAL NEAR closefold();	/* MJB: 11-Sep-89 */
PASCAL NEAR clrmes();
PASCAL NEAR cmdstr();
PASCAL NEAR copyregion();
PASCAL NEAR crypt();
PASCAL NEAR ctlxe();
PASCAL NEAR ctlxlp();
PASCAL NEAR ctlxrp();
PASCAL NEAR ctoec();
PASCAL NEAR ctrlg();
PASCAL NEAR dcline();
PASCAL NEAR deblank();
PASCAL NEAR debug();
PASCAL NEAR delbword();
PASCAL NEAR delfold();		/* MJB: 21-Sep-89 */
PASCAL NEAR delfword();
PASCAL NEAR delgmode();
PASCAL NEAR delins();
PASCAL NEAR delmode();
PASCAL NEAR delwind();
PASCAL NEAR desbind();
PASCAL NEAR deskey();
PASCAL NEAR detab();
PASCAL NEAR dobuf();
PASCAL NEAR docmd();
PASCAL NEAR dofile();
PASCAL NEAR ectoc();
PASCAL NEAR edinit();
PASCAL NEAR editloop();
PASCAL NEAR endword();
PASCAL NEAR enlargewind();
PASCAL NEAR entab();
PASCAL NEAR enterfold();	/* MJB: 11-Sep-89 */
PASCAL NEAR execbuf();
PASCAL NEAR execcmd();
PASCAL NEAR execfile();
PASCAL NEAR execprg();
PASCAL NEAR execproc();
PASCAL NEAR execprog();
PASCAL NEAR execute();
PASCAL NEAR exitallfolds();	/* MJB: 21-Sep-89 */
PASCAL NEAR exitfold();		/* MJB: 11-Sep-89 */
PASCAL NEAR expandp();
PASCAL NEAR ffclose();
PASCAL NEAR ffgetline();
PASCAL NEAR ffputline();
PASCAL NEAR ffropen();
PASCAL NEAR ffwopen();
PASCAL NEAR filefind();
PASCAL NEAR filename();
PASCAL NEAR fileread();
PASCAL NEAR filesave();
PASCAL NEAR filewrite();
PASCAL NEAR fillpara();
PASCAL NEAR filter();
PASCAL NEAR findvar();
PASCAL NEAR fmatch();
PASCAL NEAR fnclabel();
PASCAL NEAR forwchar();
PASCAL NEAR forwdel();
PASCAL NEAR forwhunt();
PASCAL NEAR forwline();
PASCAL NEAR forwpage();
PASCAL NEAR forwsearch();
PASCAL NEAR forwword();
PASCAL NEAR freewhile();
PASCAL NEAR getccol();
PASCAL NEAR getcline();
PASCAL NEAR getcmd();
PASCAL NEAR getfence();
PASCAL NEAR getfile();
FOLDMARKENT *PASCAL NEAR getftype();	/* BG:  11-Oct-89 */
PASCAL NEAR getgoal();
PASCAL NEAR getrawregion();	/* MJB: 26-Sep-89 */
PASCAL NEAR getregion();
PASCAL NEAR getstring();
PASCAL NEAR gotobob();
PASCAL NEAR gotobol();
PASCAL NEAR gotobop();
PASCAL NEAR gotoeob();
PASCAL NEAR gotoeol();
PASCAL NEAR gotoeop();
PASCAL NEAR gotoline();
PASCAL NEAR gotomark();
PASCAL NEAR help();
PASCAL NEAR ifile();
PASCAL NEAR indent();
PASCAL NEAR initchars();
PASCAL NEAR initfoldstrings();
PASCAL NEAR insbrace();
PASCAL NEAR insfile();
PASCAL NEAR inspound();
PASCAL NEAR insspace();
PASCAL NEAR inword();
PASCAL NEAR isearch();
PASCAL NEAR ismodeline();
PASCAL NEAR istring();
PASCAL NEAR kdelete();
PASCAL NEAR killbuffer();
PASCAL NEAR killpara();
PASCAL NEAR killregion();
PASCAL NEAR killtext();
PASCAL NEAR kinsert();
PASCAL NEAR lchange();
PASCAL NEAR ldelete();
PASCAL NEAR ldelnewline();
PASCAL NEAR lfree();
PASCAL NEAR linsert();
PASCAL NEAR linstr();
PASCAL NEAR listbuffers();
PASCAL NEAR lnewline();
PASCAL NEAR long_asc();
PASCAL NEAR lover();
PASCAL NEAR lowerregion();
PASCAL NEAR lowerword();
PASCAL NEAR lowrite();
PASCAL NEAR macarg();
PASCAL NEAR macrotokey();
PASCAL NEAR makefold();		/* MJB: 11-Sep-89 */
PASCAL NEAR makelist();
PASCAL NEAR makelit();
PASCAL NEAR mcclear();
PASCAL NEAR meexit();
PASCAL NEAR meta();
PASCAL NEAR mlerase();
PASCAL NEAR mlforce();
PASCAL NEAR mlout();
PASCAL NEAR mlputf();
PASCAL NEAR mlputi();
PASCAL NEAR mlputli();
PASCAL NEAR mlputs();
PASCAL NEAR mlreply();
PASCAL NEAR mltreply();
PASCAL NEAR mlyesno();
PASCAL NEAR modeline();
PASCAL NEAR mouseoffset();
PASCAL NEAR movecursor();
PASCAL NEAR movemd();
PASCAL NEAR movemu();
PASCAL NEAR mregdown();
PASCAL NEAR mregup();
PASCAL NEAR mvdnwind();
PASCAL NEAR mvupwind();
PASCAL NEAR namebuffer();
PASCAL NEAR namedcmd();
PASCAL NEAR narrow();
PASCAL NEAR newline();
PASCAL NEAR newsize();
PASCAL NEAR newwidth();
PASCAL NEAR nextarg();
PASCAL NEAR nextbuffer();
PASCAL NEAR nextdown();
PASCAL NEAR nextup();
PASCAL NEAR nextwind();
PASCAL NEAR nullproc();
PASCAL NEAR onlywind();
PASCAL NEAR openfold();		/* MJB: 11-Sep-89 */
PASCAL NEAR openline();
PASCAL NEAR openoutfolds();	/* MJB: 13-Oct-89 */
PASCAL NEAR ostring();
PASCAL NEAR outstring();
PASCAL NEAR ovstring();
PASCAL NEAR pipecmd();
PASCAL NEAR prevwind();
PASCAL NEAR putctext();
PASCAL NEAR putline();
PASCAL NEAR qreplace();
PASCAL NEAR quickexit();
PASCAL NEAR quit();
PASCAL NEAR quote();
PASCAL NEAR rdonly();
PASCAL NEAR readin();
PASCAL NEAR reform();
PASCAL NEAR reframe();
PASCAL NEAR refresh();
PASCAL NEAR removefold();	/* MJB: 11-Sep-89 */
PASCAL NEAR reposition();
PASCAL NEAR resetkey();
PASCAL NEAR resize();
PASCAL NEAR resizm();
PASCAL NEAR resterr();
PASCAL NEAR restwnd();
PASCAL NEAR rmcclear();
PASCAL NEAR rmcstr();
PASCAL NEAR rvstrcpy();
PASCAL NEAR savematch();
PASCAL NEAR savewnd();
PASCAL NEAR scwrite();
PASCAL NEAR searchffold();	/* MJB: 21-Sep-89 */
PASCAL NEAR searchbfold();	/* MJB: 21-Sep-89 */
PASCAL NEAR setbit();
PASCAL NEAR setccol();
PASCAL NEAR setekey();
PASCAL NEAR setfillcol();
PASCAL NEAR setgmode();
PASCAL NEAR setjtable();
PASCAL NEAR setmark();
PASCAL NEAR setmod();
PASCAL NEAR shellprog();
PASCAL NEAR showcpos();
PASCAL NEAR shrinkwind();
PASCAL NEAR spal();
PASCAL NEAR spawn();
PASCAL NEAR spawncli();
PASCAL NEAR splitwind();
PASCAL NEAR sreplace();
PASCAL NEAR startup();
PASCAL NEAR storemac();
PASCAL NEAR storeproc();
PASCAL NEAR strinc();
PASCAL NEAR swapmark();
PASCAL NEAR swbuffer();
PASCAL NEAR tab();
PASCAL NEAR trim();
PASCAL NEAR ttclose();
PASCAL NEAR ttflush();
PASCAL NEAR ttgetc();
PASCAL NEAR ttopen();
PASCAL NEAR ttputc();
PASCAL NEAR twiddle();
PASCAL NEAR typahead();
PASCAL NEAR unarg();
PASCAL NEAR unbindchar();
PASCAL NEAR unbindkey();
PASCAL NEAR unmark();
PASCAL NEAR unqname();
PASCAL NEAR updall();
PASCAL NEAR update();
PASCAL NEAR updateline();
PASCAL NEAR upddex();
PASCAL NEAR updext();
PASCAL NEAR updgar();
PASCAL NEAR updone();
PASCAL NEAR updpos();
PASCAL NEAR updupd();
PASCAL NEAR upmode();
PASCAL NEAR upperregion();
PASCAL NEAR upperword();
PASCAL NEAR upscreen();
PASCAL NEAR upwind();
PASCAL NEAR usebuffer();
PASCAL NEAR varclean();
PASCAL NEAR varinit();
PASCAL NEAR viewfile();
PASCAL NEAR vteeol();
PASCAL NEAR vtfree();
PASCAL NEAR vtinit();
PASCAL NEAR vtmove();
PASCAL NEAR vtputc();
PASCAL NEAR vttidy();
PASCAL NEAR widen();
PASCAL NEAR wordcount();
PASCAL NEAR wrapword();
PASCAL NEAR writemsg();
PASCAL NEAR writeout();
PASCAL NEAR yank();
PASCAL NEAR zotbuf();
unsigned int PASCAL NEAR chcase();
unsigned int PASCAL NEAR getckey();
unsigned int PASCAL NEAR stock();
KEYTAB *getbind();
WINDOW *PASCAL NEAR mousewindow();
WINDOW *PASCAL NEAR wpopup();

#if	MSDOS & TURBO
PASCAL NEAR binary(char *, char *(PASCAL NEAR *)(), int);
#else
PASCAL NEAR binary();
#endif

#if	COMPLET
char *PASCAL NEAR getffile();
char *PASCAL NEAR getnfile();
#endif

char PASCAL NEAR upperc();
char PASCAL NEAR lowerc();

#if	DIACRIT
int PASCAL NEAR islower();
int PASCAL NEAR isupper();
int PASCAL NEAR isletter();
#endif

#if	MAGIC
BITMAP PASCAL NEAR clearbits();
#endif

#if	DTL
CDECL NEAR mlwrite(...);
#else
CDECL NEAR mlwrite();
#endif

/* some library redefinitions */

char *strcat();
char *strcpy();
char *malloc();

