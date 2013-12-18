/* $Id: debug.c,v 1.9 1994/09/23 08:44:45 nickc Exp $ */

#include "cchdr.h"
#include "AEops.h"
#include "cg.h"
#include "util.h"
#include <stdio.h>
#ifdef __STDC__
#define start_args( a, b )	va_start( a, b )
#include <stdarg.h>
#define va_dcl
#else
#include <varargs.h>
#define start_args( a, b )	va_start( a )
#endif

#ifdef COMPILING_ON_XPUTER
#undef va_alist
#define va_alist ...
#define va_dcl
#endif

#ifdef COMPILING_ON_DOS
#define va_alist ...
#define va_dcl 
#endif

#define LAYOUT

#define sizeof_int 4
#define padsize(n,align) (-((-(n)) & (-(align))))

extern int dataloc;
static int indent = 0;

FILE *dbfile = NULL;

struct stdef
{
	struct stdef	*next;
	int		done;
	TagBinder	*b;
} *stlist;


void dbprintf(fmt, va_alist )
char *fmt;
va_dcl
{
	va_list a;
	
	start_args(a,fmt);
	
	vfprintf(dbfile,fmt,a);
	
	va_end(a);
}

db_init(name)
char *name;
{
	char dbname[30];
	char *p = dbname;
	
	strcpy(dbname,name);
	
	while ( *p && (*p != '.') ) p++;
	*p = 0;
	
	strcat(dbname,".dbg");
	
	dbfile = fopen(dbname,"w");

	dbprintf("void=#0;\n");	
	dbprintf("char=#1;\n");
	dbprintf("short=-2;\n");	
	dbprintf("int=-4;\n");	
	dbprintf("long=-4;\n");	

	dbprintf("signed char=-2;\n");
	
	dbprintf("unsigned short=#2;\n");
	dbprintf("unsigned int=#4;\n");
	dbprintf("unsigned long=#4;\n");

	dbprintf("enum=#4;\n");	
	dbprintf("float=.4;\n");
	dbprintf("double=.8;\n");
}

db_tidy()
{
	struct stdef *s = stlist;
#ifdef LAYOUT
	dbprintf("\n");
#endif
	while( s )
	{
		if( !s->done )
		{
			TagBinder *b = s->b;
			TypeExpr *x = list3(s_typespec,bitoftype_(s_struct),b);
			dbprintf("__struct_%s=",_symname(bindsym_(b)));
			db_type(x);
			dbprintf(";\n");
		}
		s = s->next;
	}
	fclose(dbfile);
}


void addstdef(b)
TagBinder *b;
{
	struct stdef *s = stlist;
	int done = (tagbindmems_(b) != 0);

	while( s )
	{
		if( s->b == b )
		{
			if( done ) s->done = 1;
			return;
		}
		s = s->next;
	}

	s = (struct stdef *) malloc(sizeof(struct stdef));

	s->next = stlist;
	s->done = done;
	s->b = b;

	stlist = s;
}

