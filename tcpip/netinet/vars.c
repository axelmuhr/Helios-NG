
/* variables for netinet code */

#include "param.h"
#include "ioctl.h"
#include "mbuf.h"
#include "protosw.h"
#include "socket.h"
#include "socketvar.h"
#include "user.h"
#include "in_systm.h"
#include "../net/if.h"
#include "../net/route.h"
#include "../net/af.h"
#include "domain.h"
#include "in.h"
#include "netinet/ip.h"
#include "in_pcb.h"

/* IP */

#include "in_var.h"
struct	ifqueue	ipintrq;		/* ip packet input queue */

#include "ip_var.h"
struct	ipstat	ipstat;
struct	ipq	ipq;			/* ip reass. queue */
u_short	ip_id;				/* ip packet ctr, for ids */


/* ICMP */

#include "ip_icmp.h"
#include "icmp_var.h"
struct	icmpstat icmpstat;

/* UDP */

#include "udp.h"
#include "udp_var.h"
struct	inpcb udb;
struct	udpstat udpstat;

/* TCP */

#include "tcp.h"
#include "tcp_seq.h"
#include "tcp_timer.h"
tcp_seq	tcp_iss;		/* tcp initial send seq # */

#include "tcp_var.h"
struct	inpcb tcb;		/* head of queue of active tcpcb's */
struct	tcpstat tcpstat;	/* tcp statistics */

#include "tcpip.h"
#include "tcp_debug.h"
struct	tcp_debug tcp_debug[TCP_NDEBUG];
int	tcp_debx;



