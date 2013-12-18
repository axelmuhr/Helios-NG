/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- port.c								--
--                                                                      --
--	Kernel port handling functions.					--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: port1.c,v 1.18 1993/10/04 15:05:59 paul Exp $ */


#define __in_port 1	/* flag that we are in this module */

#include "kernel.h"
#include <message.h>
#include <root.h>

static void ExtendTable(void);
static void TimeoutHandler(void);

/*--------------------------------------------------------
-- PortInit						--
--							--
-- Initalise port table.				--
--							--
--------------------------------------------------------*/

void PortInit(Config *config)
{
	int i;
	PTE **ptab;	
	RootStruct *root = GetRoot();

	root->Time = config->Date;
	root->Errors = 0;
			
	root->PTFreeq = MinInt;
	
	root->PTSize = 32;		/* base table size */
	
	ptab = (PTE **)LowAllocate(32, root->FreePool,&root->SysPool);
	root->PortTable = ptab;
	
	for( i = 0; i < 7; i++ ) ptab[i] = (PTE *)MinInt;
	ptab[7] = NULL;
	
	ExtendTable();
	ExtendTable();
	ExtendTable();	

	NewWorker(TimeoutHandler); 

	config = config;
}

/*--------------------------------------------------------
-- _NewPort						--
--							--
-- Internal port allocator.				--
-- MUST be called at high priority.			--
--							--
--------------------------------------------------------*/

Port _NewPort(void)
{
	PTE *pte;
	Port port;
	RootStruct *root = GetRoot();
	
	if( root->PTFreeq == MinInt ) ExtendTable();
	
	port = root->PTFreeq;
	
	pte = GetPTE(port,root);

	root->PTFreeq = pte->Owner;

	pte->Type = T_Local;
	pte->Link = 0;
	pte->Age = 1;
	pte->Owner = NULL;
	pte->TxId = NULL;
	pte->RxId = NULL;

	port |= ((uword)pte->Cycle << Port_Cycle_shift) | MinInt;

	return port;
}

/*--------------------------------------------------------
-- _FreePort						--
--							--
-- Internal port deallocator.				--
-- MUST be called at high priority.			--
--							--
--------------------------------------------------------*/

Code _FreePort(Port port)
{
	PTE *pte;
	RootStruct *root = GetRoot();
	
	pte = GetPTE(port,root);

	if( NotSameCycle_(port,pte) ) return Err_BadPort;
	
	if(pte->Type == T_Free || pte->Type == T_Permanent) return Err_BadPort;
		
	pte->Type = T_Free;
	pte->Cycle++;
	pte->Owner = root->PTFreeq;
	pte->TxId = NULL;
	pte->RxId = NULL;

	root->PTFreeq = port&Port_Index_mask;

	return Err_Null;
}

/*--------------------------------------------------------
-- _GetPortInfo						--
--							--
-- Return some Info on a Port.				--
--							--
--------------------------------------------------------*/

Code _GetPortInfo(Port port, PortInfo *info)
{
	RootStruct *root = GetRoot();
	PTE *pte = GetPTE(port,root);
	
	if( NotSameCycle_(port,pte) ) return Err_BadPort;
	
	/* The following copy depends on a PortInfo being	*/
	/* roughly equivalent to the first 3 words of a PTE	*/
	
	*info = *(PortInfo *)pte;	/* structure copy */
	
	/* for efficiency reasons from V1.2.1 the Link and Cycle */
	/* fields of a PTE have been exchanged. To preserve the	 */
	/* external interface of GetPortInfo, we must make the	 */
	/* following assignments.				 */
	
	info->Link = pte->Link;
	info->Cycle = pte->Cycle;
	
	return Err_Null;
}

/*--------------------------------------------------------
-- ClearPorts						--
--							--
-- Clear port table of any ports for the given link.	--
--							--
--------------------------------------------------------*/

