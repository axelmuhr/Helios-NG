/*
 * C++ interface to OS authorization-related functions
 */

#ifndef os_auth_h
#define os_auth_h

extern unsigned short getuid();
extern unsigned short getgid();
extern unsigned short geteuid();
extern unsigned short getegid();

extern int setuid(int);
extern int setgid(int);

extern char* crypt(char* key, char* salt);
int encrypt(char*);

#endif
