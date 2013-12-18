
/* objects.c : Message passing testbed for use with Norcroft C */
/*             Copyright (C) Codemist Ltd, 1988                */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nonansi.h>


int _unknown_method(BasicObject *object, Message message)
{
    int n = fprintf(stderr, "Message %s not recognized by object ",
                             name_of_message(message));
    return n + _send(object, Print__, stderr);
}

#define MESSAGE_TABLE_SIZE 8
#define MESSAGE_TABLE_LENGTH (1<<MESSAGE_TABLE_SIZE)
#define MESSAGE_TABLE_MASK (MESSAGE_TABLE_LENGTH-1)

char *message_name_table[MESSAGE_TABLE_LENGTH];

char *name_of_message(Message message)
{
    static char message_buff[50];
    if ((unsigned)message < (unsigned)MESSAGE_TABLE_LENGTH)
    {   char *s = message_name_table[message];
        if (s == 0)
        {   sprintf(message_buff, "<message code %#x not in use>", message);
            return message_buff;
        }
        else return s;
    }
    sprintf(message_buff, "<message code %#x out of range>", message);
    return message_buff;
}

Message message_number(char *name)
{
    int hash = 12345, ch, probes;
    char *p = name;
    while ((ch = *p++) != 0)
    {   unsigned temp = hash << 7;
        hash = ((hash >> 25) ^ (temp >> 1) ^ (temp >> 4) ^ ch) & 0x7fffffff;
    }
    hash &= MESSAGE_TABLE_MASK;
    for (probes = 0;probes < MESSAGE_TABLE_SIZE;probes++)
    {   char *n = message_name_table[hash];
        if (n == NULL)
        {   message_name_table[hash] = name;
            return (Message)hash;
        }
        else if (strcmp(n, name) == 0) return (Message)hash;
        else hash = (hash + 1) & MESSAGE_TABLE_SIZE;
    }
    fprintf(stderr, "Message table full up - object system fails!\n");
    return 0;
}

ClassObject *root_object;

MethodPair root_methods[] = {{0,0}, {0,0}, {0,0}, {0,0}};

ClassObject real_root =
    {0,             /* superclass */
     root_methods,  /* method table */
     3};            /* method table size */

Message Print__;

int print_root_object(void *object, FILE *stream)
{
    return fprintf(stream, "<root object at %p>", object);
}

void obj_init()
{
    MethodPair *p;
    int i;
    for (i = 0; i < MESSAGE_TABLE_LENGTH; i++)
        message_name_table[i] = 0;
    Print__ = message_number("print");
    real_root.class = &real_root;
    p = real_root.methods;
    for (i = 0; i < 3; i++)
        p[i].message = 0,
        p[i].code = (function)_unknown_method;
    p[Print__ & 3].message = Print__;
    p[Print__ & 3].code = (function)print_root_object;
    root_object = &real_root;
}

/* End of objects.c */

