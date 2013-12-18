_report ['include adapter.m]
_def  'adapter.m_flag 1

_def adapter [0xf8000000]

	 struct Adapter [
    		vec	7 LA_Pad1
		byte	LA_Idata
    		vec	7 LA_Pad2
		byte	LA_Odata
    		vec	7 LA_Pad3
		byte	LA_Istat
    		vec	7 LA_Pad4
		byte	LA_Ostat
        	]

_def 'i_rdy              1
_def 'i_inte             2
_def 'o_rdy              1
_def 'o_inte             2

