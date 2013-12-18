        mrs     r2, cpsr
        mrs     r2, spsr
//        mrs     r2, cpsr_flg    // illegal
//        mrs     r2, spsr_flg    // illegal
        mrs     r2, cpsr_all
        mrs     r2, spsr_all

        msr     cpsr, r3
        msr     spsr, r3
        msr     cpsr_flg, r3
        msr     spsr_flg, r3
        msr     cpsr_all, r3
        msr     spsr_all, r3

//        msr     cpsr, 0x5000000		// illegal
//        msr     spsr, 0x5000000         // illegal
        msr     cpsr_flg, 0x5000000
        msr     spsr_flg, 0x5000000

//        msr     cpsr_all, 0x5000000	// illegal
//        msr     spsr_all, 0x5000000	// illegal
