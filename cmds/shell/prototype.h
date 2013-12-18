/**
*
* Title:  Helios Shell - Prototype header file
*
* Author: Andy England
*
* $Header: /hsrc/cmds/shell/RCS/prototype.h,v 1.11 1993/08/12 15:20:03 nickc Exp $
*
**/

extern char **environ;
extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];

extern BOOL memorychecking;
extern int totalmemory;
extern CMD *globcmd;
extern ARGV globargv;
extern BOOL debugging;
extern BOOL exitonerror;
extern BOOL fast;
extern BOOL singleline;
extern BOOL parsingerror;
extern BOOL interactive;
extern BOOL login;
extern BOOL historyused;
extern BOOL usingcdl;
extern BOOL exitflag;
extern BOOL backgroundtask;
extern int innewfile;
extern FILE *inputfile;
extern char *filename;
extern char wordbuffer[ WORD_MAX + 1];
extern SUBSTATE wordstate;
extern int wordindex;
extern int eventnumber;
extern long lineposition;

extern jmp_buf home;
extern ARGV wordlist;
extern char *currentword;
extern BOOL lastword;
extern TOKEN token;
extern int parencount;
extern int mode;
extern int waitwrpid;
extern int shellpid;
extern BOOL breakflag;
extern int sfds[3];
extern int fdssaved;

extern LIST aliaslist;
extern LIST varlist;
extern LIST historylist;
extern LIST dirlist;
extern LIST joblist;
extern LIST filelist;
extern LIST looplist;
extern BUILTIN builtins[ MAX_BUILT ];


#ifdef __STDC__
/* builtin.c */
void getlastword(char *);
char *getword(char *);
BOOL isspecial(char *);
BOOL needsfullsubs(int (*)());
BUILTIN *findbuiltin(char *);
int b_alias(int, char **);
int b_alloc(int, char **);
int b_bg(int, char **);
int b_break(int, char **);
int b_breaksw(int, char **);
int b_case(int, char **);
int b_cd(int, char **);
int b_continue(int, char **);
int b_default(int, char **);
int b_dirs(int, char **);
int b_echo(int, char **);
int b_else(int, char **);
int b_end(int, char **);
int b_endif(int, char **);
int b_endsw(int, char **);
int b_eval(int, char **);
int b_exec(int, char **);
int b_exit(int, char **);
int b_fault(int, char **);
int b_fg(int, char **);
int b_foreach(int, char **);
int b_glob(int, char **);
int b_goto(int, char **);
int b_hashstat(int, char **);
int b_history(int, char **);
int b_if(int, char **);
int b_job(int, char **);
int b_jobs(int, char **);
int b_kill(int, char **);
int b_label(int, char **);
int b_limit(int, char **);
int b_login(int, char **);
int b_logout(int, char **);
int b_nice(int, char **);
int b_nohup(int, char **);
int b_notify(int, char **);
int b_onintr(int, char **);
int b_popd(int, char **);
int b_printenv(int, char **);
int b_pushd(int, char **);
int b_pwd(int, char **);
int b_rehash(int, char **);
int b_repeat(int, char **);
int b_set(int, char **);
int b_setenv(int, char **);
int b_shift(int, char **);
int b_source(int, char **);
int b_stop(int, char **);
int b_suspend(int, char **);
int b_switch(int, char **);
int b_time(int, char **);
int b_umask(int, char **);
int b_unalias(int, char **);
int b_unhash(int, char **);
int b_unlimit(int, char **);
int b_unset(int, char **);
int b_unsetenv(int, char **);
int b_version(int, char **);
int b_wait(int, char **);
int b_which(int, char **);
int b_while(int, char **);
int b_at(int, char **);

