
#define f_pfix          0x2
#define f_nfix          0x6
#define f_call          0x9
#define f_j             0x0
#define f_cj            0xa
#define f_ldc           0x4
#define f_ldnl          0x3
#define f_ldnlp         0x5
#define f_stnl          0xe
#define f_ldl           0x7
#define f_stl           0xd
#define f_ldlp          0x1

#ifdef T414
#define MemStart  MinInt+0x48
#define LoadBase (MinInt+0x800)
#endif

#ifdef T800
#define MemStart  MinInt+0x70
#define LoadBase (MinInt+0x1000)
#endif
