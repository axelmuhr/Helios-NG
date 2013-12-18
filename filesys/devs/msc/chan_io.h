/* Helios Channel I/O routines */

extern int 	ChanRead(Channel *, byte *, int);
extern int 	ChanWrite(Channel *, byte *, int);
extern word	ReadWord(Channel *);
extern void	WriteWord(Channel *, word);
extern word	ReadByte(Channel *);
extern void 	WriteByte(Channel *, byte);

