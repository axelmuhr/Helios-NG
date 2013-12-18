#ifndef adapter_h
#define adapter_h

#define i_rdy	1
#define i_inte	2
#define o_rdy	1
#define o_inte	2

typedef struct adapter {
	volatile char	LA_Pad1[7];
	volatile char	LA_IData;
	volatile char	LA_Pad2[7];
	volatile char	LA_OData;
	volatile char	LA_Pad3[7];
	volatile char	LA_IStat;
	volatile char	LA_Pad4[7];
	volatile char	LA_OStat;
} adapter;
           
#define ADAPTER ((adapter *)(0xf8000000))

#endif
