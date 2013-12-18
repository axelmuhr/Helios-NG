/* BUGS: mostly due to flaky beta Helios-ARM - later versions should fix?:
   1) TimedWait() is not functioning with HardenedSignal(). See reset code in
      InitKeyboard_HW().
   2) Numlock/caps/scrolllock don't work - Need to be SVCMode - but this hangs
      system.
   3) Need to stop tail call happening at end of InitKeyboard_HW() otherwise a
      crash will ensue - problem with EnterSVCMode()?
   4) Code needs to be integrated with window server.
*/

/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   S E R V E R			--
--                     -------------------------                        --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- keyboard.c								--
--                                                                      --
--	Native keyboard server						--
--                                                                      --
--	Author:  PAB 22/3/91						--
--                                                                      --
-- This keyboard server provides two interfaces, the std /keyboard	--
-- scancode key up/down interface as used by X Windows, and the		--
-- /console full ASCII keymap translation interface as used by some	--
-- basic window servers.				 		--
--									--
-- If CONSOLE is not #defined, the server will only support /keyboard.	--
-- Otherwise clients of the /keyboard server are given keyboard input	--
-- in preference to /console server clients.				--
--                                                                      --
-- There is also support for a simple remapping of the CapsLock/Ctrl	--
-- combination (because I hate the std IBM keyboard config).		--
--                                                                      --
-- CapsLock lights are also supported.                                  --
--									--
-- @@@ A device interface should be added at some point! Currently	--
-- The defunct ABC microlink interface and the Acorn Archimedes are	--
-- supported. This support is selected via #defines.			--
--									--
-- @@@ NumLock has to be handled properly				--
--									--
-- @@@ Need way of making server re-read its config info, i.e. to	--
-- change its keymap translation file, etc. (SIGHUP?)			--
-- At the mo the system must be reset. - use SetInfo() on /console.	--
--                                                                      --
-- @@@ What support might we want to allow for keyboard enhancers -	--
-- programs that might want to intercept and add characters to the	--
-- keyboard stream?							--
--									--
-- @@@ Add a mixed shift mask + scancode option to the keymap files	--
-- should allow you to specify shift+ctrl+f1 -> ascii style mappings	--
--									--
-- @@@ ARM ABC inter/extern keyboard should probably have different	--
-- EEPROM country config byte for each keyboard?			--
--									--
-- @@@ SetInfo interface						--
--	to send send new name of keymap to read and use			--
--	to set the value of swapctrlcaps, repeat rate and delay.	--
-- @@@ a SetInfo keyboard control program for user setting of above.	--
--									--
-- @@@ should be more closely tied to window server so programs may	--
-- indulge in direct keyboard reads without effecting other programs	--
-- @@@ Related to above, opens of /console should be stacked, old	--
-- users clients should not just be discarded when new ones open the	--
-- keyboard stream							--
--									--
--									--
-- ToDo:								--
-- CapsLock Light and bell for buffer full				--
-- EventPort check owner on FG_Close					--
--									--
------------------------------------------------------------------------*/
/* RCSId: $Id: keyboard.c,v 1.9 1993/09/15 09:53:39 paul Exp $ */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __in_keyboard 1	/* flag that we are in this module */
#define in_kernel 1	/* Cheat to get access to the "Config" information */

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>
#include <task.h>
#include <root.h>
#include <process.h>
#include <ioevents.h>
#ifdef __ABC
# include <abcARM/ABClib.h>	/* ABC specific library functions */
# include <abcARM/ROMitems.h>	/* ROM ITEM information and access functions */
#endif
#ifdef __ARCHIMEDES
# include <event.h>
# include <arm.h>
# include <ARM/ioc.h>
#endif

/*--------------------------------------------------------
--		Private types and definitions		--
--------------------------------------------------------*/

#ifndef __TRAN
/* Currently only the transputer implementation of Helios doesn't have these */
/* facilities. If this server is implemented on the tranputer the areas */
/* bracketed with these identifiers will need some revision. */
# define HARDENEDWAIT
# define TIMEDWAIT
#endif

#define Upb		ObjNode.Size	/* use ObjNode size field	*/
#define Buffers		ObjNode.Contents /* use ObjNode contents field	*/
#define Users		ObjNode.Account  /* number of opens		*/

typedef struct File {
	ObjNode		ObjNode;	/* node for directory struct	*/
} File;


/* IOEvent structure. Used to communicate scancode info between /keyboard
 * server and rest of system (direct users or /console server).
 */
typedef struct my_KeyboardEvent {
	IOEventHdr	h;
	Keyboard_Event	k;
} my_KeyboardEvent;


/* Scancode queue size */
#define	SCQSize 8

/* Scancode queue increment */
#define SCQINC(kh)	(((kh) + 1) & (SCQSize - 1))


#ifdef __ARCHIMEDES
/* Acorn Archimedes keyboard controller protocol definition */

/* Protocol ARM->Keyboard (PAK_) */
# define PAK_RequestKbId	0x20
# define PAK_RequestSPDReset	0x21
# define PAK_RequestMousePos	0x22
# define PAK_AckByte		0x3f
# define PAK_AckScan		0x31
# define PAK_AckMouse		0x32
# define PAK_AckScanMouse	0x33
# define PAK_LEDS		0x00	/* low nybble = LEDS */
# define PAK_SPDData		0xe0	/* low nybble = data */

/* Protocol Keyboard->ARM (PKA_) */
/* Two byte packets, row then col in low nybble, or x then y */
# define PKA_KeyUp		0xd0	/* low nybble = row then col */
# define PKA_KeyDown		0xc0	/* low nybble = row then col */
# define PKA_MouseChange	0x00	/* low 7 bits = x then y */
# define PKA_SPDData		0xe0	/* low nybble = data */

/* Shared protocol definitions ARM<->Keyboard */
# define P_HardReset		0xff
# define P_ResetAck1		0xfe
# define P_ResetAck2		0xfd

/* LED bits in protocol definition */
# define CapsLockLED	1
# define NumLockLED	2
# define ScrollLockLED	4

/*
 * Keyboard controller Tx protocol queue size.
 * possible items to Q are:
 *	reset requests or
 *	2 request types
 *	2 ack types
 *	2 led controls
 */
# define	TxQSize 8	/* 6, but keep word aligned */

/* TxQ queue increment */
# define TxQINC(txh)	(((txh) + 1) & (TxQSize - 1))

/* Enable and disable serial communications interrupts to and from
 * keyboard controller.
 */
# define EnableKCTx()	(IOC->irq_b.mask |= IRQB_KSTx)
# define EnableKCRx()	(IOC->irq_b.mask |= IRQB_KSRx)
# define DisableKCTx()	(IOC->irq_b.mask &= ~IRQB_KSTx)
# define DisableKCRx()	(IOC->irq_b.mask &= ~IRQB_KSRx)

 /* Interrupt handler structures. */
 Event KCRxEvent;	/* keyboard Controller Rx interrupt handler struct. */
 Event KCTxEvent;	/* 		       Tx 			    */

#endif


/* Private data used by /keyboard server and its device specific functions. */
typedef struct KeyboardBuff {
	word		KeyboardCounter;	/* protocol fluff */
	word		KeyboardTail;		/* used by reading fn */
	word		KeyboardHead;		/* used by writing fn */
	my_KeyboardEvent Keytab[SCQSize];	/* keyboard buffer */

#ifdef HARDENEDWAIT
	Semaphore	KeyboardSem;		/* kick keybrd Thread */
#endif
#ifdef __ARCHIMEDES
	/* data required by the Archi interrupt handlers. */
	volatile word	TxQHead;
	volatile word	TxQTail;
	volatile char	TxQ[TxQSize];

	word		LEDStatus;		/* State of keyboard LEDs. */

	word		ExpectResetSeq;
	word		ScanRow;
	word		DebugRx; /* TMP DBG */
	word		DebugRx2; /* TMP DBG */
	word		DebugTx; /* TMP DBG */
	word		DebugTx2; /* TMP DBG */
#ifdef DEBUG
	Semaphore	DebugSem;
	word		DebugNum;
#endif
#endif
} KeyboardBuff;


/*--------------------------------------------------------
--		Boring but Useful Prototypes		--
--------------------------------------------------------*/

