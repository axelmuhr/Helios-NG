#ifndef __ftpd_h
#define __ftpd_h

extern void fatal( char * s );
extern void ack(  char * s );
extern void nack( char * s );
extern void yyerror( char * s );
extern void mydelete( char * name );
extern void cwd( char * path );
extern void makedir( char * name );
extern void removedir( char * name );
extern void pwd( void );
extern void reply( int n, char * fmt, ... );
extern void dologout( int status );
extern void dolog( struct sockaddr_in * sin );
extern void lostconn( int a );
extern void lreply( int n, char * fmt, ... );
extern void user( char * name );
extern void passive( void );
extern void myoob( void );
extern void perror_reply( int code, char * strng );
extern void store( char * name, char * mode, int unique );
extern void send_file_list( char * whichfiles );
extern void statfilecmd( char * filename );
extern void renamecmd( char * from, char * to );
extern char *  renamefrom( char * name );
extern void    retrieve( char * cmd, char * name );
extern void    pass( char * passwd );
extern void    statcmd( void );

extern char ** glob( char * v );
extern char *  globerr;
extern char ** copyblk( char ** v );
extern void    blkfree(	char ** av0 );

extern int     yyparse( void );
extern void    upper( char * s );
extern char *  getline(	char * s, int n, FILE * iop );

extern void    logwtmp(	char * line, char * name, char * host );

#endif /* __ftpd_h */