/* shell.c */
void shell(void);
BOOL docmdline(void);
int runcmdline(char *);
void initialise(int, char **);
void logout(int);
char *homename(char *, char *);
BOOL source(char *);
BOOL record(char *);
BOOL getinterpreter(char *, char *);
int runcmdlist(CMD *);
int runpipeline(CMD *);
int runcmd(CMD *, int, int);
int runlist(CMD *, IOINFO *, int, int);
int runsimplecmd(ARGV, IOINFO *, int, int, int);
void executecmd(char *, ARGV);
int waitforcmd(int);
BOOL redirect(IOINFO *, int, int);
BOOL wait2(int *, int);

/* error.c */
int syserr(char *);
int error(int, char *);
void restorefds(int *);
void recover(void);
void bug(char *);

/* extra.c */
BOOL pushdir(char *);
BOOL changedir(char *);
BOOL dirs(void);
char *strdup(char *);
char *newmemory(int);
void freememory(int *);
void putmem(void);
void newfile(FILE *);
void oldfile(void);
void tidyupfiles(void);
void newloop(void);
void oldloop(void);
void tidyuploops(void);
BOOL inloop(void);

/* hash.c */
void hash(void);
void unhash(void);
void hashstat(void);
DIRECTORY *adddirectory(char *);
DIRECTORY *finddirectory(char *);
void freedirectory(DIRECTORY *);
ENTRY *addentry(DIRECTORY *, char *);
ENTRY *findentry(DIRECTORY *, char *);
void freeentry(ENTRY *);
BOOL lookforcmd(char *, char *);
void listcmds(char *);
BOOL completecmd(char *);
void listfiles(char *);
BOOL completefile(char *);
char *splitfilename(char *, char *, char *);
void formfilename(char *, char *, char *);
int hashnumber(char *);
int strlequ(char *, char *);

/* helios.c */
int fifo(int fds[2]);
BOOL isdir(DIRENT *, char *);
BOOL isexec(DIRENT *, char *);
void sysinit(void);
void systidy(void);
void ctrlcbegin(void);
void ctrlcend(void);
void ctrlcwaiter(void);
void initenv(void);
void freeenv(void);
void setenv(char *, char *);
void delenv(char *);
void terminit(void);
void raw(void);
int termgetc(FILE *);
void cooked(void);
void fault(unsigned long);

/* job.c */
void newjob(CMD *, int);
void freejob(JOB *);
void killjob(JOB *);
JOB *currentjob(void);
JOB *previousjob(void);
JOB *findjob(int);
JOB *getjob(int);
void putjob(JOB *);
void notifyjob(int, int);
void pendingjobs(void);
void putjobtable(void);

/* line.c */
char *getline(char *, BOOL);
BOOL handlecsi(void);
void replaceline(void);
void putline(void);
BOOL complete(void);
BOOL display(void);
BOOL retrieve(void);
BOOL refresh(void);
int getthisword(char *);
BOOL incmd(void);
BOOL nothing(void);
BOOL undo(void);
void inserttext(char *);
void putprompt(void);
BOOL ctrlc(void);
BOOL left(void);
BOOL right(void);
BOOL startofline(void);
BOOL endofline(void);
BOOL up(void);
BOOL down(void);
BOOL deletechar(void);
BOOL erasechar(void);
BOOL rightword(void);
BOOL leftword(void);
BOOL deleteword(void);
BOOL eraseword(void);
BOOL deletetoend(void);
BOOL insertchar(int);
void update(void);
void storeline(char *);
BOOL fetchline(char *);
void rewindinput(void);
void resetinput(void);
long note(void);
void point(long);
void freelinevector(void);