static void InitKeyboard_HW(void);
static void InitKeyboard(void);
static void KeyboardOpen(ServInfo *servinfo);
static void KeyReadThread(void);
#ifdef __ABC
 static void KeyKick(void *buf, KeyboardBuff *KB);
#endif
#ifdef __ARCHIMEDES
 static word KCTxIntrHandler(KeyboardBuff *KB2, word vec);
 static word KCRxIntrHandler(KeyboardBuff *KB2, word vec);
 static void AddToTxQ(KeyboardBuff *KB2, char proto);
#endif

static void CapsLockLight(bool);
static void NumLockLight(bool);
static void ScrollLockLight(bool);

#ifdef CONSOLE
 static void ConsoleDecodeThread(void);
 static void KeyRepeatThread(void);
 static void InitConsole(void);
# ifdef __ABC
 static void InitKeyMap(bool internalkeyboard);
# else
 static void InitKeyMap(void);
# endif
 static void ConsoleOpen(ServInfo *servinfo);
#endif


/*--------------------------------------------------------
--		Private Data Definitions 		--
--------------------------------------------------------*/

#ifndef DEMANDLOADED
NameInfo Key_NameInfo =
{	NullPort,
	Flags_StripName,
	DefNameMatrix,
	(word *)NULL
};

# ifdef CONSOLE
NameInfo Con_NameInfo =
{	NullPort,
	Flags_StripName,
	DefNameMatrix,
	(word *)NULL
};
# endif
#endif

static ObjNode	KeyboardRoot;

static DispatchInfo Key_DisInfo = {
	(DirNode *)&KeyboardRoot,
	NullPort,
	SS_Keyboard,
	NULL,
	{ NULL, 0 },
	{
		{ KeyboardOpen,	2000 },		/* FG_Open		*/
		{ InvalidFn,	2000 },		/* FG_Create		*/
		{ DoLocate,	2000 },		/* FG_Locate		*/
		{ DoObjInfo,	2000 },		/* FG_ObjectInfo	*/
		{ NullFn,	2000 },		/* FG_ServerInfo	*/
		{ InvalidFn,	2000 },		/* FG_Delete		*/
		{ DoRename,	2000 },		/* FG_Rename		*/
		{ InvalidFn,	2000 },		/* FG_Link		*/
		{ DoProtect,	2000 },		/* FG_Protect		*/
		{ DoSetDate,	2000 },		/* FG_SetDate		*/
		{ DoRefine,	2000 },		/* FG_Refine		*/
		{ NullFn,	2000 },		/* FG_CloseObj		*/
		{ DoRevoke,	2000 },		/* FG_Revoke		*/
		{ InvalidFn,	2000 },		/* Reserved 		*/
		{ InvalidFn,	2000 }		/* Reserved		*/
	}
};

static Port		KeyboardPort	= NullPort;
static KeyboardBuff	KB;

#ifdef __ABC
static	ML_MsgHandler	MLHand_InKey, MLHand_ExKey;
# ifndef MLEinkey
#  define MLEinkey	0x94	/* internal keyboard microlink protocol */
#  define MLEexkey	0x98	/* external keyboard */
# endif
#endif

#ifdef CONSOLE
static ObjNode	ConsoleRoot;

static DispatchInfo Con_DisInfo = {
	(DirNode *)&ConsoleRoot,
	NullPort,
	SS_Keyboard,
	NULL,
	{ NULL, 0 },
	{
		{ ConsoleOpen,	3000 },		/* FG_Open		*/
		{ InvalidFn,	2000 },		/* FG_Create		*/
		{ DoLocate,	2000 },		/* FG_Locate		*/
		{ DoObjInfo,	2000 },		/* FG_ObjectInfo	*/
		{ NullFn,	2000 },		/* FG_ServerInfo	*/
		{ InvalidFn,	2000 },		/* FG_Delete		*/
		{ DoRename,	2000 },		/* FG_Rename		*/
		{ InvalidFn,	2000 },		/* FG_Link		*/
		{ DoProtect,	2000 },		/* FG_Protect		*/
		{ DoSetDate,	2000 },		/* FG_SetDate		*/
		{ DoRefine,	2000 },		/* FG_Refine		*/
		{ NullFn,	2000 },		/* FG_CloseObj		*/
		{ DoRevoke,	2000 },		/* FG_Revoke		*/
		{ InvalidFn,	2000 },		/* Reserved 		*/
		{ InvalidFn,	2000 }		/* Reserved		*/
	}
};

/* Port to send key events to clients. */
static Port	ConsolePort = NullPort;

/* Port to talk directly to the raw keyboard on. */
static Port	KeyboardToConsole;

/* Scancode to ASCII conversion table. */
static char	*KeyMap = NULL;

# ifdef __ABC
 static bool	KeyMapInROM = FALSE;		/* location of keymap */
 static bool	internalkeyboard = TRUE;	/* Which keymap to use */
# endif

/* Shift key scan codes: */
static char	l_shift_scancode, r_shift_scancode, l_ctrl_scancode,
		r_ctrl_scancode, l_alt_scancode, r_alt_scancode,
		l_fn_scancode, r_fn_scancode,
		capslock_scancode, numlock_scancode,
		scrolllock_scancode;

/* Current state of the shift keys. */
static char shiftstate = 0;
static bool capslock = FALSE;
static bool numlock = FALSE;
static bool scrolllock = FALSE;

/* TRUE if we are to swap ctrl and capslock key functions. */
static bool	swapctrlcaps = FALSE;

/* Masks for shift keys in shift state. */
#define	L_SHIFT 1
#define	R_SHIFT 2
#define	L_CTRL	4
#define	R_CTRL	8
#define	L_ALT	16
#define	R_ALT	32
#define	L_FN	64
#define	R_FN	128

/* Number of special shift keys defined at start of key map. */
#define NumShiftKeys 12

/* Combined shift key masks. */
#define	SHIFT	3
#define	CTRL	12
#define	ALT	48
#define	FN	192

/* Escape sequence start character. */
#define CSI	0x9b

/* # of shift key columns in keymap */
#define KEYMAPCOLS	4

/* define meta keys that will expand to escape sequences when post processed */
/* these keys always have their top bit set */
#define METAnull	0x80
#define METAstart	0x81
#define METAF1		0x81
#define METAF2		0x82
#define METAF3		0x83
#define METAF4		0x84
#define METAF5		0x85
#define METAF6		0x86
#define METAF7		0x87
#define METAF8		0x88
#define METAF9		0x89
#define	METAF10		0x8A
#define	METAF11		0x8B /* @@@ 11-16 are nonstd esc sequence definiions. */
#define	METAF12		0x8C
#define	METAF13		0x8D
#define	METAF14		0x8E
#define	METAF15		0x8F
#define	METAF16		0x90

#define METAUP		0x91
#define	METADOWN	0x92
#define METARIGHT	0x93
#define	METALEFT	0x94
#define	METAHELP	0x95
#define	METAUNDO	0x96
#define	METAHOME	0x97
#define	METAPAGEUP	0x98
#define METAEND		0x99
#define	METAPAGEDOWN	0x9A
#define	METAINSERT	0x9B
#define METAend		0x9B

/* Key Q used to hold /keyboard server input before passing on to user
 * when requested.
 */
#define	KeyQSize 64			/* key queue size */
#define KEYQINC(kh) \
	(((kh) + 1) & (KeyQSize - 1))	/* increment key Q  */
static char KeyQ[KeyQSize];		/* the keyboard queue */

word		KeyQTail = 0;		/* used by reading fn */
word		KeyQHead = 0;		/* used by writing fn */
word		KeyQCount = 0;
Port		KeyQSelect = NullPort;

Semaphore	KeyQSem;
Semaphore	KeyQKickSem;

#ifndef TIMEDWAIT
# define ConPollDelay	(OneSec / 30)	/* Disgusting delayed poll */
#endif


/* Key repeat thread variables */
					/* delays are specified in usecs */
static int RepStartDelay;		/* initial delay before a key repeat */
static int RepInterKeyDelay;		/* delay between each repeat of a key */

static Semaphore RepStartSem;		/* start a key repeat */
static Semaphore RepCancelSem;		/* cancel a key repeat */
static char RepKey = 0;			/* the processed character to repeat */
static int RepScanCode = 0;		/* the scancode to repeat */
#endif /* CONSOLE */


