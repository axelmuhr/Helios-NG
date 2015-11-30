#define MAXETHERPKT		1514
#define MAX_PKT_TABLE		8		/* size of buffer table */

#define	MIN_PKT_INT		0x60		/* range of possible software */
#define	MAX_PKT_INT		0x80		/* interrupts */
#define DRVR_SIG		"PKT DRVR"	/* packet driver "signature" */
#define NUM_HANDLES		2		/* ARP, IP */
#define HOST_PKT_INT		"PACKET_INT"	/* host.con entry */

/*
-- 
-- Ethertype definitions
-- IP				0x0800	
-- Addr. resolution (ARP)	0x0806
-- NB: require byte network ordering !
-- 
*/
#define PKT_TYPE_LEN		2
#define	ETHERTYPE_IP		0x0008		/* IP protocol */
#define ETHERTYPE_ARP		0x0608		/* Addr. resolution protocol */

#ifdef ETHER_DEBUG
#define IFACE_NAME_LEN		64
#endif /* ETHER_DEBUG */

#define CANT_LOCATE		"Failed to locate packet driver"

/*
-- Packet driver operations
*/
#define	DRIVER_INFO		1
#define	ACCESS_TYPE		2
#define	RELEASE_TYPE		3
#define	SEND_PKT		4
#define	TERMINATE		5
#define	GET_ADDRESS		6
#define	RESET_INTERFACE		7
#define GET_PARAMETERS		10
#define AS_SEND_PKT		11
#define	SET_RCV_MODE		20
#define	GET_RCV_MODE		21
#define	SET_MULTICAST_LIST	22
#define	GET_MULTICAST_LIST	23
#define	GET_STATISTICS		24
#define SET_ADDRESS		25

#define NUM_ERR_CODES		16


