/* string.h: ANSI draft (X3J11 Oct 86) library header, section 4.11 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 - SccsId: %W% %G% */
/* $Id: string.h,v 1.4 90/11/16 14:42:39 nick Exp $ */

#ifndef __string_h
#define __string_h

#ifndef size_t
#  define size_t unsigned int
#endif

void *memcpy(void *s1, const void *s2, size_t n);
void *memmove(void *s1, const void *s2, size_t n);
char *strcpy(char *s1, const char *s2);
char *strncpy(char *s1, const char *s2, size_t n);

char *strcat(char *s1, const char *s2);
char *strncat(char *s1, const char *s2, size_t n);

int memcmp(const void *s1, const void *s2, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);


void *memchr(const void *s, int c, size_t n);
char *strchr(const char *s, int c);
size_t strcspn(const char *s1, const char *s2);
char *strpbrk(const char *s1, const char *s2);
char *strrchr(const char *s, int c);
size_t strspn(const char *s1, const char *s2);
char *strstr(const char *s1, const char *s2);
char *strtok(char *s1, const char *s2);

void *memset(void *s, int c, size_t n);
char *strerror(int errnum);
size_t strlen(const char *s);

int strcoll(const char *s1, const char *s2);  /* BLV - this used to be wrong */
size_t strxfrm(char *s1, const char *s2, size_t n);  /* and this was missing */

#ifndef _POSIX_SOURCE
#ifdef _BSD
/* BSD compatibility - use with bsd.lib */

extern void   bcopy(char *from, char *to, int size);
extern void   bzero(char *to, int size);
extern int    bcmp(char *a, char *b, int size);
extern char * index(char *s, char c);
extern char * rindex(char *s,char c);

extern int strcasecmp(char *a, char *b);
extern int strncasecmp(char *a, char *b, int n);

#endif
#endif

#endif

/* end of string.h */