/*--------------------------------------------------------
-- main							--
--							--
-- Entry point of keyboard server.			--
--							--
--------------------------------------------------------*/

int main()
{
#ifndef DEMANDLOADED
	Object *Knte;
# ifdef CONSOLE
	Object *Cnte;
# endif
#endif
	char mcname[100];

#ifdef STANDALONE
	Environ env;
	GetEnv(MyTask->Port, &env); /* posix exec send's env ! */
	/* if executed by procman alone, we shouldn't read this */
#endif

#if 1
	SetPriority(ServerPri); /* ensure good response from keyboard */
#endif

	/* Do any internal server initialisation here */
	/* these fn should call Exit() if they fail */
	InitKeyboard();

#ifdef CONSOLE
	InitConsole();
#endif

	{
		Object *o;

		MachineName(mcname);
	
		Key_DisInfo.ParentName = mcname;
		InitNode(&KeyboardRoot, "keyboard", Type_File, 0, DefFileMatrix );
		InitList(&KeyboardRoot.Contents);

#ifdef CONSOLE
		Con_DisInfo.ParentName = mcname;
		InitNode(&ConsoleRoot, "console", Type_File, 0, DefFileMatrix );
		InitList(&ConsoleRoot.Contents);
#endif

#ifdef DEMANDLOADED
		Key_DisInfo.ReqPort = MyTask->Port;
#else
		Key_NameInfo.Port = Key_DisInfo.ReqPort = NewPort();
#endif
		Key_NameInfo.Flags = Flags_StripName;
		Key_NameInfo.Matrix = DefNameMatrix;
		Key_NameInfo.LoadData = NULL; /* @@@ Required?? */

#ifdef CONSOLE
# ifdef DEMANDLOADED
		Con_DisInfo.ReqPort = MyTask->Port;
# else
		Con_NameInfo.Port = Con_DisInfo.ReqPort = NewPort();
# endif
		Con_NameInfo.Flags = Flags_StripName;
		Con_NameInfo.Matrix = DefNameMatrix;
		Con_NameInfo.LoadData = NULL;
#endif
		/* parent is our machine root	*/
		o = Locate(NULL,mcname);

#ifndef DEMANDLOADED
		/* demand loaded servers already have name entry */
		Knte = Create(o,"keyboard",Type_Name,sizeof(NameInfo),
				(byte *)&Key_NameInfo);
# ifdef CONSOLE
		Cnte = Create(o,"console",Type_Name,sizeof(NameInfo),
				(byte *)&Con_NameInfo);
# endif
#endif
		Close(o);
	}

	Fork(1500, KeyReadThread, 0, 0);	/* get hardware key events */
#ifdef CONSOLE
	Fork(1500, ConsoleDecodeThread, 0);	/* scancode->ascii thread */
	Fork(2000, Dispatch, 4, &Con_DisInfo);  /* launch console server */
	Fork(1000, KeyRepeatThread, 0, 0);	/* key repeat handler */
#endif

#ifdef INSYSTEMIMAGE
	/* reply to procman that we have started */
	/* if we are part of system image 0x456 is expect to be returned */
	{
		MCB m;
		word e;
		InitMCB(&m,0,MyTask->Parent,NullPort,0x456);
		e = PutMsg(&m);
	}
#endif

	Dispatch(&Key_DisInfo);		/* start keyboard server */

#ifndef DEMANDLOADED
	Delete(Knte,NULL);
	Close(Knte);
# ifdef CONSOLE
	Delete(Cnte,NULL);	/* just assume console has been stopped as well */
	Close(Cnte);
# endif
#endif
	Exit(0);
}


/*--------------------------------------------------------
-- KeyboardOpen						--
--							--
-- Handle stream requests to keyboard server		--
--------------------------------------------------------*/

static void KeyboardOpen(ServInfo *servinfo)
{
	MCB	*mcb = servinfo->m;
	MsgBuf	*r;
	DirNode *d;
	ObjNode *f;
	IOCMsg2 *req = (IOCMsg2 *) (mcb->Control);
	Port    StreamPort;
	BYTE    *data = mcb->Data;
	char    *pathname = servinfo->Pathname;
  
	if ((d = (DirNode *) GetTargetDir(servinfo)) == NULL) {
		ErrorMsg(mcb, EO_Directory);
		return;
	}
   
	if ((f = GetTargetObj(servinfo)) == NULL) {
		ErrorMsg(mcb, EO_File);
		return;
	}
  
	unless (CheckMask(req->Common.Access.Access, req->Arg.Mode & Flags_Mode) ) {
		ErrorMsg(mcb, EC_Error+EG_Protected + EO_File);
		return;
	}

	if ((r = New(MsgBuf)) == NULL) {
		ErrorMsg(mcb, EC_Error + EG_NoMemory);
		return;
	}

	FormOpenReply(r, mcb, f,
		Flags_Closeable | Flags_Interactive | Flags_Server,
		pathname);

	if ((StreamPort = NewPort()) == NullPort) {
		ErrorMsg(mcb, EC_Error + EG_Congested + EO_Port);
		return;
	}
	r->mcb.MsgHdr.Reply = StreamPort;
  
	PutMsg(&r->mcb);
	Free(r);

	f->Account++;
	UnLockTarget(servinfo);
  
	forever	{
		WORD errcode;

		mcb->MsgHdr.Dest= StreamPort;
		mcb->Timeout	= StreamTimeout;
		mcb->Data	= data;

		errcode = GetMsg(mcb);

		if (errcode == EK_Timeout) {
			if (KeyboardPort != NullPort)
				continue;
			break;
		}

		if (errcode < Err_Null) { continue; }

		if ((errcode & FC_Mask) != FC_GSP) { 
			ErrorMsg(mcb, EC_Error + EG_WrongFn + EO_Stream);
        		continue;
		}

		switch( errcode & FG_Mask )
		{
        	case FG_Close: 
			if (KeyboardPort != NullPort) {
				AbortPort(KeyboardPort, EC_Error + SS_Keyboard + EG_Broken + EO_Stream);
#ifdef CONSOLE
					/* default back to console port */
					KeyboardPort = KeyboardToConsole;
#else
					KeyboardPort = NullPort;
#endif
			}
			if (mcb->MsgHdr.Reply != NullPort) {
				mcb->MsgHdr.FnRc	= 0;
				ErrorMsg(mcb, Err_Null);
			}

			FreePort(StreamPort);
			f->Account--;
			return;

		/* only allow people to enable events - no direct reads allowed */
		case FG_EnableEvents: 
 		{
			word mask = mcb->Control[0] & Event_Keyboard;

			if (mask == 0) {	/* disable */
				if (KeyboardPort != NullPort) {
					AbortPort(KeyboardPort, EC_Error + SS_Keyboard + EG_Broken + EO_Stream);
#ifdef CONSOLE
					/* default back to console port */
					KeyboardPort = KeyboardToConsole;
#else
					KeyboardPort = NullPort;
#endif
				}
				InitMCB(mcb, 0, mcb->MsgHdr.Reply, NullPort, Err_Null);
				MarshalWord(mcb, 0);
				PutMsg(mcb);
			}
			else
			{
				if (KeyboardPort != NullPort)
					AbortPort(KeyboardPort, EC_Error + SS_Keyboard + EG_Broken + EO_Stream);

				KeyboardPort = mcb->MsgHdr.Reply;
				InitMCB(mcb, MsgHdr_Flags_preserve, mcb->MsgHdr.Reply, NullPort, Err_Null);
				MarshalWord(mcb, mask);
				PutMsg(mcb);
			}
			break;
 		}

#if 0
		/* may wish to use these for something in the future */
		case FG_GetInfo:
		case FG_SetInfo:
#endif
		default:
			ErrorMsg(mcb, EC_Error + EG_WrongFn + EO_File);
		}
	}

	f->Account--;
	FreePort(StreamPort);
	if (KeyboardPort != NullPort) {
		AbortPort(KeyboardPort, EC_Error + SS_Keyboard + EG_Broken + EO_Stream);
		KeyboardPort = NullPort;
	}
}


