// Simple assembler hello world C intercalling example
// P.A.Beskeen July '92
// Modified to support seperate code and data by NC, Nov '92

// include some AMPP macro files containing simple macros and constants


// By convention exported fn labels are prefixed by '.'

export .main				// export main fn.
import .printf				// import printf fn.


///////////////////////////////////////////////////////////////////////////////
// Entry point to code
//
// int main(int argc, char **argv)
.main:

// Get address of message into first argument reg.
// The patch instruction ensures that the correct field of the module table is
// used.

// No need to find symbol offset in data area as it is the only one.
// So just get address of start of this modules static data area.
patchinstr( PATCHC40MASK8ADD, shift ( 1, MODNUM ),
	LDI	*+ar4(0), r0 )		// get address of message 
					// r0 = arg 1 = address of string
	B	.printf			// branch to C printf() routine

// Note that due to the use of B, rather than LAJ, the call to printf() will
// return directly to caller.
// End of main().


///////////////////////////////////////////////////////////////////////////////
// Module initialisation code
//
// All Helios modules have to initialise their data areas. These data areas
// are guaranteed to be addressable from C byte pointers, code is not.
// Therefore we have to copy the string to be printed from the code, into
// the modules static data area.

	// Add this code to program's initialisation chain with 'init'.
	// Initialisation chains are called down before the main code.
	init

	// Check to see if we are initialising the data area (arg = 0)
	// and ignore any other kind of initialisation.
	CMPI	0,	r0
	Bne	r11

	// Get absolute address of hello_msg string in code.
	ldi	r11, ar5		// save link reg
	laj	4
		nop			// get around C40 LAJ bug
		patchinstr(PATCHC40MASK16ADD,
			shift(-2, labelref(hello_msg)),
			addi	-2, r11)	// add . + label
		ldi	r11, ar0
	ldi	ar5, r11		// restore link reg

	// Get C address of data area from module table into ar5.
	patchinstr( PATCHC40MASK8ADD, shift ( 1, MODNUM ),
		LDI	*+ar4(0),	ar5 )

	// Convert C byte address in ar5 to word pointer.
	lsh	-2, ar5
	addi	ir0, ar5

	// Copy string from code area to data area.
loop:
	LDI	*ar0++(1), r1		// load word from message
	STI	r1, *ar5++(1)		// store word into data area
	Bne	loop			// repeat until we have copied a zero
	Bu	r11			// return

// Declare message in the code.
hello_msg:
	byte	"Hello World via assembly language\n", 0
align
	word	0			// zero word halts copy
end_msg:

// Allocate space for our message in the module table's data area.

	data    message, (end_msg - hello_msg) * 4


	end