/* list.c */
void fputargv(FILE *, ARGV, BOOL);
void sputargv(char *, ARGV, char);
void putsortedargv(ARGV);
ARGV nullargv(void);
ARGV makeargv(char *);
ARGV nummakeargv(int);
ARGV envmakeargv(char *);
ARGV buildargv(ARGV );
ARGV dupargv(ARGV );
ARGV addword(ARGV , char *);
ARGV prefixword(ARGV , char *);
void set(char *, ARGV );
BOOL setword(char *, ARGV , int, char *);
void unset(char *);
void freesubnode(SUBNODE *);
void fputsublist(FILE *, LIST *, int, BOOL, BOOL, BOOL);
int lenargv(ARGV);
void freeargv(ARGV);
void addsubnode(LIST *, char *, ARGV);
void setsubnode(LIST *, char *, ARGV);
void remsubnode(LIST *);
void patremsubnode(LIST *, char *);
char *getsubnode(LIST *, char *);
ARGV findsubnode(LIST *, char *);
void addevent(ARGV);
ARGV findevent(int);
ARGV findhistory(char *);
int lensublist(LIST *);
void adddir(char *);
BOOL popdir(char *);
BOOL getdir(int, char *);
void putdirnode(DIRNODE *);
void freedirnode(DIRNODE *);

/* parse.c */
void initexprparse(ARGV);
BOOL initparse(ARGV);
void tidyupparse(void);
void setexprtoken(void);
void settoken(void);
void readexprtoken(void);
void readtoken(void);
BOOL checkfor(TOKEN);
BOOL isdelimitor(TOKEN);
CMD *readpipeline(void);
CMD *readcmd(void);
CMD *readsimplecmd(void);
CMD *readcmdlist(int);
CMD *readauxiliaries(void);
CMD *readauxiliary(void);
IOINFO *readredirection(IOINFO *);
void freeexpr(EXPR *);
void freecmd(CMD *);
void freeioinfo(IOINFO *);
CMD *dupcmd(CMD *);
IOINFO *dupioinfo(IOINFO *);
void sputcmd(char *, CMD *);
void putcmd(CMD *);
void putioinfo(IOINFO *);
int evaluate(EXPR *);
char *streval(char *, EXPR *);
EXPR *readexpr(int);
EXPR *readprefixexpr(void);
BOOL match(char *, char *);
int removetoken(char *buffer);

/* signal.c */
BOOL testbreak(void);
void siginit(void);
void sighandler(int);
void putctrlc(void);
void sigstart(void);
void sigstop(void);

/* sub.c */
ARGV fullsub(ARGV);
ARGV smallsub(ARGV);
ARGV historysub(ARGV);
ARGV varsub(ARGV);
ARGV cmdsub(ARGV);
ARGV filenamesub(ARGV);
ARGV searchdir(ARGV, char *, char *);
BOOL ispattern(char *);
ARGV quotesub(ARGV);
char *readname(char *, char *);
char *readnumber(char *, int *, int);
int getdigit(int);
char *readdesignator(char *, int *);
char *readargnumber(char *, int *);
char *readselector(char *, int *);
void addchar(int);
BOOL endword(void);

/* cdl.c */
int runtaskforce(CMD *);
int addargv(ARGV);

#else
/* builtin.c */
void getlastword();
char *getword();
BOOL isspecial();
BOOL needsfullsubs();
BUILTIN *findbuiltin();
int b_alias();
int b_alloc();
int b_bg();
int b_break();
int b_breaksw();
int b_case();
int b_cd();
int b_continue();
int b_default();
int b_dirs();
int b_echo();
int b_else();
int b_end();
int b_endif();
int b_endsw();
int b_eval();
int b_exec();
int b_exit();
int b_fg();
int b_foreach();
int b_glob();
int b_goto();
int b_hashstat();
int b_history();
int b_if();
int b_job();
int b_jobs();
int b_kill();
int b_label();
int b_limit();
int b_login();
int b_logout();
int b_nice();
int b_nohup();
int b_notify();
int b_onintr();
int b_popd();
int b_pushd();
int b_rehash();
int b_repeat();
int b_set();
int b_setenv();
int b_shift();
int b_source();
int b_stop();
int b_suspend();
int b_switch();
int b_time();
int b_umask();
int b_unalias();
int b_unhash();
int b_unlimit();
int b_unset();
int b_unsetenv();
int b_version();
int b_wait();
int b_while();
int b_at();

/* shell.c */
void shell();
BOOL docmdline();
int runcmdline();
int runcmdlist();
int invoke();
void executecmd();
void initialise();
void logout();
char *homename();
BOOL source();
BOOL record();
BOOL redirect();