/* Initialise the keyboard servers private data and the low level device. */
/* This is called before any KeyReadThread is forked. If an error is found */
/* then Exit() should be called. */
static	void InitKeyboard(void)
{
	int	i;

	KB.KeyboardCounter = 1;
	KB.KeyboardTail = KB.KeyboardHead = 0;

	for (i = 0; i < SCQSize; i++) {
		KB.Keytab[i].h.Type 	= Event_Keyboard;
		KB.Keytab[i].h.Stamp	= 0;
	}

#ifdef HARDENEDWAIT
	InitSemaphore(&KB.KeyboardSem, 0);
#ifdef DEBUG
	InitSemaphore(&KB.DebugSem, 0);
	KB.DebugNum = 0;
#endif
#endif

KB.DebugRx = 0; /* TMP DBG */
KB.DebugRx2 = 123; /* TMP DBG */
KB.DebugTx = 0; /* TMP DBG */
KB.DebugTx2 = 123; /* TMP DBG */

	/* Device specific initialisation. */
	InitKeyboard_HW();
}


#ifdef CONSOLE
static void InitConsole(void)
{
	/* default the keyboard server to send raw key events to our Thread */
	KeyboardPort = KeyboardToConsole = NewPort();

#ifdef __ABC
	InitKeyMap(TRUE);
#else
	InitKeyMap();
#endif
	InitSemaphore(&KeyQSem, 1);
	InitSemaphore(&KeyQKickSem, 0);
	InitSemaphore(&RepStartSem, 0);
	InitSemaphore(&RepCancelSem, 0);

#ifdef __ABC
	/* Enable soft configuration of repeat parameters */
	RepStartDelay = ReadEEPROM(EEPROM_KeyStartDelay);
	RepInterKeyDelay = ReadEEPROM(EEPROM_KeyInterDelay);

	if (RepStartDelay == 0)
		RepStartDelay = 666 * OneMillisec;
	if (RepInterKeyDelay == 0)
		RepInterKeyDelay = OneSec / 16;
#else
	RepStartDelay = 666 * OneMillisec;
	RepInterKeyDelay = OneSec / 16;
#endif
}


/*
 * Initialise keyboard map.
 * Be aware that this code may be called several times.
 * If the 'internalkeyboard' is TRUE then load map for internal keyboard,
 * else load map for external keyboard.
 */

#ifdef __ABC
static void InitKeyMap(bool internalkeyboard)
#else
static void InitKeyMap(void)
#endif
{
	char keyfile[32]; /* /helios/lib/inkeyNN.bkm or keyboard.bkm */
#ifdef __ABC
	word index = 0;			/* ROM ITEM index, (updated by exec) */
	ITEMstructure *iteminfo;	/* ROM ITEM header */
	char countrystr[3];
	char country;

	/* release old keymap memory */
	if (!KeyMapInROM && KeyMap != NULL)
		/* -NumShiftKeys as we adjusted its start past shift keys */
		Free(&KeyMap[-NumShiftKeys]);

	KeyMapInROM = FALSE;

	/* Find what keymap name we are supposed to be reading. */

	/* find IDD country keyboard code to use: inkeyXX.bkm */
	country = ReadEEPROM(EEPROM_KeyMap);

	/* top bit flags swapping of ctrl and CapsLock on keyboard */
	swapctrlcaps = country & 0x80;
	country &= 0x7f;

	if (country == 0)
		country = 44; /* default to UK */

	countrystr[0] = '0' + country / 10;
	countrystr[1] = '0' + country % 10;
	countrystr[2] = '\0';

	if (internalkeyboard)
		strcpy(keyfile,"/helios/lib/inkey");
	else
		strcpy(keyfile,"/helios/lib/exkey");

	strcat(keyfile, countrystr);
	strcat(keyfile, ".bkm");

#if 0
	IOdebug("key file: %s, cntrystr %s, country %d",keyfile,countrystr,country);
#endif	

	/* find location of keymap in ROM */
	while (GetROMItem(loc_internal, &index, &iteminfo)) {
		if ( strcmp(&keyfile[1], (char *)iteminfo->ITEMName) == 0) {
			KeyMap = (char *)(iteminfo->OBJECTOffset + (int)iteminfo);
			KeyMapInROM = TRUE;
			break;
		}
	}

	/* if keymap not in ROM, malloc space and read it in */
	if (KeyMap == NULL)
#else
	/* Release old keymap's memory if we are re-initialising. */
	if (KeyMap != NULL)
		/* -NumShiftKeys as we adjusted its start past shift keys */
		Free(&KeyMap[-NumShiftKeys]);

	strcpy(keyfile, "/helios/lib/keyboard.bkm");
#endif
	{
		Object	* obj = Locate(NULL, keyfile);
		Stream	* mfs = Open(obj, NULL, O_ReadOnly);
		int i, s;

		if (obj == NULL) {
			IOdebug("Keyboard Server: Cannot locate key map '%s'", keyfile);
			Exit(4);
		}
		if (mfs == NULL) {
			IOdebug("Keyboard Server: Cannot load key map '%s'", keyfile);
			Exit(4);
		}
		
		if ((s = GetFileSize(mfs)) <= 0) {
			IOdebug("Keyboard Server: empty key map");
			Exit(4);
		}

		if ((KeyMap = Malloc(s)) == NULL) {
			IOdebug("Keyboard Server: No memory free for key map");
			Exit(4);
		}

		i = Read(mfs, KeyMap, s, -1);
		if (i < s) {
			IOdebug("Keyboard Server: error reading key map");
			Exit(4);
		}

		Close(mfs);
		Close(obj);
	}

	/* Assign the shift keys */
	l_shift_scancode =	KeyMap[0];
	r_shift_scancode =	KeyMap[1];
	l_ctrl_scancode =	KeyMap[2];
	r_ctrl_scancode =	KeyMap[3];
	l_alt_scancode =	KeyMap[4];
	r_alt_scancode =	KeyMap[5];
	l_fn_scancode =		KeyMap[6];
	r_fn_scancode =		KeyMap[7];
	capslock_scancode =	KeyMap[8];
	numlock_scancode =	KeyMap[9];
	scrolllock_scancode =	KeyMap[10];

	if (swapctrlcaps) {
		/* swap functionality of ctrl and capslock keys */
		capslock_scancode =	l_ctrl_scancode;
		l_ctrl_scancode =	KeyMap[8];
	}

	/* now point at start of scancode conversion table */
	KeyMap = &KeyMap[NumShiftKeys];
}


static bool IsShift(char scancode)
{
	if (scancode == l_shift_scancode ||
	    scancode ==	r_shift_scancode ||
	    scancode == l_ctrl_scancode ||
	    scancode == r_ctrl_scancode ||
	    scancode == l_alt_scancode ||
	    scancode == r_alt_scancode ||
	    scancode == l_fn_scancode ||
	    scancode == r_fn_scancode ||
	    scancode == capslock_scancode ||
	    scancode == numlock_scancode ||
	    scancode == scrolllock_scancode) {
		return TRUE;
	    }

	return FALSE;
}


/* set the shift key state of a given shift key */
static void SetShift(char scancode)
{
	if (scancode == l_shift_scancode) {
		shiftstate |= L_SHIFT;
		return;
	}

	if (scancode ==	r_shift_scancode) {
		shiftstate |= R_SHIFT;
		return;
	}

	if (scancode == l_ctrl_scancode) {
		shiftstate |= L_CTRL;
		return;
	}

	if (scancode == r_ctrl_scancode) {
		shiftstate |= R_CTRL;
		return;
	}

	if (scancode == l_alt_scancode) {
		shiftstate |= L_ALT;
		return;
	}

 	if (scancode == r_alt_scancode) {
		shiftstate |= R_ALT;
		return;
	}

	if (scancode == l_fn_scancode) {
		shiftstate |= L_FN;
		return;
	}

	if (scancode == l_fn_scancode) {
		shiftstate |= R_FN;
		return;
	}

	if (scancode == capslock_scancode) {
		capslock = ~capslock;
		CapsLockLight(capslock);
		return;
	}

	if (scancode == numlock_scancode) {
		numlock = ~numlock;
		NumLockLight(numlock);
		return;
	}

	if (scancode == scrolllock_scancode) {
		scrolllock = ~scrolllock;
		ScrollLockLight(scrolllock);
		return;
	}
}