void ClearPorts(LinkInfo *link)
{
	RootStruct *root = GetRoot();
	word linkid = link->Id;
	word btindex;	

#if 0	
	Id *id, *prev;

	/* First clear link TxQueue	*/
	for( id=link->TxQueue,prev=NULL; id!=NULL; id=id->next)
	{
		id->rc = Err_BadRoute;
		dq(&link->TxQueue,prev,id);
		Resume(id->state);
	}
#endif
	/* Now scan port tables looking for surrogates for this link */
	for( btindex = 0; root->PortTable[btindex] != NULL; btindex++ )
	{
		word ptindex;
		PTE *ptab = root->PortTable[btindex];
	
		if( ptab == (PTE *)MinInt ) continue;

		for( ptindex = 0 ; ptindex < 64 ; ptindex++ )
		{
			PTE *pte = &ptab[ptindex];
			
			if( 	pte->Type >= T_Surrogate &&
				pte->Type != T_Permanent )
			{
				Port port;
				if( pte->Link == linkid )
				{
					/* For a surrogate pointing into the */
					/* link, just free it.		     */
					port =	MinInt | ((uword)pte->Cycle \
						<< Port_Cycle_shift) \
						| (btindex << 8) | ptindex;
					_FreePort(port);
				}
				elif (	pte->Type == T_Trail &&
					(word)pte->RxId == linkid )
				{
					/* For a port coming from the link */
					/* send an exception down it	   */
					port =	MinInt | Port_Flags_Tx |
						((uword)pte->Cycle << Port_Cycle_shift) |
						(btindex << 8) | ptindex;
					Exception(Err_BadRoute,port);
				}
			}
		}
		Yield();	/* keep response times up */
	}
}

/*--------------------------------------------------------
-- ExtendTable						--
--							--
-- Extend port table.					--
-- MUST be called at high priority.			--
--							--
--------------------------------------------------------*/

static void ExtendTable(void)
{
	PTE **basetab;
	RootStruct *root = GetRoot();
	word btindex;
	word ptsize;

	basetab = root->PortTable;
	ptsize = root->PTSize;

	/* look for a free slot in the table */
	for( btindex = 0; basetab[btindex] != (PTE *)MinInt ; btindex++ )
	{
		/* see if we have reached the end of the table.	*/
		if( basetab[btindex] == NULL )
		{
			word newsize = ptsize * 2;
			PTE **oldtab = basetab;
			word i;
			
			/* allocate a new table twice the size of old one */
			basetab = (PTE **)LowAllocate(newsize,root->FreePool,&root->SysPool);

			/* clear it out */
			for( i = 0; i < newsize/sizeof(PTE *) ; i++ )
				basetab[i] = (PTE *)MinInt;

			basetab[(newsize/sizeof(PTE *))-1] = NULL;
			
			/* copy old table over */
			MoveBlock(basetab,oldtab,ptsize-sizeof(PTE *));
			
			/* replace in root and free old table */
			root->PortTable = basetab;
			root->PTSize = newsize;
			_Free((Memory *)oldtab,root->FreePool);
			
			/* btindex now points to a free slot */
			break;
		}

	}

	/* here btindex is the index of a free slot in the base	*/
	/* port table.						*/

	{
		PTE *newtab = (PTE *)LowAllocate(PTBlockSize,root->FreePool,&root->SysPool);
		int ptindex;
		
		basetab[btindex] = newtab;
		
		/* shift basetable index up to correct pos */
		btindex = btindex << 8;

		/* add new ports to port table */
		for(	ptindex = 0 ;
			ptindex < (int)(PTBlockSize / sizeof(PTE)) ;
			ptindex++ )
		{
			PTE *pte = &newtab[ptindex];
			*(word *)pte = 0x00010000;	/* zero first 4 fields */
							/* except cycle		*/
			pte->TxId = NULL;		
			pte->RxId = NULL;
			pte->Owner = root->PTFreeq;
			/* make double index */
			root->PTFreeq = btindex | ptindex;
		}
	}
}

/*--------------------------------------------------------
-- TimeoutHandler					--
--							--
-- Kernel process which checks ports and links for	--
-- timeouts every PortTimeout interval.			--
--							--
--------------------------------------------------------*/

