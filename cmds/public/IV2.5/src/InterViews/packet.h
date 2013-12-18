/*
 * Format of object packet header.
 */

#ifndef packet_h

class Packet {
public:
    unsigned tag;		/* target object tag */
    unsigned op : 8;		/* operation on object */
    unsigned count : 8;		/* repeat operation this many times */
    unsigned extend : 1;	/* next operation is on same object */
    unsigned length : 15;	/* size in words of operands */
    /* operands follow */
};

#endif