/* reset the shift key state of a given shift key */
static void ResetShift(char scancode)
{
	if (scancode == l_shift_scancode) {
		shiftstate &= ~L_SHIFT;
		return;
	}

	if (scancode ==	r_shift_scancode) {
		shiftstate &= ~R_SHIFT;
		return;
	}

	if (scancode == l_ctrl_scancode) {
		shiftstate &= ~L_CTRL;
		return;
	}

	if (scancode == r_ctrl_scancode) {
		shiftstate &= ~R_CTRL;
		return;
	}

	if (scancode == l_alt_scancode) {
		shiftstate &= ~L_ALT;
		return;
	}

	if (scancode == r_alt_scancode) {
		shiftstate &= ~R_ALT;
		return;
	}

	if (scancode == l_fn_scancode) {
		shiftstate &= ~L_FN;
		return;
	}

	if (scancode == l_fn_scancode) {
		shiftstate &= ~R_FN;
		return;
	}
}


static char ApplyShifts(char scancode)
{
	char *scanrow = &KeyMap[scancode * KEYMAPCOLS];
	char ascii;

	/* If CTRL-ALT and DELETE or BACKSPACE then do a soft system reset */
	if ((shiftstate & CTRL) && (shiftstate & ALT)
	&& (scanrow[0] == 8 || scanrow[0] == 127 || scanrow[1] == 127))
		Terminate(); /* no messin' system reset */

#if 1
	/* CTRL-SHIFT-F10 sequence resets the system as well */
	if ((shiftstate & CTRL) && (shiftstate & SHIFT)
	&& (scanrow[0] == METAF10 || ((shiftstate & FN) && scanrow[3] == METAF10)))
		Terminate(); /* no messin' system reset */
#endif

	if (shiftstate & FN)		/* extra shift key */
		return scanrow[3];

	if (shiftstate & ALT)
		return scanrow[2];

#if 0
	/* @@@ wait until we get a keyboard with numlock to play with */
	if (numlock) {
		/* do numlock processing */
	}
#endif
	/* Scroll Lock? - no effect on keys I believe */

	if (shiftstate & CTRL) {
		ascii = scanrow[1];	/* check for alpha keys and specials */
		if (ascii >= 'A' && ascii <= '_')
			return ascii - 0x40;

		ascii = scanrow[0];	/* check for shifted specials */
		if (ascii >= '[' && ascii <= '_')
			return ascii - 0x40;

		if (ascii == ' ')	/* special case ctrl-space = NULL */
			return 0;
	}

	if (capslock && *scanrow >= 'a' && *scanrow <= 'z') {
		if (shiftstate & SHIFT)
			return scanrow[0]; /* toggle case if caps & shift */
		else
			return scanrow[1];
	}

	if (shiftstate & SHIFT)
		return scanrow[1];

	return scanrow[0];
}


/* queue up character to return to the user */
static void SendKey(char c, char scancode)
{
	/* for Events, don't bother queueing the characters, just send them */
	if (ConsolePort != NullPort) {
		MCB m;
		my_KeyboardEvent Key;

		Key.k.Key = c;
		Key.k.What = scancode;
		Key.k.What |= (shiftstate << 16);
		if (capslock)
			Key.k.What |= (1 << 24);
		if (numlock)
			Key.k.What |= (1 << 25);
		if (scrolllock)
			Key.k.What |= (1 << 26);

		InitMCB(&m, MsgHdr_Flags_preserve, ConsolePort, NullPort, 0);
		m.Data = (byte *)&Key;
		m.MsgHdr.DataSize = sizeof(my_KeyboardEvent);
		m.Timeout	    = -1; /*5 * OneSec ?*/
		(void) PutMsg(&m);
		return;
	}

	if (KeyQCount == KeyQSize) { /* buffer full? */
		/* @@@ Ring Bell? */
#ifdef SYSDEB
		IOdebug("\aKey Queue Full (implement a bell%c)", 0x7);
#endif
#ifdef __ABC
		/* @@@ microlink simple bell */
#endif
		return;
	}

	Wait(&KeyQSem);
		KeyQ[KeyQHead] = c;
		KeyQHead = KEYQINC(KeyQHead);
		KeyQCount++;
		Signal(&KeyQKickSem);	/* kick console read routine into life */
	Signal(&KeyQSem);

	/* Select support - send message if we have waiter for input. */
	if (KeyQSelect != NullPort) {
		MCB m;
		InitMCB(&m, 0, KeyQSelect, NullPort, O_ReadOnly);
		PutMsg(&m);
		KeyQSelect = NullPort;
	}

}


static void PostProcess(char c, char scancode)
{
	SendKey(CSI, scancode); /* start esc sequence */

	if (c >= METAF1 && c <= METAF16) { /* max 16 fn keys allowed */
		/* Add shift prefixes to esc sequence */
		if ((shiftstate & SHIFT) && (shiftstate & ALT))
			SendKey('3', scancode);
		else if (shiftstate & ALT)
			SendKey('2', scancode);
		else if (shiftstate & SHIFT)
			SendKey('1', scancode);
		SendKey('0' + (c - 0x81), scancode);
		SendKey('~', scancode);

		return;
	}

	/* unshifted, ALT and ALT-shift are the same */
	/* for all meta keys apart from the fn keys */

	switch (c) {
	case METAUP:					/* cursor up */
		if (shiftstate & SHIFT) {
			SendKey('T', scancode); SendKey('~', scancode);
		}
		else
			SendKey('A', scancode);
		return;

	case METADOWN:					/* cursor down */
		if (shiftstate & SHIFT) {
			SendKey('S', scancode); SendKey('~', scancode);
		}
		else
			SendKey('B', scancode); /* unshifted and ALT shift are the same */
		return;

	case METARIGHT:					/* cursor right */
		if (shiftstate & SHIFT) {
			SendKey(' ', scancode); SendKey('@', scancode);
			SendKey('~', scancode);
		}
		else
			SendKey('C', scancode);
		return;

	case METALEFT:					/* cursor left */
		if (shiftstate & SHIFT) {
			SendKey(' ', scancode); SendKey('A', scancode);
			SendKey('~', scancode);
		}
		else
			SendKey('D', scancode);
		return;

	case METAHELP:						/* help */
		SendKey('?', scancode); SendKey('~', scancode);
		return;

	case METAUNDO:						/* undo */
		SendKey('1', scancode); SendKey('z', scancode);
		return;

	case METAHOME:						/* home */
		SendKey('H', scancode);
		return;

	case METAPAGEUP:					/* pageup */
		SendKey('3', scancode); SendKey('z', scancode);
		return;

	case METAPAGEDOWN:					/* pagedown */
		SendKey('4', scancode); SendKey('z', scancode);
		return;

	case METAEND:						/* insert */
		SendKey('2', scancode); SendKey('z', scancode);
		return;

	case METAINSERT:					/* insert */
		SendKey('@', scancode);
		return;
	}
}


static void StartRepeat(char c, int scancode)
{
	if (RepScanCode != 0)		/* is a key currently being repeated? */
		Signal(&RepCancelSem);	/* then stop that repeat */
	RepKey = c;
	RepScanCode = scancode;
	Signal(&RepStartSem);
}


static void CancelRepeat(int scancode)
{
	if (scancode == RepScanCode) {
		Signal(&RepCancelSem);
		RepScanCode = RepKey = 0;
	}
}

/*
** Wait for signal to repeat keys, then repeat the key with the relevent
** delays. This process can be canceled, either by the key being released,
** or a new key being held down.
**
*/

static void KeyRepeatThread(void)
{
#ifdef TIMEDWAIT
	forever {
		Wait(&RepStartSem);

		if (TimedWait(&RepCancelSem, RepStartDelay))
			continue;

		while (!TimedWait(&RepCancelSem, RepInterKeyDelay)) {
			/* The key queue is protected from concurrent access */
			/* by a semaphore in the SendKey fn */
			char key = RepKey;
			int  scancode = RepScanCode;

			if (key == 0)
				continue;

			if (key > 0x80) /* fn key esc sequences, etc */
				PostProcess(key, scancode);
			else
				SendKey(key, scancode);
		}
	}
#else
/*
** If you want to work out how to implement decent repeating keys without
** TimedWait(), then welcome to some mental gymnastics!
*/
# error Require TimedWait() fn in this processors version of Helios
#endif
}


