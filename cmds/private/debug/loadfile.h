

#define t_eof           -1
#define t_code          -11
#define t_long          16
#define t_init          41
#define t_maininit      43
#define t_datasymb      33
#define t_codesymb      45
#define t_codefix       37
#define t_datafix       38
#define t_staticfix     39
#define t_limit         40
#define t_modnum        44
#define t_ref           34

#define f_pfix          0x2
#define f_nfix          0x6
#define f_call          0x9
#define f_j             0x0
#define f_cj            0xa
#define f_ldc           0x4
#define f_ldnl          0x3
#define f_ldnlp         0x5
#define f_stnl          0xe

struct symb *nearsym();
struct symb *findsym();

extern struct List codes;
extern struct List datas;
extern struct List refs;
extern struct List fixes;

extern WORD modbase;

struct symb {
        struct Node node;
        WORD value;
        BYTE name[32];
};

struct fix {
        struct Node node;
        WORD op;
        WORD type;
        WORD loc;
        WORD value;
};

