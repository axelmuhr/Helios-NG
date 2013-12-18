/*
 * Author:      Alex Schuilenburg
 * Date:        18 July 1994
 *
 * Copyright 1994 Perihelion Distributed Software Limited
 *
 * $Id: getargs.c,v 1.1 1994/08/03 10:47:31 al Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include "getargs.h"

/* Internal Types */
struct getargstackstruct {
    char                            **argv;
    struct getargstackstruct        *next;
};

struct getargstackstruct    *getargstack = NULL;  

/*
 * Routine to read in a file contents and convert it to
 * argc, argv style.
 * It is passed the file name and an address of argc.
 * It returns argv or NULL on error.
 */

char **getargs(char *name, int *rargc)
{
    int     argc, argc_size;
    char    **rargv;
    long    l, p;
    FILE    *f;
    char    *ch;
    struct getargstackstruct    *newitem;

    /* Default return */
    *rargc = 0;
    
    /* 1st open the file */
    if ((f = fopen(name, "rb")) == NULL) {
        return NULL;
    }
    
    /* Allocate memory the same size as the file */
    fseek(f, 0, SEEK_END);
    l = ftell(f);
    fseek(f, 0, SEEK_SET);
    if ((ch = (char *)malloc((int)l+1)) == NULL) {
        fclose(f);
        return NULL;
    }

    /* Allocate some buffer space for argv */
    argc_size = 10;
    if ((rargv = (char **)malloc(argc_size * sizeof(char *))) == NULL) {
	    free(ch);
        fclose(f);
        return NULL;
    }

    /* Read in the entire file */
    p = 0;
    while (!feof(f) && (p < l)) {
        p += fread(&ch[p], sizeof(char), (int)l - (int)p, f);
    }
    l = p;
    fclose(f);
    
    /* 1st position is the whole array */
    rargv[0] = ch;         
    argc = 1;

    /* Parse into the list */
    p = 0;
    while (p < l) {
        if (ch[p] == '#') {
            /* This line is a comment */
            p++;
            while ((p < l) && !((ch[p] == '\r') || (ch[p] == '\n')))
                p++;
        } else if ((ch[p] == '\n') || (ch[p] == '\r') || (ch[p] == '\t') || (ch[p] == ' ')) {
            /* Skip <nl> <cr> and whitespace */
        } else {
            /* We have a new argv */

            /* Enough room ? */
            if ((argc+1) == argc_size) {
                /* We must grow */
                char **newptr;
                
                argc_size += 10;
                newptr = (char **)realloc(rargv, sizeof(char *) * argc_size);
                if (newptr == NULL) {
                    /* No more memory */
                    free(rargv[0]);
                    free(rargv);
                    return NULL;
                }

                rargv = newptr;
            }
        
            /* Add the item */
            if (ch[p] == '\'') {
                /* This is in quotes */
                p++;                            /* Skip Quote */
                rargv[argc] = &ch[p++];

                /* Skip until quote */
                while ((p < l) && (ch[p] != '\'')) {
                    p++;
                }

                /* Terminate */
                ch[p] = '\0';
            } else if (ch[p] == '"') {
                /* This is also in quotes */
                p++;                            /* Skip Quote */
                rargv[argc] = &ch[p++];

                /* Skip until quote */
                while ((p < l) && (ch[p] != '"')) {
                    p++;
                }

                /* Terminate */
                ch[p] = '\0';
            } else {
                /* Standard argument */
                rargv[argc] = &ch[p++];

                /* Skip until \n, \r or whitespace */
                while ((p < l) && (ch[p] != '\n') && (ch[p] != '\r') &&
                       (ch[p] != '\t') && (ch[p] != ' ')) {
                    p++;
                }

                /* Terminate */
                ch[p] = '\0';
            }

            /* New argc */
            argc++;
        }

        /* Next character */
        p++;
    }

    /* If we do not have, we free everything */
    if (argc == 1) {
        /* Free all */
        free(rargv[0]);
        free(rargv);
        return NULL;
    } else {
        /* The last one must be a NULL */
        rargv[argc] = NULL;
    }

    /* Advance to the next */
    rargv++;
    *rargc = argc-1;                    /* One less than you think */

    /* Add on the stack */
    newitem = (struct getargstackstruct *)malloc(sizeof(struct getargstackstruct));
    if (newitem != NULL) {
        newitem->argv = rargv;
        newitem->next = getargstack;
        getargstack = newitem;
    }    
    
    return rargv;
}

/*
 * This routine is used to free the argv pointer and data area.
 */
void freeargs()
{
    char                        **rargv;
    struct getargstackstruct    *olditem;

    while (getargstack != NULL) {
        /* Remove 1st item */
        olditem = getargstack;
        getargstack = olditem->next;

        /* Set the bit to delete */
        rargv = olditem->argv;    
        if (rargv != NULL) {
            /* Move back by one */
            rargv--;

            /* Now we can free */
            free(*rargv);                   /* Free the buffer */
            free(rargv);                    /* Free the **argv */
        }

        /* And free the old item */
        free(olditem);
    }
}

/*
 * This routine is used to push the arguments onto an ArgStack
 */
void    pushargs(ArgStack **argstack, int argc, char **argv)
{
    ArgStack    *newitem;

    /* Get new */
    newitem = (ArgStack *)malloc(sizeof(ArgStack));

    /* Add to stack */
    if (newitem != NULL) {
        newitem->argc = argc;
        newitem->argv = argv;
        newitem->next = *argstack;
        *argstack = newitem;
    }
}

/*
 * This routine is used to pop arguments from an ArgStack
 */
void    popargs(ArgStack **argstack, int *argc, char ***argv)
{
    ArgStack    *olditem;
    
    /* Catch any funny routines */
    if ((argstack == NULL) || (*argstack == NULL))
        return;

    /* Take old off stack */
    olditem = *argstack;
    *argstack = olditem->next;

    /* Set results */
    *argc = olditem->argc;
    *argv = olditem->argv;

    /* Free */
    free(olditem);
}