/* error.c */
int syserr();
int error();
void recover();
void bug();

/* extra.c */
BOOL pushdir();
BOOL changedir();
BOOL dirs();
char *strdup();
char *newmemory();
void freememory();
void putmem();
void newfile();
void oldfile();
void tidyupfiles();
void newloop();
void oldloop();
void tidyuploops();
BOOL inloop();

/* hash.c */
void hash();
void unhash();
void hashstat();
DIRECTORY *adddirectory();
DIRECTORY *finddirectory();
void freedirectory();
ENTRY *addentry();
ENTRY *findentry();
void freeentry();
BOOL lookforcmd();
void listcomands();
BOOL completecmd();
void listfiles();
BOOL completefile();
char *splitfilename();
void formfilename();
int hashnumber();

/* job.c */
void newjob();
void freejob();
void killjob();
JOB *currentjob();
JOB *previousjob();
JOB *findjob();
JOB *getjob();
void putjob();
void notifyjob();
void pendingjobs();
void listjobs();

/* line.c */
char *getline();
void replaceline();
void putline();
BOOL complete();
BOOL display();
BOOL retrieve();
BOOL refresh();
int getthisword();
BOOL incmd();
BOOL nothing();
BOOL undo();
void inserttext();
void putprompt();
BOOL ctrlc();
BOOL left();
BOOL right();
BOOL startofline();
BOOL endofline();
BOOL up();
BOOL down();
BOOL deletechar();
BOOL erasechar();
BOOL rightword();
BOOL leftword();
BOOL deleteword();
BOOL eraseword();
BOOL deletetoend();
BOOL insertchar();
void update();
void storeline();
BOOL fetchline();
void rewindinput();
void resetinput();
long note();
void point();
void freelinevector();

/* list.c */
void fputargv();
void sputargv();
ARGV nullargv();
ARGV makeargv();
ARGV nummakeargv();
ARGV envmakeargv();
ARGV buildargv();
ARGV dupargv();
ARGV addword();
ARGV prefixword();
void set();
BOOL setword();
void freesubnode();
void fputsublist();
int lenargv();
void freeargv();
void addsubnode();
void setsubnode();
void remsubnode();
void patremsubnode();
char *getsubnode();
ARGV findsubnode();
void addevent();
ARGV findevent();
ARGV findhistory();
int lensublist();
void adddir();
BOOL popdir();
BOOL getdir();
void putdirnode();
void freedirnode();

/* parse.c */
void initexprparse();
BOOL initparse();
void tidyupparse();
void setexprtoken();
void settoken();
void readexprtoken();
void readtoken();
BOOL checkfor();
CMD *readpipeline();
CMD *readcmd();
CMD *readsimplecmd();
CMD *readcmdlist();
CMD *readauxiliaries();
CMD *readauxiliary();
IOINFO *readredirection();
void freeexpr();
void freecmd();
void freeioinfo();
CMD *dupcmd();
IOINFO *dupioinfo();
void putcmd();
void putioinfo();
int evaluate();
char *streval();
EXPR *readexpr();
EXPR *readprefixexpr();
BOOL match();

/* signal.c */
BOOL testbreak();
void siginit();
void sighandler();

/* sub.c */
ARGV fullsub();
ARGV smallsub();
ARGV historysub();
ARGV modifiersub();
ARGV varsub();
ARGV cmdsub();
ARGV filenamesub();
ARGV searchdir();
BOOL ispattern();
ARGV quotesub();
char *readname();
char *readnumber();
int getdigit();
char *readdesignator();
char *readargnumber();
char *readselector();
void addchar();
BOOL endword();

/* system.c */
void unixpath();
void syspath();
DIR *opendir();
struct direct *readdir();
char *strstr();
char *getenv();
char *getcwd();
BOOL isdir();
BOOL isexec();
void InitList();
void AddHead();
void AddTail();
void PreInsert();
void Remove();
NODE *RemHead();
#endif

