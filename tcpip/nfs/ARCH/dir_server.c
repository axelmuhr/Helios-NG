#include "nfs.h"

#include <protect.h>

static void marshal_entry(MCB *mcb, entry *ent, nfshandle *ph)
{
	word type = Type_File;
	word flags = 0;
	word matrix = DefFileMatrix;
	diropres *res;
	diropargs args;

#if 0	
	/* there seems to be no obvious alternative to doing a	*/
	/* lookup here to get the file's attributes		*/

	args.dir = ph->File;
	args.name = ent->name;
	
	Wait(&nfslock);
	while((res = nfsproc_lookup_2(&args, nfsclnt)) == NULL)
		clnt_perror(nfsclnt,"direntry lookup");
	Signal(&nfslock);
	
	if( res->status == NFS_OK )
	{
		/* @@@ init matrx and flags from attributes	*/
		switch( res->diropres_u.diropres.attributes.type )
		{
		case NFDIR:	type = Type_Directory; matrix = DefDirMatrix; break;
		default:
		case NFREG:	type = Type_File; matrix = DefFileMatrix; break;	
		}
	}
#endif
	MarshalData(mcb,4,&type);
	MarshalData(mcb,4,&flags);
	MarshalData(mcb,4,&matrix);
	MarshalData(mcb,32,ent->name);
}

extern void dir_server(MCB *mcb, nfshandle *h)
{
	Port reqport = mcb->MsgHdr.Reply;
	readdirargs args;
	readdirres _res, *res;
	entry *head, **tail;
	entry *ent;
	int nentries = 0;
	static int argscount = 512;	
	
	/* Note: many NFS servers seem unnecessarily sensitive to args.count */
	/* For example, HPUX will not allow it to be less than 512 while     */
	/* Ultrix and 4.2BSD don't like it to be more. SunOS will accept any */
	/* value, as it should.						     */
	
	if( DEBUG ) IOdebug("NFS dir_server %s",h->Path);
			
	args.dir = h->File;
	bzero(&args.cookie,NFS_COOKIESIZE);
	
	tail = &head;
	
	Wait(&nfslock);
	
	do
	{
		forever
		{
			args.count = argscount;
			if( DEBUG ) IOdebug("NFS readdir args.count %d",args.count);	
			res = nfsproc_readdir_2(&args, nfsclnt);
			if( res != NULL ) break;
			clnt_perror(nfsclnt,"readdir");
			argscount /= 2;
		}

		if( DEBUG ) IOdebug("NFS readdir %s stat %d",h->Path,res->status);
		
		if( res->status != NFS_OK ) break;

		ent = res->readdirres_u.reply.entries;
		if( ent )
		{
			*tail = ent;
			while( ent->nextentry ) nentries++, ent=ent->nextentry;
			nentries++;
			tail = &ent->nextentry;
			memcpy(&args.cookie,&ent->cookie,NFS_COOKIESIZE);
		}
		res->readdirres_u.reply.entries = NULL;
	} while( !res->readdirres_u.reply.eof );

	_res = *res;
	*tail = tail;
	
	Signal(&nfslock);
  
	for(;;)
	{
		word e;
		InitMCB(mcb,0,reqport,NullPort,0);
		
		e = GetMsg(mcb);
		
		if( e < 0 ) continue;

		mcb->MsgHdr.FnRc = SS_HardDisk;
		
		if( res == NULL ) goto bad;
				
		switch( e & FG_Mask )
		{
		case FG_Read:
		{
			ReadWrite *rw = (ReadWrite *)mcb->Control;
			word pos = rw->Pos;
			word size = rw->Size;
			word dirsize = nentries * sizeof(DirEntry);
			word dpos = 0;
			word tfr = 0;
			word seq = 0;
			Port reply = mcb->MsgHdr.Reply;
			
			if( pos == dirsize )
			{
				InitMCB(mcb,0,reply,NullPort,ReadRc_EOF);
				PutMsg(mcb);
				break;
			}
			
			if( pos < 0 || pos > dirsize ||
				pos % sizeof(DirEntry) != 0 )
			{
				ErrorMsg(mcb,EC_Error+EG_Parameter+1);
				break;
			}
			
			if( pos + size > dirsize ) size = dirsize - pos;
			
			InitMCB(mcb,MsgHdr_Flags_preserve,reply,NullPort,ReadRc_More);
			ent = head;
			
			while( size >= sizeof(DirEntry) )
			{
				if( dpos != pos )
				{
					dpos += sizeof(DirEntry);
					ent = ent->nextentry;
					continue;
				}
				
				marshal_entry(mcb,ent,h);
				
				ent = ent->nextentry;
				dpos += sizeof(DirEntry);
				tfr += sizeof(DirEntry);
				pos += sizeof(DirEntry);
				size -= sizeof(DirEntry);
				
				if( size < sizeof(DirEntry) ||
					IOCDataMax - tfr < sizeof(DirEntry) )
				{
					word e;
					if( size < sizeof(DirEntry) )
					{
						mcb->MsgHdr.Flags = 0;
						if( dpos == dirsize )
						    mcb->MsgHdr.FnRc = ReadRc_EOF|seq;
						else
						    mcb->MsgHdr.FnRc = ReadRc_EOD|seq;
					}

					e = PutMsg(mcb);
					seq += ReadRc_SeqInc;
					InitMCB(mcb,MsgHdr_Flags_preserve,
						reply,NullPort,ReadRc_More|seq);
					tfr = 0;
				}
			}

			if( tfr > 0 )
			{
IOdebug("finish off tfr %d",tfr);
				mcb->MsgHdr.Flags = 0;
				if( dpos == dirsize )
					mcb->MsgHdr.FnRc = ReadRc_EOF|seq;
				else
					mcb->MsgHdr.FnRc = ReadRc_EOD|seq;
				e = PutMsg(mcb);
			}
			
			break;
		}
		
		case FG_GetSize:
			InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,0);
			MarshalWord(mcb,nentries*sizeof(DirEntry));
			PutMsg(mcb);
			break;
			
		case FG_Close:
			goto done;
			
		default:
		bad:
			ErrorMsg(mcb,EC_Error|EG_WrongFn|EO_Directory);
			break;
		}
	}

done:	
	*tail = NULL;
	_res.readdirres_u.reply.entries = head;
	clnt_freeres(nfsclnt,xdr_readdirres,&_res);
}
