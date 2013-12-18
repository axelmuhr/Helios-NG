#ifndef _lowlevel_h
#define _lowlevel_h

uword GetDirBase(void);
void SetDirBase(uword newdb);
void FlushData(uword flushspace);
void Install_Trap_Handler(Trap_Handler f);
void Trap_Exit(struct TrapData *td);
void *_GetModTab(void);

void Enable_Ints(void);
void Disable_Ints(void);

word SaveCPUState(SaveState *s);
word RestoreCPUState(SaveState *s);
word StartCPUState(SaveState *s);

word _system(WordFnPtr func,word arg1, word arg2, word arg3);
word Set_Psr(word psr);
void test_print(void);
word trap_test(void);

#endif
