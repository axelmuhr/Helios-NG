/* $Id: inet.h,v 1.3 1992/06/18 13:31:57 nickc Exp $ */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * inet.h from UCB 5.1 5/30/85
 *
 */

#include <netinet/in.h>

/*
 * External definitions for
 * functions in inet(3N)
 */
unsigned long 	inet_addr( const char * host_name );
char *		inet_ntoa( struct in_addr in );
struct	in_addr inet_makeaddr( int net, int lna );
unsigned long	inet_network( const char * network_name );
