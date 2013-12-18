
#include <device.h>
#include <codes.h>
#include <syslib.h>

#include "param.h"
#include "mbuf.h"
#include "socket.h"
#include "errno.h"
#include "ioctl.h"
#include "if.h"
#include "netisr.h"
#include "route.h"

#ifdef	INET
#include "../netinet/in.h"
#include "../netinet/in_systm.h"
#include "../netinet/in_var.h"
#include "../netinet/ip.h"
#endif

#include "../netinet/if_ether.h"

#define ep_dst		2		/* see comment in ether_rxdone() */
#define ep_src		8
#define ep_type		14
#define	ep_data		16

#define ETHERPKTSIZE	2000

struct arpcom ec_softc;

int etoutput(), etioctl();

void ether_init(), ether_tx(), ether_rxdone();

void
etattach()
{
	struct ifnet *ifp = &ec_softc.ac_if;
	
	ifp->if_name = "ec";
	ifp->if_mtu = ETHERMTU;
	ifp->if_flags = IFF_NOTRAILERS|IFF_BROADCAST;
	ifp->if_ioctl = etioctl;
	ifp->if_output = etoutput;
	
	ether_init(ec_softc.ac_enaddr);
	
	if_attach(ifp);
}

int etoutput(
	     struct ifnet *ifp,
	     register struct mbuf *m0,
	     struct sockaddr *dst )
{
	struct sockaddr_in *dstin = (struct sockaddr_in *)dst;
	struct mbuf *m;
	int usetrailers;
	u_char dstaddr[6];

	if(	(dst->sa_family == AF_UNSPEC) || 
		(arpresolve(&ec_softc,m0,&dstin->sin_addr,dstaddr,&usetrailers)) 
	  )
	{
		byte *pkt = (byte *) Malloc(ETHERPKTSIZE);
		char *pp;
		int size = 0;
		
		if( pkt == NULL ) 
		{
			m_freem(m0);
			return 0;
		}
		
		pp = pkt+ep_data;
		m = m0;
		
		while(m)
		{
			struct mbuf *	m2;
			char *		cp  = mtod(m, char *);
			int		len = m->m_len;
			
			memcpy(pp,cp,len);
			
			pp   += len;
			size += len;
			
			MFREE(m,m2);
			
			m = m2;
		}

		if( dst->sa_family == AF_UNSPEC )
		{
#if 0
{int i;
IOdebug("ARP output %d %",size+14);
for(i=0;i<14;i++)IOdebug("%x %",dst->sa_data[i]);
IOdebug("");
}
#endif
		
			memcpy(pkt+ep_dst,dst->sa_data,14);
			memcpy(pkt+ep_src,ec_softc.ac_enaddr,6);
			*(u_short *)(pkt+ep_type) = htons(*(u_short *)(pkt+ep_type));
		}
		else
		{
			memcpy(pkt+ep_dst,dstaddr,6);
			memcpy(pkt+ep_src,ec_softc.ac_enaddr,6);
			*(u_short *)(pkt+ep_type) = htons(ETHERTYPE_IP);
		}
		
		ether_tx(pkt,size+14);
	}
	
	return 0;
}

/*
 * Copy data buffer to mbuf chain; add ifnet pointer ifp.
 */
struct mbuf *
et_btom(
	void *		buf,
	register int	len,
	struct ifnet *	ifp )
{
	register caddr_t cp;
	register struct mbuf *m, **mp;
	register unsigned count;
	struct mbuf *top = NULL;

	cp = (caddr_t) buf;
	mp = &top;
	while (len > 0) {
		MGET(m, M_DONTWAIT, MT_DATA);
		if ((*mp = m) == NULL) {
			m_freem(top);
			return (NULL);
		}
		m->m_len = MLEN;
		
		if (ifp)
			m->m_off += sizeof(ifp);
		/*
		 * If we have at least NBPG bytes,
		 * allocate a new page.  
		 */

		if (len >= NBPG) 
		{
			MCLGET(m);
			if (ifp) m->m_off += sizeof(ifp);
		}
		if (ifp)
			count = MIN(len, m->m_len - sizeof(ifp));
		else
			count = MIN(len, m->m_len);

		bcopy(cp, mtod(m, caddr_t), count);

		m->m_len = count;
		if (ifp) {
			m->m_off -= sizeof(ifp);
			m->m_len += sizeof(ifp);
			*mtod(m, struct ifnet **) = ifp;
			ifp = NULL;
		}
		cp += count;
		len -= count;
		mp = &m->m_next;
	}
	return (top);
}

