/*
--  ---------------------------------------------------------------------------
--
--      ISERVER - INMOS standard file server
--
--      udplink.h
--
--      The main body
--
--      Copyright (c) INMOS Ltd., 1989, 1990
--      All Rights Reserved.
--
-- ---------------------------------------------------------------------------
*/

#define rp_NULL         (unsigned char) 0
#define rp_DATA         (unsigned char) 1
#define rp_ACK          (unsigned char) 2
#define rp_RESET        (unsigned char) 3
#define rp_ANALYSE      (unsigned char) 4
#define rp_ERROR        (unsigned char) 5
#define rp_SYN          (unsigned char) 6
#define rp_NACK         (unsigned char) 7
#define rp_RST          (unsigned char) 8
#define rp_SUSP         (unsigned char) 9
#define rp_REQ          (unsigned char) 10
#define rp_CONT         (unsigned char) 11
#define rp_ACTIVE       (unsigned char) 12
#define rp_PEEK         (unsigned char) 13
#define rp_TIMEOUT      (unsigned char) 255

#define rp_DATA_HEADER 4

#define rp_MAX_DATA_SIZE 1400

/* timeout value send to transputer in prot_dgrams */
#define TRANS_TIMEOUT 5

#define HOSTLINK_PORT 5002

#define MAX_RETRIES 20
#define timeout_period 50 /* initial timeout period in milliseconds */

/* timeout for hearing nothing from transputer (in seconds) */
#define QUIETTIMEOUT    5

/* structure used to send/receive data to/from transputer */
struct data_dgram {
  unsigned char op;
  unsigned char seq;
  short int len;
  unsigned char data[rp_MAX_DATA_SIZE];
};

/* structure used to send/receive datagrams to/from transputer */
struct prot_dgram {
  unsigned char op;
  unsigned char seq;
  short int len;
  short int timeout;
};

/* structure used to receive rp_ERROR datagrams from transputer */
struct error_dgram {
  unsigned char op;
  unsigned char seq;
  unsigned char status;
  unsigned char dummy_padding;
}; 

/* structure used to send peek datagrams to the transputer */
struct peek_dgram {
  unsigned char op;
  unsigned char seq;
  short int numwords;
  long address;
};
