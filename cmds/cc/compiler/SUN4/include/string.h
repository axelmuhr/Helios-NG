
/* string.h: ANSI draft (X3J11 Oct 86) library header, section 4.11 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 */

#ifndef __string_h
#define __string_h

#ifndef size_t
#  define size_t unsigned int
#endif

void *memcpy();
void *memmove();
char *strcpy();
char *strncpy();

char *strcat();
char *strncat();

int memcmp();
int strcmp();
int strncmp();
size_t strcoll();

void *memchr();
char *strchr();
size_t strcspn();
char *strpbrk();
char *strrchr();
size_t strspn();
char *strstr();
char *strtok();

void *memset();
char *strerror();
size_t strlen();

#endif

/* end of string.h */
