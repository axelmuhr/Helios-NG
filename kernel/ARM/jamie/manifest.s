TickChunk	*	(1000)	     

TickSize	*	(&00002700) 

TicksPerSlice	*	(&00007500) 

log2_numpris	*	(3)	

NumberPris	*	(1 :SHL: log2_numpris)

loc_internal	*	(&00000000)	

loc_CARD1	*	(&00000001)	

loc_internalFlash	*	(&000000FF)	

loc_limit	*	(loc_CARD1)	

VINT_CRX	*	(0)	

VINT_CTX	*	(1)	

VINT_MRX	*	(2)	

VINT_MTX	*	(3)	

VINT_EXA	*	(4)	

VINT_EXB	*	(5)	

VINT_EXC	*	(6)	

VINT_EXD	*	(7)	

VINT_TIM	*	(8)	

VINT_LCD	*	(9)	

VINT_MBK	*	(10)	

VINT_DB1	*	(11)	

VINT_DB2	*	(12)	

VINT_DB3	*	(13)	

VINT_POR	*	(14)	

UVec_Power	*	(0)	

UVec_MemLow	*	(1)	

UVec_Card	*	(2)	

InterruptVectors	*	(VINT_POR + 8)

UserVectors	*	(UVec_Card + 8)

Power_Fail	*	(1)	

Power_Down	*	(2)	

Power_Back	*	(3)	

MemLow_Cache	*	(1)	

MemLow_Low	*	(2)	

MemLow_Out	*	(3)	

Card_Insert	*	(1)	

Card_Extract	*	(2)	

	END