/*
** Get characters from the /keyboard server and queue them up.
** When a server read request arrives, it simply pulls the keys out of this
** queue and returns them to the caller.
*/

void ConsoleDecodeThread(void)
{
	MCB m;
	my_KeyboardEvent Key;
	char scancode;

	forever {
		/* get char from /keyboard server */
		InitMCB(&m, MsgHdr_Flags_preserve, KeyboardToConsole, NullPort, 0);
		m.Data = (BYTE *) &Key;
		m.MsgHdr.DataSize = sizeof(my_KeyboardEvent);
		m.Timeout = -1;

		if (GetMsg(&m) < 0) {
#ifdef SYSDEB
			IOdebug("GetMsg Error in ConsoleThread"); /*debug*/
#endif
			/* /keyboard server may have new client - we will */
			/* continue to timeout until it defaults back to us */
			continue;
		}

		scancode = Key.k.Key & 0x7f;

#ifdef __ABC
		/* ABC ARM hardware supports two keyboards */
		/* see if we have to swap to a new keyboard map */
		/* for this scancode */
		if (internalkeyboard && (Key.k.Key & 0x80)) {
			/* Use external keyboard mappings */
			internalkeyboard = FALSE;
			InitKeyMap(FALSE);  /* load exkey.bkm */
		}
		else if (!internalkeyboard && !(Key.k.Key & 0x80)) {
			/* Use internal keyboard map */
			internalkeyboard = TRUE;
			InitKeyMap(TRUE); /* load inkey.bkm */
		}
#endif

		if (Key.k.What == Keys_KeyDown) {
			if (IsShift(scancode))
				SetShift(scancode);
			else {
				char c = ApplyShifts(scancode);

				/* if not null or unknown meta key */
				if (c != METAnull && c <= METAend)
				{
					StartRepeat(c, scancode);

					if (c > 0x80) /* fn key esc sequences, etc */
						PostProcess(c, scancode);
					else
						SendKey(c, scancode);
				}
#if 1
				else
					IOdebug("Unknown key %x passed to console thread", c);
#endif
			}
		}
		else
		{ /* keyup event */
			if (IsShift(scancode))
				ResetShift(scancode);
			else
				CancelRepeat(scancode);
		}
	}
}


/*
** Read keys out of our buffer.
**
** If no keys avail, then wait until timeout or chars
**
** Ignore any size - we always return one character at a time
** or a timeout (Read protocol allows this).
*/

static void ConsoleRead(MCB *sendto)
{
	ReadWrite *readwrite = (ReadWrite *) &(sendto->Control[0]);
 	word timeout = readwrite->Timeout;
	char c;

#ifdef TIMEDWAIT
	if (!TimedWait(&KeyQKickSem, timeout)) {
		/* timed out with no keys delivered */
		sendto->MsgHdr.FnRc = 0;
		ErrorMsg(sendto, EC_Recover + SS_Keyboard + EG_Timeout + EO_Stream);
		return;
	}
#else
	bool dotimeout = (timeout == -1 ? FALSE : TRUE);

	while( KeyQCount == 0 ) {
		/* Delay poll - inefficient - YUK! */
		Delay(ConPollDelay);
		if (dotimeout && (timeout -= ConPollDelay) <= 0) {
			/* timed out with no keys delivered */
			sendto->MsgHdr.FnRc = 0;
			ErrorMsg(sendto, EC_Recover + SS_Keyboard + EG_Timeout + EO_Stream);
			return;
		}
	}
#endif

	Wait(&KeyQSem);
		c = KeyQ[KeyQTail];
		KeyQTail = KEYQINC(KeyQTail);
		KeyQCount--;
	Signal(&KeyQSem);

	/* Send key back to client */
	InitMCB(sendto, MsgHdr_Flags_preserve, sendto->MsgHdr.Reply, NullPort, ReadRc_EOD);
	sendto->Data = &c;
	sendto->MsgHdr.DataSize = 1;
	PutMsg(sendto);
}


static void ConsoleSelect(MCB *m)
{
	word mode = m->MsgHdr.FnRc & FF_Mask;
	word result = 0;
	
	if( (mode & O_ReadOnly) && (KeyQCount != 0) ) {
		InitMCB(m, 0, m->MsgHdr.Reply, NullPort, O_ReadOnly);
		PutMsg(m);
	} else {
		if (KeyQSelect != NullPort)
			FreePort(KeyQSelect);
		KeyQSelect = m->MsgHdr.Reply;
	}
}


/*--------------------------------------------------------
-- ConsoleOpen						--
--							--
-- Handle stream requests to console server		--
--------------------------------------------------------*/

static void ConsoleOpen(ServInfo *servinfo)
{
	MCB	*mcb = servinfo->m;
	MsgBuf	*r;
	DirNode *d;
	ObjNode *f;
	IOCMsg2 *req = (IOCMsg2 *) (mcb->Control);
	Port    StreamPort;
	BYTE    *data = mcb->Data;
	char    *pathname = servinfo->Pathname;
  
	if ((d = (DirNode *) GetTargetDir(servinfo)) == NULL) {
		ErrorMsg(mcb, EO_Directory);
		return;
	}
   
	if((f = GetTargetObj(servinfo)) == NULL) {
		ErrorMsg(mcb, EO_File);
		return;
	}
  
	unless(CheckMask(req->Common.Access.Access, req->Arg.Mode & Flags_Mode) ) {
		ErrorMsg(mcb, EC_Error+EG_Protected + EO_File);
		return;
	}

	if ((r = New(MsgBuf)) == NULL) {
		ErrorMsg(mcb, EC_Error + EG_NoMemory);
		return;
	}

	FormOpenReply(r, mcb, f,
	 Flags_Closeable | Flags_Interactive | Flags_Selectable | Flags_Server,
	 pathname);

	if ((StreamPort = NewPort()) == NullPort) {
		ErrorMsg(mcb, EC_Error + EG_Congested + EO_Port);
		return;
	}

	r->mcb.MsgHdr.Reply = StreamPort;
   
	PutMsg(&r->mcb);
	Free(r);
	f->Account++;
	UnLockTarget(servinfo);
  
	forever	{
		WORD errcode;
		mcb->MsgHdr.Dest= StreamPort;
		mcb->Timeout	= StreamTimeout;
		mcb->Data	= data;

		errcode = GetMsg(mcb);

		if (errcode == EK_Timeout)
			break;

		if (errcode < Err_Null) { continue; }
     
		if ((errcode & FC_Mask) != FC_GSP) {
		       	ErrorMsg(mcb, EC_Error + EG_WrongFn + EO_Stream);
        		continue;
		}

		switch( errcode & FG_Mask )
		{
        	case FG_Close:
			/* @@@ check if EventPort owner is doing a close! */
			if (ConsolePort != NullPort) {
				FreePort(ConsolePort);
				ConsolePort = NullPort;
			}
				
			if (mcb->MsgHdr.Reply != NullPort) {
				mcb->MsgHdr.FnRc	= 0;
				ErrorMsg(mcb, Err_Null);
			}
			FreePort(StreamPort);
			f->Account--;
			return;

		case FG_Read:
			ConsoleRead(mcb);
			break;

		case FG_Select:
			ConsoleSelect(mcb);
			break;

		case FG_EnableEvents: 
 		{
			word mask = mcb->Control[0] & Event_Keyboard;

			if (mask == 0) {	/* disable */
				if (ConsolePort != NullPort) {
					AbortPort(ConsolePort, EC_Error + SS_Keyboard + EG_Broken + EO_Stream);
					ConsolePort = NullPort;
				}
				InitMCB(mcb, 0, mcb->MsgHdr.Reply, NullPort, Err_Null);
				MarshalWord(mcb, 0);
				PutMsg(mcb);
			}
			else
			{
				if (ConsolePort != NullPort)
					AbortPort(ConsolePort, EC_Error + SS_Keyboard + EG_Broken + EO_Stream);

				ConsolePort = mcb->MsgHdr.Reply;
				InitMCB(mcb, MsgHdr_Flags_preserve, mcb->MsgHdr.Reply, NullPort, Err_Null);
				MarshalWord(mcb, mask);
				PutMsg(mcb);
			}
			break;
		}

		case FG_GetInfo:
		case FG_SetInfo:
			/* @@@ set new values for KeyMap number / name */
			/* swapctrlcaps, and repeat rate and delay */
			/* + stay compatible with attribute mechanism */
			ErrorMsg(mcb, 0);
			break;

		default:
			ErrorMsg(mcb, EC_Error + EG_WrongFn + EO_File);
		}
	}

	f->Account--;
	FreePort(StreamPort);
}
#endif /* CONSOLE */


