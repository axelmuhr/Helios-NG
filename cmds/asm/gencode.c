/****************************************************************/
/* File: gencode.c                                              */
/*                                                              */
/* Generate code into codebuf                                   */
/*                                                              */
/* Author: NHG 13-May-87                                        */
/****************************************************************/
/* $Id: gencode.c,v 1.6 1994/08/09 16:43:25 al Exp $ */

#include "asm.h"

#define trace if(traceflags&db_gencode)_trace

PUBLIC UBYTE *codevec;
PUBLIC WORD codepos;
PRIVATE WORD curpc;
PRIVATE WORD setsize;

#ifdef __DOS386

PRIVATE WORD guesssize(WORD vtype, WORD valuearg);
PUBLIC WORD asize(WORD value);

#else /* !__DOS386 */

PRIVATE WORD guesssize(ellipsis);
PUBLIC WORD asize(ellipsis);

#endif /* __DOS386 */

PRIVATE VMRef lastinit;


/* the following macro duplicates the newcode function in mem.c for	*/
/* speed. Make sure it is kept in step!!!!				*/
#define newcode(xtype,xsize,xvtype,xloc,xvalue)				\
{									\
	Code *_p;							\
	VMRef _v;							\
	extern VMRef codeseg;						\
	if( VMTable[page_(codeseg)].left < sizeof(Code)*2 ) codeptr();	\
	_v = VMalloc(sizeof(Code),codeseg);				\
	_p = VMAddr(Code,_v);						\
        _p->type = (UBYTE)(xtype);					\
        _p->size = (UBYTE)(xsize);					\
        _p->vtype = (UBYTE)(xvtype);					\
        _p->flags = codeflags; codeflags = 0;				\
        _p->loc = (WORD)(xloc);						\
        _p->value = (xvalue);						\
	codesize += sizeof(Code);					\
        VMDirty(_v);							\
}

PUBLIC void initcode()
{
        codevec = alloc(256L);
        codepos = 0;
        lastinit = NullVMRef;
	setsize = -1;
	curpc = 0;
	
}

PUBLIC void gencode(code,size)
UBYTE *code;
WORD size;
{
   int i;
   for( i = 0 ; i < size; i++ ) genbyte((WORD)code[i]);
}

PUBLIC void genbyte(value)
UWORD value;
{
        trace("genbyte %2x",value);
        if ( codepos >= 255 ) copycode();
        codevec[codepos] = value;
	codepos++;
}

PUBLIC void gendirect(op,vtype,value)
UWORD op,vtype;
WORD value;
{
        if( vtype == s_number ) encode((WORD)op,value,genbyte);
        else {
		WORD size;
		Value vv; vv.w = value;
		if( setsize != -1 )
		{
			size = setsize;
			setsize = -1;
			codeflags |= codeflags_fixed;
		}
		else size = fastopt ? guesssize(vtype,value) : 1 ;
                copycode();
                newcode((WORD)op>>4,size,vtype,curpc,vv);
                curpc += size;
        }
}

PUBLIC void copycode()
{
	Value vv;
        if( codepos == 0 ) return;
        if( codepos <= 4 ) 
	{
		int i;
		for( i = 0; i < codepos ; i++ ) vv.b[i] = codevec[i];
		newcode((WORD)s_literal,codepos,0l,curpc,vv);
	}
        else {
                VMRef vmr = VMNew(codepos);
                UBYTE *v = VMAddr(UBYTE,vmr);
		vv.v = vmr;
                codesize += codepos;

		memcpy(v,codevec,codepos);
		
                newcode((WORD)s_code,codepos,0l,curpc,vv);
        }
        curpc += codepos;
        codepos = 0;
}

PUBLIC VMRef deflabel()
{
        copycode();     /* empty code buffer */
        return codeptr();
}

PUBLIC void genglobal(label)
VMRef label;
{
	Symbol *l = VMAddr(Symbol,label);
        trace("GLOBAL %s",l->name);
	if( defopt == TRUE )
	{
		fprintf(deffd,"\tglobal %s\n",l->name);
	}
	if( preasm )
	{
		Value vv; vv.v = label;
        	copycode();
        	newcode((WORD)s_global,0l,(WORD)s_label,curpc,vv);
	}
}

PUBLIC void genref(label)
VMRef label;
{
	Symbol *l = VMAddr(Symbol,label);
        trace("REF %s",l->name);
	if( defopt == TRUE )
	{
		fprintf(deffd,"\tref %s\n",l->name);
	}
	if( preasm )
	{
		Value vv; vv.v = label;
        	copycode();
        	newcode((WORD)s_ref,0l,(WORD)s_label,curpc,vv);
	}
}

PUBLIC void genlabdef(label)
VMRef label;
{
	Symbol *l = VMAddr(Symbol,label);
        trace("LABDEF %s",l->name);
	if( preasm )
	{
		Value vv; vv.v = label;
        	copycode();
        	newcode((WORD)s_labdef,0l,0l,curpc,vv);
	}
}

PUBLIC void gendata(label,size)
VMRef label;
WORD size;
{
	Symbol *l = VMAddr(Symbol,label);
	Value vv; vv.v = label;
        trace("DATA %s %ld",l->name,size);
	l->def.w = size;
	VMDirty(label);
	if( defopt == TRUE )
	{
		fprintf(deffd,"\tdata %s %ld\n",l->name,size);
	}
        copycode();
        newcode((WORD)s_data,0l,(WORD)s_datasymb,curpc,vv);
}

