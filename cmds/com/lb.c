/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- lb.c									--
--                                                                      --
--	New load balancer						--
--                                                                      --
--	Author:  BLV 15.8.89						--
--                                                                      --
------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <helios.h>
#include <sem.h>
#include <nonansi.h>
#include <posix.h>
#include <lb.h>

/**
*** various utility macros
**/
#define	eq	==
#define ne 	!=

#define Terminate 0x1234
#define Continue  0x4321

/**
*** macros to access the file descriptors
***/
#define from_master 0      /* stdin  */
#define to_master   1      /* stdout */
#define from_slave(id)   (id + id + 4)
#define to_slave(id)     (id + id + 5)

/**
*** various global variables
**/
int		number_of_slaves;

/**
*** The main input buffer
**/
PACKET		**packet_table;
Semaphore	table_free;
Semaphore	table_used;
Semaphore	table_lock;
int		table_head;
int		table_tail;

/**
*** Control access to the output stream to the master task
**/
Semaphore	master_lock;

/**
*** Control broadcast packets
**/
Semaphore	broadcast_master;
Semaphore	broadcast_slave;

/**
*** function prototypes
**/
void	read_master		(void);
void	interact_with_slave	(int);
int 	control_packet		(LB_HEADER *);
void	broadcast_packet	(PACKET *);
int	full_read		(int, BYTE *, int);

/**
*** Main() : work out the number of slave tasks using the first argument
***          passed to the program. Initialise the packet table used to
***          buffer packets coming from the master, and the other global
***          variables. Then Fork() off processes to handle all the
***          slave tasks, and enter the main loop. An error condition is
***          handled by sending a terminate to all the slave tasks, and
***          by reading a single byte from the master - this should give
***          a SIGINT signal on the master side, which will terminate the
***          task force.
**/

int main(int argc, char **argv)
{ int i; 

  if (argc ne 2)
   { fprintf(stderr, "lb : syntax lb <no of workers>\n");
     exit(1);
   }	 

  number_of_slaves = atoi(*(++argv));
  
  packet_table = (PACKET **) malloc(number_of_slaves * sizeof(PACKET *));
  if (packet_table eq Null(PACKET *))
   { fprintf(stderr, "lb : insufficient memory\n");
     goto fail;
   }
   
  InitSemaphore(&table_free, number_of_slaves);
  InitSemaphore(&table_used, 0);
  InitSemaphore(&table_lock, 1);
  InitSemaphore(&master_lock, 1);
  InitSemaphore(&broadcast_master, 0);
  InitSemaphore(&broadcast_slave, 0);
  
  table_head   = 0;
  table_tail   = 0;

  for (i = 0; i < number_of_slaves; i++)
   unless (Fork(2000, &interact_with_slave, 4, i))
    goto fail;
    
  read_master();
  exit(0);

fail:

  { LB_HEADER header;
  
    header.size		= 0;
    header.control	= LB_MASTER + Fn_Terminate;

    for (i = 0; i < number_of_slaves; i++)
     (void) write(to_slave(i), (BYTE *) &header, sizeof(LB_HEADER));
     
    (void) read(from_master, (BYTE *) &i, 1);
    
    exit(1);
  }
}

/**
*** This is the main  routine, which reads in packets from the master task
*** and buffers them in the table. First the packet header is read in. If
*** this is a control packet of some sort, it is handled appropriately
*** which may mean that the load balancer should terminate. Otherwise it
*** is a data packet, so a suitable buffer is allocated and the remainder
*** is read in. If the packet is intended to be broadcasted a suitable
*** handler routine is called, otherwise it is put into the table
*** synchronising with the other processes. The call to broadcast_packet()
*** will not return until the broadcast has been completed. Please note that
*** this version of the load balancer makes no attempt to recover from errors
*** such as running out of memory.
**/

void read_master(void)
{ LB_HEADER header;
  int       result;
  PACKET    *pkt;
  
  forever
   { 
     result = full_read(from_master, (BYTE *) &header, sizeof(LB_HEADER));
     if (result < 0)      /* pipe from master has been broken */
      { fprintf(stderr, "lb : failed to read header from master\n");
        exit(1);
      }   

     if (header.control & LB_MASTER)
      { int result = control_packet(&header);
        if (result eq Terminate)
         break;
        else
         continue;
      }

     pkt = (PACKET *) malloc(sizeof(PACKET) + (int)header.size);
     if (pkt eq Null(PACKET))
      { fprintf(stderr, "lb : insufficient memory");
        exit(1);
      }
      
     pkt->header.size    = header.size;
     pkt->header.control = header.control;
     if (full_read(from_master, &(pkt->data[0]), (int)header.size) < 0)
      { fprintf(stderr, "lb : failed to read data from master\n");
        exit(1);
      }
      
     if (header.control & LB_BROADCAST)
      { broadcast_packet(pkt);
        free(pkt);      
        continue;
      }

     Wait(&table_free);   /* for a free slot in the table */
      
     packet_table[table_head] = pkt;
     table_head = (table_head + 1) % number_of_slaves;
     
     Signal(&table_used);   /* let a slave read the packet */
   }
}

/**
*** For a farm with say 5 slave tasks, there will be one process in the
*** load balancer associated with each slave. These 5 processes will all
*** execute the following code in parallel. First each process waits for
*** a packet to be sent by the master task, which will be read in and
*** buffered by the main loop above. If this packet is a broadcast packet
*** special action is taken, which is described later. Otherwise the
*** packet is removed from the table, which will free a slot in the table,
*** and the packet is sent to the appropriate slave. The slave task should
*** now be busy calculating, and the process is suspended until the
*** slave task has finished calculating and sends back a reply packet.
*** A buffer must be allocated to read in the whole packet, and then the
*** reply packet can be sent to the master. Ofcourse several slaves may
*** finish work at about the same time, so several reply packets may be
*** ready to be sent to the master task at the same time. Hence it is
*** necessary to lock acccess to the master via a semaphore.
**/