static void TimeoutHandler(void)
{
	int gccount = 0;
	RootStruct *root = GetRoot();
	word now = Timer();
	char *gcc = (char *)&(root->GCControl);

	/* Port Garbage Collector permanently off for now since it has	*/
	/* some unidentifiable problems, particularly with long-lived	*/
	/* task forces.							*/
	
	gcc[0] = 0;
	gcc[1] = GCTicks;
	gcc[2] = GCAge;
	
	forever
	{
	    word ptindex;

	    if( gccount >= gcc[1] ) gccount = 0;
	    
	    /* wait for the timeout period, less execution time of loop */
	    Sleep(DiffTimes(OneSec,DiffTimes(Timer(),now)));

	    now = Timer();

	    root->Timer = now;
	    
	    root->Time++;

#ifdef __TRAN
	    if( !testerr_() ) root->Errors++;
#endif
	    	    
	    gccount++;

	    /* loop over port tables */
	    for( ptindex = 0; root->PortTable[ptindex] != NULL; ptindex++ )
	    {
		PTE *pte;
		PTE *ptupb;
		Id *id;
		Id *prev;

		pte = root->PortTable[ptindex];
		if( pte == (PTE *)MinInt ) continue;

		ptupb = pte+(PTBlockSize/sizeof(PTE));


		for(; pte != ptupb; pte++ )
		{
			/* ignore free ports */
			if( pte->Type == T_Free ) continue;
			
			/* Inactive ports are candidates for garbage	*/
			/* collection. Increment the Age field and if 	*/
			/* it reaches GCAge, free the port		*/
			if( gccount >= gcc[1]		&& 
			    ( (pte->Type == T_Local	&&
			       pte->TxId == NULL	&& 
			       pte->RxId == NULL
		 	      )				||
			      (pte->Type > T_Local)
			    )
			  )
			{
				/* Only garbage collect if gcc[0] is	*/
				/* TRUE.				*/
				if( gcc[0] && pte->Age >= gcc[2] )
				{
				    Port port = MinInt |
					((uword)pte->Cycle << Port_Cycle_shift);
				    port |= ((word)pte -
					(word)(root->PortTable[ptindex]))
					/ sizeof(PTE);
				    port |= ptindex << 8;
				    __FreePort(port,TRUE,Err_Abort);
				}
				else pte->Age++;
				continue;
			}
						
			/* no processes queued on surrogates	*/
			if( pte->Type != T_Local ) continue;
			
			/* here we have an active port		*/

			/* start by checking the transmitters	*/
			for(id = pte->TxId,prev=NULL; id != NULL; id=id->next)
			{
				CheckState(id->state);
				/* if timeout is -1, dont time it out	*/
				if( Forever(id->endtime) || !After(now,id->endtime) )
				{ prev = id; continue; }

				id->rc = Err_Timeout;
				dq(&pte->TxId,prev,id);
				Resume(id->state);
			}

			/* now do the same for receivers	*/
			for(id = pte->RxId,prev=NULL; id != NULL; id=id->next)
			{
				word idrc = id->rc;
				CheckState(id->state);
				/* if timeout is -1, dont time it out	*/
				if( Forever(id->endtime) || !After(now,id->endtime) ) 
				{ prev = id; continue; }

				id->rc = Err_Timeout;
				dq(&pte->RxId,prev,id);				

				if( idrc == MultiWaiting )
				{
					SaveState **statep = (SaveState **)id->state;

					if( *statep != NULL )
					{
						Resume(*statep);
						*statep = NULL;
					}
				}
				else Resume(id->state);
			}
			
		} /* sub table */
		
		Yield();	/* give rest of system a chance	*/
		
	    } /* ptindex */

#ifdef LINKIO
	    {
	    	LinkInfo **links = root->Links;
	    	int i;

	    	for( i = 0; links[i] != NULL ; i++ )
	    	{
	    		LinkInfo *l = links[i];
	    		Id *lu = l->TxUser;
			Id *id, *prev;

	    		if( lu != NULL && lu->endtime != -1 && After(now,lu->endtime))
	    		{
				SaveState *s = AbortLinkTx(l);
	    			if( !NullStateP(s) )
	    			{
	    				CheckState(s);
	    				lu->rc = Err_Timeout;
	    				Resume(s);
	    			}
	    			lu->endtime = -1;
	    		}
	    		lu = l->RxUser;
	    		if( lu != NULL && lu->endtime != -1 && After(now,lu->endtime))
	    		{
#ifndef __TRAN
				SaveState *s = AbortLinkRx(l);
#else
				/* BLV - there are problems with the behaviour	*/
				/* of resetch_. This is an attempted fix.	*/
				SaveState *s = P_NullState;
				Channel   *channel = l->RxChan;
				if (*channel != MinInt)
				 s = (SaveState *) resetch_(channel);
#endif
	    			if( !NullStateP(s) )
	    			{
	    				CheckState(s);
	    				lu->rc = Err_Timeout;
	    				Resume(s);
	    			}
	    			lu->endtime = -1;	    			
	    		}
			/* Now check link TxQueue	*/
			for( id=l->TxQueue,prev=NULL; id!=NULL; id=id->next)
			{
				CheckState(id->state);
				/* if timeout is -1, dont time it out	*/
				if( Forever(id->endtime) || !After(now,id->endtime) )
				{ prev = id; continue; }

				id->rc = Err_Timeout;
				dq(&l->TxQueue,prev,id);
				Resume(id->state);
			}
	    	}
	    }
#if defined(__C40) && defined(ALLOCDMA)
	/* Check the 'C40 DMA engine request queue. */
	/* If a timeout is exceeded, then dequeue the request and return */
	/* it to the caller with a timeout return value */
	    {
		DMAReq *DMARequest = root->DMAReqQhead;
		DMAReq *prev = (DMAReq *)&root->DMAReqQhead;

		while (DMARequest != NULL) {
			if (DMARequest->endtime != -1 && \
			    After(now,DMARequest->endtime)) {

				/* remove request from Q */
				if ((prev->next = DMARequest->next) == NULL)
					root->DMAReqQtail = prev;

				/* note timeout abort */
				DMARequest->rc = -1;

				/* Resume caller of AllocDMA with timeout*/
				Resume(DMARequest->state);
			}
			prev = DMARequest;
			DMARequest = DMARequest->next;
		}
	    }
# endif
#endif
	} /* end of main loop */
}