#ifdef __TRAN
void _stack_error(void)
{
	IOdebug("/keyboard server stack overflow!!");
}
#elif defined (__ARM)
# if 0 /* now have stack extension code */
static void __stack_overflow(void)
{
	IOdebug("/keyboard server stack overflow!");
}
static void __stack_overflow_1(void)
{
	IOdebug("/keyboard server stack overflow1!");
}
# endif
#endif


/* @@@ The following low level device specific functions really should be
 * converted into a device driver interface.
 */

#ifdef __ABC
/* ABC specific low level keyboard handler. */
/* This handler gets packets from the microlink and converts this into */
/* keyboard scancodes. */

static	void InitKeyboard_HW(void) {

	/* Register keybaord handler with microlink manager */
	MLHand_InKey.msgType = MLEinkey;
	MLHand_ExKey.msgType = MLEexkey;
	MLHand_ExKey.func = MLHand_InKey.func = KeyKick;
	MLHand_ExKey.arg = MLHand_InKey.arg = &KB;

	/* register fn to be called when in/ex keyboard message is received */
	ML_RegisterHandler(&MLHand_ExKey);
	ML_RegisterHandler(&MLHand_InKey);
}

static void CapsLockLight(bool clock)
{
	/* @@@ on ARM send request via the microlink */
	ubyte mlproto[2] = {0x98, 0xe0};	

	if (clock)
		mlproto[1] |= 1; /* light on */

	while (ML_Transmit(mlproto) < 0) ; /*null stat*/
}

static void NumLockLight(bool nlock)
{
#if 0
	if (nlock)
		/* numlock light on */
	else
		/* light off */
#endif
}

static void ScrollLockLight(bool slock)
{
#if 0
	if (slock)
		/* scroll lock light on */

	else
		/* light off */
#endif
}

/*
** This function receives microlink packets.
**
** It is called in SVC mode, and so can only queue up the data passed and
** HardenedSignal() a user Thread to further process the key
*/
static void KeyKick(void *buf, KeyboardBuff *KB2)
{
	bool  keyup = ((char *)buf)[1] & 0x80;
	ubyte scancode = ((char *)buf)[1] & 0x7F;
	bool  exkey = ((char *)buf)[0] == MLEexkey; /* extern keyboard? */

	if (SCQINC(KB2->KeyboardHead) == KB2->KeyboardTail) /* buffer full? */
		return;

	scancode = (exkey ? scancode | 0x80 : scancode);

	/* queue up key stroke */
	KB2->Keytab[KB2->KeyboardHead].h.Counter = KB2->KeyboardCounter++;
	KB2->Keytab[KB2->KeyboardHead].k.Key = scancode;
	KB2->Keytab[KB2->KeyboardHead].k.What = (keyup ? Keys_KeyUp : Keys_KeyDown);
	KB2->KeyboardHead = SCQINC(KB2->KeyboardHead);

	/* wake up user Thread */
	HardenedSignal(&KB2->KeyboardSem);
}

#else

/*
 * Low level Acorn Archimedes routines.
 *
 * Information on the Acorn keyboard protocol and handling can be
 * gleaned from the files ARM/akbdprot.doc and ARM/akbdsrc.s. Information
 * on the IOC keyboard interface can be found in the VLSI book (~pages 100-104).
 */
 
static	void InitKeyboard_HW(void) {
	int i, j;

	/* Initialise keyboard controller Tx protocol Q */
	KB.TxQTail = KB.TxQHead = 0;

#ifdef __ARM
	EnterSVCMode();
#endif

	/* Initialise baudrate generator 1 = max speed = 31,250 */
	IOC->timer_kart.count_lo = 1;
	IOC->timer_kart.count_hi = 0;
	IOC->timer_kart.go_cmd = 0; /* start baud rate generator */

	/* Dummy write to initialise link */
	IOC->kart_data = 0;
	/* delay for short period - buggy hardware fix */
	for (i = 0x200; i >= 0 ; i--)
		j = _linkreg();

	/* Dummy read to initialise link */
	i = IOC->kart_data;

	/* Key up/down protocol is a two byte packet: row, then column. */
	/* ScanRow = 0xff == row byte not received yet (as 0 is a valid row). */
	KB.ScanRow = 0xff;
	KB.ExpectResetSeq = P_HardReset;

	KCTxEvent.Pri = 999;
	KCTxEvent.Vector = INTR_IRQB_6;
	KCTxEvent.Code = (WordFnPtr)KCTxIntrHandler;
	KCTxEvent.Data = &KB;
	SetEvent(&KCTxEvent);

	KCRxEvent.Pri = 999;
	KCRxEvent.Vector = INTR_IRQB_7;
	KCRxEvent.Code = (WordFnPtr)KCRxIntrHandler;
	KCRxEvent.Data = &KB;
	SetEvent(&KCRxEvent);

	/* Enable keyboard controller Rx interrupts. */
	EnableKCRx(); /* These are enabled permanenently */

#if 0
	/* @@@ or should we loop forever - maybe the keyboard was not plugged */
	/* in, but could be in the future? */
	i = 3; /* number of times to retry reset */
#endif

#ifdef IFTIMEDWAITWASWORKINGWITHHARDENEDSIGNALCORRECTLY
	do {
#if 0
		if (i-- <= 0) {
			IOdebug("Failed to find or could not reset keyboard controller");
		Exit();
		}
#endif
		/* Start reset sequence, Rx interrupt handler will */
		/* handle the rest of the sequence, Signalling us on */
		/* successful completion of sequence. */
		AddToTxQ(&KB, KB.ExpectResetSeq = P_HardReset);

		/* If we do not receive confirmation that the sequence was */
		/* completed  OK then try another hard reset. */
	} while (TimedWait(&KB.KeyboardSem, OneSec * 3) == FALSE);
#else
	/* Start reset sequence, Rx interrupt handler will */
	/* handle the rest of the sequence, Signalling us on */
	/* successful completion of sequence. */
	AddToTxQ(&KB, KB.ExpectResetSeq = P_HardReset);

	HardenedWait(&KB.KeyboardSem);
#endif

#if 1 /* get around daft bug */
	for (i = 0x200; i >= 0 ; i--)
		j = _linkreg();
/*	IOdebug("InitKeyboard_HW: exit");
*/
#endif
	/* @@@ In theory, we could check the keyboard id here... */
	/* but only one type of keyboard exists so don't bother. */
}

static void CapsLockLight(bool clock)
{
	if (clock)
		/* capslock light on */
		KB.LEDStatus |= CapsLockLED;
	else
		/* light off */
		KB.LEDStatus &= ~CapsLockLED;

#ifdef UNKNOWNCRASHPROBLEM
/* SVCMode required! for KBTxIntrEnable set - but this still has problems */
	AddToTxQ(&KB, PAK_LEDS | KB.LEDStatus);
#endif
}

static void NumLockLight(bool nlock)
{
	if (nlock)
		/* numlock light on */
		KB.LEDStatus |= NumLockLED;
	else
		/* light off */
		KB.LEDStatus &= ~NumLockLED;

#ifdef UNKNOWNCRASHPROBLEM
	AddToTxQ(&KB, PAK_LEDS | KB.LEDStatus);
#endif
}

static void ScrollLockLight(bool slock)
{
	if (slock)
		/* scroll lock light on */
		KB.LEDStatus |= ScrollLockLED;
	else
		/* light off */
		KB.LEDStatus &= ~ScrollLockLED;

#ifdef UNKNOWNCRASHPROBLEM
	AddToTxQ(&KB, PAK_LEDS | KB.LEDStatus);
#endif
}
#endif

#pragma no_check_stack	/* shouldn't do stack checking in interrupt handlers */


/* Add byte to be sent to keyboard controller. The Tx interrupt handler will
 * read them out. The RxInterrupt handler may also add messages to the Q,
 * so it has to be guarded against interrupt interference.
 */