void interact_with_slave(int id)
{ PACKET    *pkt;
  LB_HEADER header;
  	
  forever
   { Wait(&table_used);   /* for a new packet from the master */
   
     Wait(&table_lock);
     pkt = packet_table[table_tail];
     
     if (pkt->header.control & LB_BROADCAST)
      { Signal(&table_lock);		/* Next packet is a broadcast,  */
        Signal(&broadcast_master);	/* let broadcast_packet() deal  */
                                        /* with it */
        Wait(&broadcast_slave);		/* Broadcast has now finished */
        continue;
      }
      
     table_tail = (table_tail + 1) % number_of_slaves;
     Signal(&table_lock);
     
     Signal(&table_free);  /* can accept another packet from the master */
     
     if (write(to_slave(id), (BYTE *) pkt, 
               sizeof(LB_HEADER) + (int)(pkt->header.size))
          < sizeof(LB_HEADER) + pkt->header.size)
      { fprintf(stderr, "lb : error writing to slave %d\n", id);
        exit(1);
      }

     if (full_read(from_slave(id), (BYTE *) &header, sizeof(LB_HEADER)) < 0)
      { fprintf(stderr, "lb : error reading header from slave %d\n", id);
        exit(1);
      }
      
     if (header.size > pkt->header.size)   /* have to get another buffer */
      { free(pkt);
        pkt = (PACKET *) malloc(sizeof(LB_HEADER) + (int)header.size);
        if (pkt eq Null(PACKET))
         { fprintf(stderr, "lb : insufficient memory\n");
           exit(1);
         }
      }

     pkt->header.size    = header.size;
     pkt->header.control = header.control;
     if (full_read(from_slave(id), &(pkt->data[0]), (int)header.size) < 0)
      { fprintf(stderr, "lb : error reading data from slave %d\n", id);
        exit(1);
      }

     Wait(&master_lock);
           
     if (write(to_master, (BYTE *) pkt,
               sizeof(LB_HEADER) + (int)header.size)
             < sizeof(LB_HEADER) + header.size)
      { fprintf(stderr, "lb : error replying to master from slave %d\n", id);
        exit(1);
      }

     Signal(&master_lock);

     free(pkt);
   }
}

/**
*** This routine deals with control packets. This version of the load
*** balancer only understands the Terminate control packet, which must
*** not have any data. A Terminate packet can be handled very easily
*** by treating it as an ordinary broadcast, and then returning Terminate
*** so that the main loop exits.
**/

int control_packet(LB_HEADER *header)
{  

  if (header->size ne 0)
   { fprintf(stderr, "lb : error, control packet received with data\n");
     exit(1);
   }
   
  if (header->control & Fn_Terminate)
   { PACKET pkt;
     pkt.header.size    = 0;
     pkt.header.control = LB_MASTER + LB_BROADCAST + Fn_Terminate;
     broadcast_packet(&pkt);
     return(Terminate);
   }
   
  fprintf(stderr, "lb : unknown control request %lx received\n",
          header->control);
  return(Terminate);
}

/**
*** The nasty thing about broadcasting packets is that every slave task
*** must be able before the broadcast can be sent. To synchronise with
*** all the interact_with_slave() processes, broadcast_packet() puts a
*** special packet into the table and waits for all the processes to
*** notice this packet - this involves a semaphore broadcast_master.
*** The broadcast packet can now be sent safely to all the slave tasks.
*** No reply is expected, so the interact_with_slaves() processes can
*** be woken up again using another semaphore. broadcast_packet() now
*** returns to the main loop, read_from_master(), which will read the
*** next packet from the master task.
**/

void broadcast_packet(PACKET *pkt)
{ int i;

  Wait(&table_free);   /* for a free slot in the table */
     
  packet_table[table_head] = pkt;

  for (i = 0; i < number_of_slaves; i++)
   Signal(&table_used);   /* let all the slaves read the packet */

		/* wait till all the slaves have read the packet */
  for (i = 0; i < number_of_slaves; i++)
   Wait(&broadcast_master);

         /* now send the packet to all the slaves */      
  for (i = 0; i < number_of_slaves; i++)
   if (write(to_slave(i), (BYTE *) pkt,
             sizeof(LB_HEADER) + (int)(pkt->header.size))
           < sizeof(LB_HEADER) + pkt->header.size)
    { fprintf(stderr, "lb : error broadcasting packet to slave %d\n", i);
      exit(1);
    }

         /* now release the slaves */
  for (i = 0; i < number_of_slaves; i++)
   Signal(&broadcast_slave);
  
  Signal(&table_free);
}

/**
*** This is a little utility routine to avoid problems with pipe reads.
*** Suppose the other side of the pipe writes 10 bytes of data as two
*** lots of 5 bytes. An ordinary posix read for 10 bytes would return
*** just the first 5, which is likely to upset things. This routine
*** continues reading from the pipe until all of the expected data has
*** arrived.
**/

int full_read(int stream, BYTE *buff, int amount)
{ int result;
  while (amount > 0)
   { result = read(stream, buff, amount);
     if (result <= 0) return(-1);
     amount -= result;
     buff = &(buff[result]);
   }
   
  return(0);
}