/* still to do here are the divisions into unsigned and long */
int db_type(x)
TypeExpr *x;
{
    SET_BITMAP m;
    int bitoff;
    TagMemList *l;
    TagBinder *b;
    int n, size;
    int padn=0;
            
    switch (h0_(x))
    {   
         case s_typespec:
            m = typespecmap_(x);
            switch (m & -m)    /* LSB - unsigned/long etc. are higher */
            {   
                case bitoftype_(s_char):
                    dbprintf("char");
                    return 1;
                    
                case bitoftype_(s_int):
                    if (m & BITFIELD)
                      sem_rerr("db_type <bit field> illegal - db_type(int) assumed");
                    if (m & bitoftype_(s_short))
                    {
                    	dbprintf("short");
                    	return 2;
                    }
                    else if (m & bitoftype_(s_long))
                    {
                    	dbprintf("long");
                    	return 4;
                    }
		    dbprintf("int");
		    return 4;
                    
                case bitoftype_(s_enum):
#if 0
                    b = typespectagbind_(x);
                    dbprintf("<");
                    if( *_symname(bindsym_(b)) != '<' ) 
                    	dbprintf("$%s;",_symname(bindsym_(b)));
                    for ( n=0; l != 0; l = l->memcdr)
                    {
                        dbprintf("%s:%d;",_symname(l->memsv),n);
                        n++;
                    }
                    dbprintf(">int");
#endif
		    dbprintf("enum");
                    return 4;
                    
                case bitoftype_(s_double):
                    if (m & bitoftype_(s_short)) 
                    { dbprintf("float"); return 4; }
                    else dbprintf("double");
                    return 8;
                    
                case bitoftype_(s_struct):
                    b = typespectagbind_(x);
                    addstdef(b);
                    if ((l = tagbindmems_(b)) == 0)
                    {
                    	dbprintf("$%s",_symname(bindsym_(b)));
                    	return 0;
		    }
                    dbprintf("{");
                    if( *_symname(bindsym_(b)) != '<' ) 
                    	dbprintf("$%s;",_symname(bindsym_(b)));
                    for (bitoff=n=0; l != 0; l = l->memcdr)
                    {
                        if( l->memsv == NULL ) dbprintf("pad%d:",padn++);
                        else dbprintf("%s:",_symname(l->memsv));
			
                    	if (l->membits)
                        {   
                            int k = evaluate(l->membits);
                            size = 0;
                            n = padsize(n, alignoftype(te_int));
                            if (k == 0) k = 32-bitoff;  /* rest of int */
                            if (k+bitoff > 32) size=sizeof_int, bitoff = 0; /* ovflow */
                            dbprintf("%%%d,%d",bitoff,k);
                            bitoff += k;
                        }
                        else
                        {   if (bitoff != 0) n += sizeof_int, bitoff = 0;
                            n = padsize(n, alignoftype(l->memtype));
                            size =  db_type(l->memtype);
                        }
                        dbprintf(":%d;",n);
                        n += size;
                    }
                    dbprintf("}");
                    if (bitoff != 0) n += sizeof_int, bitoff = 0;
                    return padsize(n,4);
                    
                case bitoftype_(s_union):
                    b = typespectagbind_(x);
                    addstdef(b);
                    if ((l = tagbindmems_(b)) == 0)
                    {
                    	dbprintf("$%s",_symname(bindsym_(b)));
                    	return 0;
		    }
                    dbprintf("{");
                    if( *_symname(bindsym_(b)) != '<' ) 
                    	dbprintf("$%s;",_symname(bindsym_(b)));
                    for (n=0; l != 0; l = l->memcdr)
                    {
                    	dbprintf("%s:",_symname(l->memsv));
                        n = max(n, l->membits ? sizeof_int : db_type(l->memtype));
                        dbprintf(":0;");
                    }
                    dbprintf("}");
                    return padsize(n,sizeof_int);
              
                case bitoftype_(s_typedefname):
		    dbprintf("%s",_symname(bindsym_(typespecbind_(x))));
                    return sizeoftype(bindtype_(typespecbind_(x)));
                    
                case bitoftype_(s_void):
                    dbprintf("void");
                    return 0;

                default: break;
            }
            /* drop through for now */
        default:
            syserr("db_type(%d,0x%x)", h0_(x), typespecmap_(x));
        case t_subscript:
            n = sizeoftype(typearg_(x));
            if (typesubsize_(x)) n *= evaluate(typesubsize_(x));
            else typesubsize_(x) = globalize_int(1),
                 sem_rerr("size of a [] array required, treated as [1]");
            dbprintf("[%d]",evaluate(typesubsize_(x)));
            db_type(typearg_(x));
            return n;
            
        case t_fnap:
	    dbprintf("()");
	    db_type(typearg_(x));
	    return 4;
	    
        case t_content:
        {
        	TypeExpr *x1 = typearg_(x);
		dbprintf("*");
		if
		(	(h0_(x1) == s_typespec) &&
			(
				(typespecmap_(x1) & bitoftype_(s_struct)) ||
				(typespecmap_(x1) & bitoftype_(s_union))
			)
		)
		{
			b = typespectagbind_(x1);
			dbprintf("$%s",_symname(bindsym_(b)));
	    	}
		else db_type(x1);
		return 4;
	}
    }
}