void
etinput(byte *pkt, int size)
{
	struct mbuf *m;
	u_short type;

	/* IOdebug( "etinput: buf = %x, size = %x", pkt, size ); */
	
	m = et_btom(pkt + ep_data, size, &ec_softc.ac_if);

	/* IOdebug( "etinput: got, m = %x", m ); */
	
	type = ntohs( *(u_short *)(pkt + ep_type) );
	
	*(u_short *)(pkt + ep_type) = type;

	if( type == ETHERTYPE_ARP ) 
	  {
	    /* IOdebug( "arp input" ); */
	  
	    arpinput(&ec_softc,m);
	  }
	else
	  {
	    if (IF_QFULL(&ipintrq))
	      {
		/* IOdebug( "buffer full, dropping input" ); */
		
		IF_DROP(&ipintrq);
		m_freem(m);
	      }
	    else
	      {
		/* IOdebug( "queing data" ); */
	      
		IF_ENQUEUE(&ipintrq, m);
		schednetisr(NETISR_IP);
	      }
	  }
}

int etioctl(
	    register struct ifnet *ifp,
	    int cmd,
	    caddr_t data )
{
	struct arpcom *ac = (struct arpcom *)ifp;
	register struct ifaddr *ifa = (struct ifaddr *)data;
	int s = splimp(), error = 0;

	switch (cmd) {

	case SIOCSIFADDR:
		if (ifa->ifa_addr.sa_family == AF_INET)
		{
			struct sockaddr_in *ia = (struct sockaddr_in *)&ifa->ifa_addr;
			ifp->if_flags |= IFF_UP;
			ac->ac_ipaddr.s_addr = ia->sin_addr.s_addr;
		}
		else
			error = EAFNOSUPPORT;
		break;

	default:
		error = EINVAL;
	}
	splx(s);
	return (error);
}


/* HELIOS specific ethernet device interface */

DCB *etherdcb;
NetDevInfo *EtherInfo = NULL;

void ether_infodone(DCB *dcb, NetDevReq *req)
{
	dcb=dcb,req=req;
}

void
ether_setinfo(NetInfo *info)
{
	NetInfoReq req;
	
	req.DevReq.Request = FG_SetInfo;
	req.DevReq.Action = ether_infodone;
	req.DevReq.SubDevice = 0;
	req.NetInfo = *info;
	
	Operate(etherdcb,&req);
	
}

void
ether_getinfo(NetInfo *info)
{
	NetInfoReq req;
	
	req.DevReq.Request = FG_GetInfo;
	req.DevReq.Action = ether_infodone;
	req.DevReq.SubDevice = 0;
	
	Operate(etherdcb,&req);
	
	*info = req.NetInfo;
}

void ether_init(u_char *addr)
{
	NetInfo ni;
	int i;
	char *edname = "ether.d";
	
	edname = (char *)RTOA(EtherInfo->Device);
	
	etherdcb = OpenDevice(edname,EtherInfo);
	
	if( etherdcb == NULL ) panic("no ethernet device: ether.d");

	/* To make this code universal, we first try to set the	*/
	/* ethernet address from *addr. We then read it out.	*/

	memcpy(ni.Addr,EtherInfo->Address,6);
	ni.Mode = EtherInfo->Mode;
	ni.State = EtherInfo->State;
	ni.Mask = NetInfo_Mask_Addr|NetInfo_Mask_Mode|NetInfo_Mask_State;	
	
	ether_setinfo(&ni);

	ether_getinfo(&ni);

	memcpy(addr,ni.Addr,6);


	for( i = 0; i < 5; i++ )
	{
		NetDevReq * req = (NetDevReq *) Malloc(sizeof(NetDevReq));
		byte *pkt;
		
		if( req == NULL ) break;
		
		pkt = (byte *) Malloc(ETHERPKTSIZE);
		
		if( pkt == NULL ) break;
		
		memset(pkt,0xcc,ETHERPKTSIZE);
		req->DevReq.Request = FG_Read;
		req->DevReq.Action = ether_rxdone;
		req->DevReq.SubDevice = 0;
		req->Size = ETHERPKTSIZE;
		req->Buf = pkt + ep_dst; /* see comment in ether_rxdone */
		
		Operate(etherdcb, req);
	}
}

void ether_txdone(DCB *dcb, NetDevReq *req)
{
	dcb=dcb,req=req;
	Free( ((byte *)req->Buf) - ep_dst );
	Free(req);
}

void ether_tx(byte *pkt, int size)
{
	NetDevReq *req;

	req = (NetDevReq *) Malloc(sizeof(NetDevReq));
	
	if( req == NULL ) return;
	
	req->DevReq.Request   = FG_Write;
	req->DevReq.Action    = ether_txdone;
	req->DevReq.SubDevice = 0;
	req->Size             = size < 64 ? 64 : size;
	req->Buf              = pkt + ep_dst;

	Operate(etherdcb, req);	
}

void ether_rxdone(DCB *dcb, NetDevReq *req)
{
	tokernel();

	/*
	 * The ethernet packet header is 14 bytes long.  In order
	 * to optimise performance by allowing the data field to
	 * start on a word boundary, the buffer passed to the device
	 * driver is advanced by 2 bytes (= ep_dst).  We adjust
	 * back here ((why - I don't know)).
	 */
	
	etinput( ((byte *)req->Buf) - ep_dst, (int) req->Actual );

	fromkernel();
	
/*	memset( req->Buf - 2 , 0xcc, ETHERPKTSIZE );	*/

	Operate(dcb,req);
}