static void AddToTxQ(KeyboardBuff *KB2, char proto)
{
	IntsOff();
		if (TxQINC(KB2->TxQHead) == KB2->TxQTail)
			return; /* buffer full */

		KB2->TxQ[KB2->TxQHead] = proto;
		KB2->TxQHead = TxQINC(KB2->TxQHead);

		/* Things in Q so enable keyboard controller Tx interrupts. */
		EnableKCTx();
	IntsOn();
}

static void IntrAddToTxQ(KeyboardBuff *KB2, char proto)
{
	if (TxQINC(KB2->TxQHead) == KB2->TxQTail)
		return; /* buffer full */

	KB2->TxQ[KB2->TxQHead] = proto;
	KB2->TxQHead = TxQINC(KB2->TxQHead);

	/* Things in Q so enable keyboard controller Tx interrupts. */
	EnableKCTx();
}


/*
 * Rx Interrupt handler for protocol packets FROM the keyboard controller.
 *
 * It is called in IRQ mode, and so can only queue up the data passed and
 * HardenedSignal() a user Thread to further process the scancode recieved.
 */
static word KCRxIntrHandler(KeyboardBuff *KB2, word vec)
{
	int	proto;

	/* In theory, we should make sure we wait for 16 microseconds at this */
	/* point as the IOC asserts the Rx interrupt before the last bit has */
	/* been received. However the overhead of the system interrupt */
	/* handler makes this unecessary. */
	for (proto = 25; proto > 0; proto--) /* completely over the top... */
		_linkreg(); /* dummy fn. */

	proto = IOC->kart_data;
	KB2->DebugRx++;
	KB2->DebugRx2 = proto;
#if 0
	{
	char *i;
	int j;

	for (i = (char *)0x1fd8400; i < (char *)0x1fd8800; i++)
		*i = (char)proto;

	for (j = 0; j < 8 ; j++)
		((char *)0x1fda000 + KB.DebugRx * 16)[j] = (char)0xff;
	}
#endif

	if (KB2->ExpectResetSeq != 0) {
		/* Continue reset sequencing: */
		/* ARM->KB	HardReset */
		/* KB->ARM	HardReset */
		/* ARM->KB	ResetAck1 */
		/* KB->ARM	ResetAck1 */
		/* ARM->KB	ResetAck2 */
		/* KB->ARM	ResetAck2 */
		/* ARM->KB	AckScan */
		/* KB->ARM	scan codes */

		if (proto != KB2->ExpectResetSeq) {
			/* Didn't get expected reply, so restart rst sequence */
			IntrAddToTxQ(KB2, KB2->ExpectResetSeq = P_HardReset);
		} else {
			if (proto == P_HardReset) {
				IntrAddToTxQ(KB2, KB2->ExpectResetSeq = P_ResetAck1);
			} else if (proto == P_ResetAck1) {
				IntrAddToTxQ(KB2, KB2->ExpectResetSeq = P_ResetAck2);
			} else if (proto == P_ResetAck2) {
				/* completed reset sequence */
				KB2->ExpectResetSeq = 0;
				KB2->ScanRow = 0xff;
				IntrAddToTxQ(KB2, PAK_AckScan);
				/* Note that reset sequence waqs completed OK */
#if 0
				/* Cannot call kernel functions directly in interrupt handlers */
				/* This will re-enable interrupts and cause chaos. */
				HardenedSignal(&KB2->KeyboardSem);
#else
				PseudoTrap((word)&KB2->KeyboardSem, 0, 0,
					TRAP_HardenedSignal);
#endif
			}
		}

		return TRUE;
	}

	if ((proto & 0xf0) == PKA_KeyUp || (proto & 0xf0) == PKA_KeyDown) {

		/* KeyUp/Down scancode protocol byte processing */

		if (KB2->ScanRow == 0xff) {
			/* Extract Row and request second column nybble */
			KB2->ScanRow = (proto & 0x0f) << 4;
			IntrAddToTxQ(KB2, PAK_AckByte);
		} else {
			/* We already have row nybble, so we must have just */
			/* received a column nybble. Return the completed */
			/* scancode to the server. */

			if (SCQINC(KB2->KeyboardHead) != KB2->KeyboardTail) {
				/* If buffer is full then ignore the scancode */
				/* Note that in theory, this could lead to */
				/* keys repeating continously even though */
				/* they are no longer held down as the key */
				/* release message may have been lost. */

				/* If buffer not full - queue up key stroke */
				KB2->Keytab[KB2->KeyboardHead].h.Counter \
					= KB2->KeyboardCounter++;
				KB2->Keytab[KB2->KeyboardHead].k.Key \
					= KB2->ScanRow | (proto & 0x0f);
				KB2->Keytab[KB2->KeyboardHead].k.What \
					= ((proto & 0xf0) == PKA_KeyUp) ?
						Keys_KeyUp : Keys_KeyDown;
				KB2->KeyboardHead = SCQINC(KB2->KeyboardHead);

				/* Wake up user KeyReadThread. */
#if 0
				/* Cannot call kernel functions directly in interrupt handlers */
				/* This will re-enable interrupts and cause chaos. */
				HardenedSignal(&KB2->KeyboardSem);
#else
				PseudoTrap((word)&KB2->KeyboardSem, 0, 0,
					TRAP_HardenedSignal);
#endif
			}

			/* Note that next byte of scancode protocol will be */
			/* the first (row) byte of a 2 byte packet. */
			KB2->ScanRow = 0xff;
			IntrAddToTxQ(KB2, PAK_AckScan);
		}
		return TRUE;
	}

#if 0
	/* In future maybe check for mouse events? */
	if (proto == PAK_MouseChange) {
	}
	/* What the **** is SPDData? */
	if (proto == PAK_SPDData) {
	}
#endif

	/* Didn't get known reply, so restart rst sequence */
	KB2->ScanRow = 0xff;
	IntrAddToTxQ(KB2, KB2->ExpectResetSeq = P_HardReset);

	return TRUE;
}

/*
 * Tx Interrupt handler for protocol packets TO the keyboard controller.
 *
 * Sends any entries in the Tx Q to the keyboard controller. If no more to
 * be sent is disables Tx interrupts. Tx interrupts will be re-enabled when
 * new data is placed into the Tx Q.
 */
static word KCTxIntrHandler(KeyboardBuff *KB2, word vec)
{
	if (KB2->TxQHead == KB2->TxQTail) {
		/* If Q is empty disable Tx interrupts until more to send. */
		DisableKCTx();
	} else {
#if 0
	char *i;

	for (i = (char *)0x1fd8800; i < (char *)0x1fd9000; i++)
		*i = KB2->TxQ[KB2->TxQTail];
#endif
		/* Otherwise send data and update Q pointer. */
#if 0
	KB2->DebugTx++;
	KB2->DebugTx2 = KB2->TxQ[KB2->TxQTail];
#endif
		IOC->kart_data = KB2->TxQ[KB2->TxQTail];
		KB2->TxQTail = TxQINC(KB2->TxQTail);
	}

	return TRUE;
}


#pragma check_stack


static void KeyReadThread(void)
{
	MCB	m;

#ifdef __ARM
	EnterSVCMode();
#endif

	forever {
#ifdef HARDENEDWAIT
		HardenedWait(&KB.KeyboardSem);
#else
# error "Other Processors: wait for key action, and place into buf"
#endif

		if (KeyboardPort == NullPort) {
			KB.KeyboardTail = SCQINC(KB.KeyboardTail);
			continue;
		}
/* IOdebug("KeyReadThread: scan %x, what %x", KB.Keytab[KB.KeyboardTail].k.Key,
			KB.Keytab[KB.KeyboardTail].k.What);
*/
		InitMCB(&m, MsgHdr_Flags_preserve, KeyboardPort, NullPort, 0);
		m.Data = (BYTE *) &(KB.Keytab[KB.KeyboardTail]);
		m.MsgHdr.DataSize = sizeof(my_KeyboardEvent);
		m.Timeout	    = -1; /*5 * OneSec ?*/
		(void) PutMsg(&m);

		KB.KeyboardTail = SCQINC(KB.KeyboardTail);
	}
}


/* End of keyboard.c */