db_bindlist(l,arglist)
BindList *l;
int arglist;
{
	extern int ssp;
	BindList *x;
	int offset = arglist?8:(ssp*4);
	int size;

	for( x = l; x != NULL ; x = x->bindlistcdr )
	{
		Binder *b = x->bindlistcar;
		if( arglist || (bindstg_(b) & bitofstg_(s_auto)) )
		{
			dbprintf("%s:",_symname(bindsym_(b)));
			size = db_type(b->bindtype);
			size = pad_to_word(size);
			if( arglist )
			{
				dbprintf(":%d;",offset);
				if( size < 4 ) size = 4;
				offset += size;
			}
			else
			{
				if( use_vecstack && size > 8 ) size = 4;
				else if( size < 4 ) size = 4;
				offset += size;
				dbprintf(":%d;",-offset);
			}
		}
		else if( bindstg_(b) & bitofstg_(s_static) )
		{
			dbprintf("%s:",_symname(bindsym_(b)));
			db_type(b->bindtype);
			dbprintf(":%d:s;",bindaddr_(b));
		}
	}
}

db_proc(proc,formals,type)
Binder *proc;
BindList *formals;
TypeExpr *type;
{
#ifdef LAYOUT
	dbprintf("\n");
#endif
	dbprintf("%s:(",_symname(bindsym_(proc)));
	db_bindlist(formals,1);
	dbprintf(")");
	db_type(type);
	dbprintf(":%d=",dataloc);
}

db_blockstart(x)
Cmd *x;
{
	CmdList *cl = cmdblk_cl_(x);
	BindList *bl = cmdblk_bl_(x);
	int startline = x->fileline.l;
	int endline = startline;	
	int i;
	
#ifdef LAYOUT
	dbprintf("\n");
	indent++;
	for( i=0; i<indent ; i++ ) dbprintf(" ");
#endif

	dbprintf("{%d",startline);

	while( cl != NULL )
	{
		Cmd *cmd = cmdcar_(cl);
		endline = cmd->fileline.l;
		dbprintf(",%d",endline);
		switch( h0_(cmd) )
		{
		default: break;
		
		case s_if:
			if( h0_(cmd2c_(cmd)) != s_block )
				dbprintf(",%d",cmd2c_(cmd)->fileline.l);
			if( cmd3c_(cmd) != 0 && h0_(cmd3c_(cmd)) != s_block )
				dbprintf(",%d",cmd3c_(cmd)->fileline.l);
			break;
			
		case s_do:	cmd = cmd1c_(cmd);	goto doloop;
		case s_for:	cmd = cmd4c_(cmd);
		doloop:
			if( h0_(cmd) != s_block )
				dbprintf(",%d",cmd->fileline.l);
		}
		cl = cdr_(cl);
	}
	dbprintf(";");
#ifdef LAYOUT
	dbprintf("\n");
	indent++;
	for( i=0; i<indent ; i++ ) dbprintf(" ");
#endif	
	db_bindlist(bl,0);
}

db_blockend(x)
Cmd *x;
{
#ifdef LAYOUT
	indent-=2;
#endif
	dbprintf("};");
}

db_static(d)
DeclRhsList *d;
{
	int stat = 0;
	SET_BITMAP stg = d->declstg;

	if( stg & b_fnconst ) return;
	
	switch( stg & -stg )
	{
	case bitofstg_(s_typedef):
		dbprintf("%s=",_symname(d->declname));
		db_type(d->decltype);
		dbprintf(";\n");
		break;
		
        default:
		syserr("db_static(0x%x)", stg);
        case bitofstg_(s_auto):
		break;
	    
        case bitofstg_(s_static): stat = 1;
        case bitofstg_(s_extern):
            if (!(d->declstg & b_undef))
	    {
		dbprintf("%s:",_symname(d->declname));
		db_type(d->decltype);
		dbprintf(":%d:%c;\n",dataloc,stat?'s':'g');
		break;
	    }
            break;
	}
}
