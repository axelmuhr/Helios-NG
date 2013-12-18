
/* <nonansi.h>     A. Mycroft & A. C. Norman. 1988 */

/* Experimental extra library for use with Norcroft C. */
/* Not for general distribution (yet?)                 */

/* sqrt(x*x+y*y) but calculated with care */
extern double hypot(double x, double y);

/* random number generator yielding 32-bit random value */
extern int lrand(void);
extern void slrand(unsigned int newseed);

/* Try-out for an Objective-C like message passing system */
extern int _send(void *object, int message, ...);

typedef int (*function)(void *object, ...);

typedef int Message;

typedef struct MethodPair
{
    Message message;
    function code;
} MethodPair;

typedef struct ClassObject
{
    struct ClassObject *class;
    MethodPair *methods;
    int methodmask;
/* further fields will often exist too.. */
} ClassObject;

typedef struct BasicObject
{
    ClassObject *class;
/* further fields will often exist too.. */
} BasicObject;


extern char *name_of_message(Message message);
extern Message message_number(char *name);

extern ClassObject *root_object;
extern Message Print__;

extern void obj_init(void);

/* End of nonansi.h */