/*--------------------------------------------------------
-- External interface.					--
--							--
--							--
--------------------------------------------------------*/

Port NewPort(void)
{
	RootStruct *root = GetRoot();
	Port port = (Port)System((WordFnPtr)_NewPort);
	PTE *pte = GetPTE( port, root );
	pte->Owner = (word)_Task_;
	return port;
}

Code FreePort(Port port)
{
#ifdef __TRAN
  RootStruct *	root = GetRoot();		/* would you believe that Err_Abort below is a macro that silently uses a variable called 'root'  - I ask you, who coded this ? */
#endif
  
	if( port == NullPort ) return Err_Null;
	
	return (Code)System(__FreePort,port,FALSE,Err_Abort);
}

Code __FreePort(Port port, bool kernel, Code rc)
{
	RootStruct *root = GetRoot();
	PTE *pte = GetPTE( port, root );
	
	/* moan about free ports */
	if( pte->Type == T_Free ) return Err_BadPort;
	
	/* If the port is a surrogate, dispose of it by sending an	*/
	/* Err_Abort exception message down it. This will dismantle	*/
	/* the trail as it goes.					*/
	if( pte->Type != T_Local )
	{
		/* A permanent port is never freed.	*/
		if( pte->Type == T_Permanent ) return Err_Null;

		Exception(rc,port);
		
		return Err_Null;
	}
	
	/* For local ports, we abort all waiters 	*/
	_AbortPort(port,rc);
	
	/* If the port is not mine I cannot free it	*/
	if( !kernel && pte->Owner != (word)_Task_) return Err_Null;
	
	return _FreePort(port);
}

Code GetPortInfo(Port port, PortInfo *info)
{
	return (Code)System(_GetPortInfo,port,info);
}

/* -- End of port.c */
