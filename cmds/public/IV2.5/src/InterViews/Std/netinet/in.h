/*
 * C++ interface to internet definitions.
 */

#ifndef netinet_in_h

#include "//usr/include/netinet/in.h"

/* just in case standard header didn't */
#ifndef netinet_in_h
#define netinet_in_h
#endif

/*
 * Can't use the library names because
 * C header might use macros or prototype-less functions.
 */

#if defined(htons)

/*
 * The operations are defined as macros, so we can
 * define the functions using the macros.
 */

inline u_short short_host_to_net (u_short x) {
    return htons(x);
}

inline u_long long_host_to_net (u_long x) {
    return htonl(x);
}

inline u_short short_net_to_host (u_short x) {
    return ntohs(x);
}

inline u_long long_net_to_host (u_long x) {
    return ntohl(x);
}

#else

#if defined(vax)
/*
 * VAX Ultrix doesn't define anything yet, so add it here.
 * When Ultrix defines them, remove this #if - #endif code.
 */

extern u_short htons(), ntohs();
extern u_long htonl(), ntohl();

#endif

/*
 * The operations are defined as functions, possibly without prototypes.
 * So, we cast pointers to the functions and call indirect.
 *
 * Yuck!!
 */

typedef u_short (*_nsfunc)(u_short);
typedef u_long (*_nlfunc)(u_long);

inline u_short short_host_to_net (u_short x) {
    return (*((_nsfunc)&htons))(x);
}

inline u_long long_host_to_net (u_long x) {
    return (*((_nlfunc)&htonl))(x);
}

inline u_short short_net_to_host (u_short x) {
    return (*((_nsfunc)&ntohs))(x);
}

inline u_long long_net_to_host (u_long x) {
    return (*((_nlfunc)&ntohl))(x);
}

#endif

#endif