PUBLIC void gencommon(label,size)
VMRef label;
WORD size;
{
	Symbol *l = VMAddr(Symbol,label);
	Value vv; vv.v = label;
        trace("COMMON %s %ld",l->name,size);
	l->def.w = max(l->def.w,size);
	VMDirty(label);
	if( defopt == TRUE )
	{
		fprintf(deffd,"\tcommon %s %ld\n",l->name,size);
	}
        copycode();
        newcode((WORD)s_common,0l,(WORD)s_commsymb,curpc,vv);
}

PUBLIC void geninit()
{
        VMRef ct;
        Value vv; vv.v = NullVMRef;
        copycode();
        ct = codeptr();
        newcode((WORD)s_init,4l,0l,curpc,vv);
        curpc += 4;     
        trace("INIT: %x %x",ct,lastinit);
        if( !NullRef(lastinit) ) 
        { 
        	Code *c = VMAddr(Code,lastinit);
        	c->value.v = ct;
        	VMDirty(lastinit);
        }
        lastinit = ct;
}

extern int filemod;

PUBLIC void genmodule(mod)
WORD mod;
{
	Value v;
	
	trace("MODULE %d",mod);
	
	genend();	/* finish off last module */
	
	curpc = 0;	/* reset pc for new module to 0 */
	
        genalign();	/* ensure module is aligned */

	filemod++;
	
	v.v = startmodule(mod);
	        
	newcode((WORD)s_module,0l,(WORD)s_number,curpc,v);

	newfile();	/* set file mark */
	
        lastinit = NullVMRef;
}

PUBLIC void gensize(size)
int size;
{
	Value vv; vv.w = size;
	setsize = size;
	if( preasm ) newcode((WORD)s_size,0l,(WORD)s_number,curpc,vv);
}

PUBLIC void newfile()
{
	char *fname = (char *)alloc((WORD)(strlen(infile)+1));
	Value vv; vv.c = fname;
	strcpy(fname,infile);
	curfile = fname;
	copycode();
	newcode((WORD)s_newfile,0l,0l,curpc,vv);
	if( defopt == TRUE )
	{
		fprintf(deffd,"-- %s\n",fname);
	}
}

PUBLIC void genalign()
{
        WORD size;
        Value vv; vv.w = 0;
        copycode();
        size = asize(curpc);
        newcode((WORD)s_align,size,0l,curpc,vv);
        curpc += size;
}

PUBLIC void genword(vtype,value)
WORD vtype;
WORD value;
{
	trace("WORD %x %x",vtype,value);

        if( vtype == s_number )
        {
                int i;
                for( i = 0 ; i < 32 ; i+=8 ) genbyte(value>>i);
        }
        else {
        	Value vv; vv.w = value;
                copycode();
                newcode((WORD)s_word,4l,vtype,curpc,vv);
                curpc += 4;
        }
}

PUBLIC void genblkw(type,size)
WORD type, size;
{
        if( type != s_number ) recover("Illegal expression in BLKW directive");
        genblkb(type, size * TargetBytesPerWord);
}

PUBLIC void genblkb(type,size)
WORD type,size;
{
        WORD bsize;
        Value vv; vv.w = 0;
        if( type != s_number ) recover("Illegal expression in BLKB directive");
        copycode();
        while( size > 0 )
        {
                bsize = size > 255 ? 255 : size;
                newcode((WORD)s_bss,bsize,0l,curpc,vv);
                curpc += bsize;
                size -= bsize;
        }
}

PUBLIC void genend()
{
	asm_Module *m;
	Value vv; vv.w = 0;
	genalign();	/* this will call copycode() */
	m = VMAddr(asm_Module,curmod);
        m->end = codeptr();
        newcode((WORD)s_end,0l,0l,curpc,vv);
        endmodule();
}

PUBLIC WORD asize(value)
WORD value;
{
        return ((value + TargetBytesPerWord - 1) &
                       ~(TargetBytesPerWord - 1) ) - value;
}

PRIVATE WORD guesssize(vtype,valuearg)
WORD vtype;
WORD valuearg;
{
	Value value; value.w = valuearg;

	switch( (int)vtype )
	{
	case s_number:
		return pfsize(value.w);
	case s_unbound:
	case s_codesymb:
	case s_coderef:
	case s_label:
	case s_datasymb:
	case s_commsymb:
		return instr_size;
	case s_modnum:
	case s_at:
		return instr_size>3?2:1;
	case s_expr:
	{
		Bnode *b = VMAddr(Bnode,value.v);
		WORD rtype, l, r;
		WORD rexp;

		rtype = b->rtype;
		rexp = b->rexp.w;
		l = guesssize((WORD)b->ltype,b->lexp.w);
		r = guesssize(rtype,rexp);
		if( l>r ) return l;
		else return r;
	}

	case s_monadic:
	{
		Unode *u = VMAddr(Unode,value.v);
		return guesssize((WORD)u->etype,u->expr.w);
	}

	default:
		recover("Unknown node in expression %d",vtype);
	}
	return 1;
}
