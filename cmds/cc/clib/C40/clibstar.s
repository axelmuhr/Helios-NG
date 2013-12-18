        align
        module  7
.ModStart:
        word    0x60f160f1
		word modsize
	blkb    31, "Clib"
	byte 0
	word	modnum
	word	1000
		word	datasymb(.MaxData)
        init
	word	codesymb(.MaxCodeP)
Clib.library:
		global	Clib.library
		align
		init
				CMPI	2, R0
				Beq	_CodeTableInit
				patchinstr(PATCHC40MASK8ADD,
					shift(1, modnum),
					ldi	*+AR4(0), AR0)
	lsh	-2, AR0
	addi	IR0, AR0
				LDI	R11,	R7
				codetable __main
				global __main
				codetable _fgetc
				global _fgetc
				codetable _fputc
				global _fputc
				codetable _getc
				global _getc
				codetable _getchar
				global _getchar
				codetable _putc
				global _putc
				codetable _putchar
				global _putchar
				codetable _feof
				global _feof
				codetable _ferror
				global _ferror
				codetable _clearerr
				global _clearerr
				codetable _setvbuf
				global _setvbuf
				codetable _setbuf
				global _setbuf
				codetable _ungetc
				global _ungetc
				codetable __flushbuf
				global __flushbuf
				codetable _fflush
				global _fflush
				codetable __fillbuf
				global __fillbuf
				codetable _fclose
				global _fclose
				codetable _freopen
				global _freopen
				codetable _fopen
				global _fopen
				codetable __fopen_string_file
				global __fopen_string_file
				codetable __fisatty
				global __fisatty
				codetable _fgets
				global _fgets
				codetable _gets
				global _gets
				codetable _fputs
				global _fputs
				codetable _puts
				global _puts
				codetable _fread
				global _fread
				codetable _fwrite
				global _fwrite
				codetable _ftell
				global _ftell
				codetable _fseek
				global _fseek
				codetable _rewind
				global _rewind
				codetable _fgetpos
				global _fgetpos
				codetable _fsetpos
				global _fsetpos
				codetable _tmpnam
				global _tmpnam
				codetable _tmpfile
				global _tmpfile
				codetable _isalnum
				global _isalnum
				codetable _isalpha
				global _isalpha
				codetable _iscntrl
				global _iscntrl
				codetable _isdigit
				global _isdigit
				codetable _isgraph
				global _isgraph
				codetable _islower
				global _islower
				codetable _isprint
				global _isprint
				codetable _ispunct
				global _ispunct
				codetable _isspace
				global _isspace
				codetable _isupper
				global _isupper
				codetable _isxdigit
				global _isxdigit
				codetable _tolower
				global _tolower
				codetable _toupper
				global _toupper
				codetable _memmove
				global _memmove
				codetable _memcmp
				global _memcmp
				codetable _memchr
				global _memchr
				codetable _strchr
				global _strchr
				codetable _strrchr
				global _strrchr
				codetable _strspn
				global _strspn
				codetable _strcspn
				global _strcspn
				codetable _strpbrk
				global _strpbrk
				codetable _strstr
				global _strstr
				codetable _strtok
				global _strtok
				codetable _strcoll
				global _strcoll
				codetable _strxfrm
				global _strxfrm
				codetable ___vfprintf
				global ___vfprintf
				codetable _fprintf
				global _fprintf
				codetable _printf
				global _printf
				codetable _sprintf
				global _sprintf
				codetable _vfprintf
				global _vfprintf
				codetable __fprintf
				global __fprintf
				codetable __printf
				global __printf
				codetable __sprintf
				global __sprintf
				codetable _fscanf
				global _fscanf
				codetable _scanf
				global _scanf
				codetable _sscanf
				global _sscanf
				codetable _strtol
				global _strtol
				codetable _strtoul
				global _strtoul
				codetable _atoi
				global _atoi
				codetable _atol
				global _atol
				codetable _rand
				global _rand
				codetable _srand
				global _srand
				codetable __atexit_stub
				global __atexit_stub
				codetable __exit_stub
				global __exit_stub
				codetable __abort_stub
				global __abort_stub
				codetable _abs
				global _abs
				codetable _labs
				global _labs
				codetable _div
				global _div
				codetable _ldiv
				global _ldiv
				codetable _bsearch
				global _bsearch
				codetable _qsort
				global _qsort
				codetable _malloc
				global _malloc
				codetable _free
				global _free
				codetable _realloc
				global _realloc
				codetable _calloc
				global _calloc
				codetable _mktime
				global _mktime
				codetable _asctime
				global _asctime
				codetable _ctime
				global _ctime
				codetable _gmtime
				global _gmtime
				codetable _localtime
				global _localtime
				codetable __assert_fail
				global __assert_fail
				codetable _strerror
				global _strerror
				codetable _perror
				global _perror
				codetable _setlocale
				global _setlocale
				codetable _strftime
				global _strftime
				codetable _clock
				global _clock
				codetable _remove
				global _remove
				codetable _fileno
				global _fileno
				codetable _fdopen
				global _fdopen
				data __iob, 704
			global __iob 
			data __dummyctype, 4
				data __ctype, 256
			global __ctype 
				codetable _vprintf
				global _vprintf
				codetable _vsprintf
				global _vsprintf
				codetable _strtod
				global _strtod
				codetable _atof
				global _atof
				codetable _difftime
				global _difftime
				codetable _frexp
				global _frexp
				codetable _ldexp
				global _ldexp
				codetable _localeconv
				global _localeconv
				codetable _mblen
				global _mblen
				codetable _mbtowc
				global _mbtowc
				codetable _wctomb
				global _wctomb
				codetable _mbstowcs
				global _mbstowcs
				codetable _wcstombs
				global _wcstombs
				codetable ___rand
				global ___rand
				codetable ___srand
				global ___srand
	laj	_absaddr_._ctype + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_._ctype:
	int	shift(-2, labelref(._ctype))
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(__ctype)),
		ldi	*+AR4(0), AR3)
	lsh	-2, AR3
	addi	IR0, AR3
	patchinstr(PATCHC40MASK16ADD,
		shift(-2, datasymb(__ctype)),
		addi	0, AR3)
					ldi	(256 >> 2) - 1, rc
					rptb	end_loop__ctype
						ldi	*AR2++(1), R6
					end_loop__ctype:	sti	R6, *AR3++(1)
				b	R7		
				_CodeTableInit:
				patchinstr(PATCHC40MASK8ADD,
					shift(1, modnum),
					ldi	*+AR4(1), AR0)
	ldi	R11, AR5			
	laj	4
		nop				
		patchinstr(PATCHC40MASK16ADD,
			shift(-2, labelref(_FuncTableEnd)),
			addi	-2, R11)	
		ldi	R11, AR1
	ldi	AR5, R11			
				B	_Loop1Start		
				_Loop1:				
				ADDI	AR1, RS		
				STI	RS,	*AR0++(1)	
				_Loop1Start:			
				LDI *--AR1, RS	
				Bne	_Loop1	    		
				B	R11			
				_FuncTable:			
					int 0			
						int shift(-2, labelref(.__srand))
						int shift(-2, labelref(.__rand))
						int shift(-2, labelref(.wcstombs))
						int shift(-2, labelref(.mbstowcs))
						int shift(-2, labelref(.wctomb))
						int shift(-2, labelref(.mbtowc))
						int shift(-2, labelref(.mblen))
						int shift(-2, labelref(.localeconv))
						int shift(-2, labelref(.ldexp))
						int shift(-2, labelref(.frexp))
						int shift(-2, labelref(.difftime))
						int shift(-2, labelref(.atof))
						int shift(-2, labelref(.strtod))
						int shift(-2, labelref(.vsprintf))
						int shift(-2, labelref(.vprintf))
						int shift(-2, labelref(.fdopen))
						int shift(-2, labelref(.fileno))
						int shift(-2, labelref(.remove))
						int shift(-2, labelref(.clock))
						int shift(-2, labelref(.strftime))
						int shift(-2, labelref(.setlocale))
						int shift(-2, labelref(.perror))
						int shift(-2, labelref(.strerror))
						int shift(-2, labelref(._assert_fail))
						int shift(-2, labelref(.localtime))
						int shift(-2, labelref(.gmtime))
						int shift(-2, labelref(.ctime))
						int shift(-2, labelref(.asctime))
						int shift(-2, labelref(.mktime))
						int shift(-2, labelref(.calloc))
						int shift(-2, labelref(.realloc))
						int shift(-2, labelref(.free))
						int shift(-2, labelref(.malloc))
						int shift(-2, labelref(.qsort))
						int shift(-2, labelref(.bsearch))
						int shift(-2, labelref(.ldiv))
						int shift(-2, labelref(.div))
						int shift(-2, labelref(.labs))
						int shift(-2, labelref(.abs))
						int shift(-2, labelref(._abort_stub))
						int shift(-2, labelref(._exit_stub))
						int shift(-2, labelref(._atexit_stub))
						int shift(-2, labelref(.srand))
						int shift(-2, labelref(.rand))
						int shift(-2, labelref(.atol))
						int shift(-2, labelref(.atoi))
						int shift(-2, labelref(.strtoul))
						int shift(-2, labelref(.strtol))
						int shift(-2, labelref(.sscanf))
						int shift(-2, labelref(.scanf))
						int shift(-2, labelref(.fscanf))
						int shift(-2, labelref(._sprintf))
						int shift(-2, labelref(._printf))
						int shift(-2, labelref(._fprintf))
						int shift(-2, labelref(.vfprintf))
						int shift(-2, labelref(.sprintf))
						int shift(-2, labelref(.printf))
						int shift(-2, labelref(.fprintf))
						int shift(-2, labelref(.__vfprintf))
						int shift(-2, labelref(.strxfrm))
						int shift(-2, labelref(.strcoll))
						int shift(-2, labelref(.strtok))
						int shift(-2, labelref(.strstr))
						int shift(-2, labelref(.strpbrk))
						int shift(-2, labelref(.strcspn))
						int shift(-2, labelref(.strspn))
						int shift(-2, labelref(.strrchr))
						int shift(-2, labelref(.strchr))
						int shift(-2, labelref(.memchr))
						int shift(-2, labelref(.memcmp))
						int shift(-2, labelref(.memmove))
						int shift(-2, labelref(.toupper))
						int shift(-2, labelref(.tolower))
						int shift(-2, labelref(.isxdigit))
						int shift(-2, labelref(.isupper))
						int shift(-2, labelref(.isspace))
						int shift(-2, labelref(.ispunct))
						int shift(-2, labelref(.isprint))
						int shift(-2, labelref(.islower))
						int shift(-2, labelref(.isgraph))
						int shift(-2, labelref(.isdigit))
						int shift(-2, labelref(.iscntrl))
						int shift(-2, labelref(.isalpha))
						int shift(-2, labelref(.isalnum))
						int shift(-2, labelref(.tmpfile))
						int shift(-2, labelref(.tmpnam))
						int shift(-2, labelref(.fsetpos))
						int shift(-2, labelref(.fgetpos))
						int shift(-2, labelref(.rewind))
						int shift(-2, labelref(.fseek))
						int shift(-2, labelref(.ftell))
						int shift(-2, labelref(.fwrite))
						int shift(-2, labelref(.fread))
						int shift(-2, labelref(.puts))
						int shift(-2, labelref(.fputs))
						int shift(-2, labelref(.gets))
						int shift(-2, labelref(.fgets))
						int shift(-2, labelref(._fisatty))
						int shift(-2, labelref(._fopen_string_file))
						int shift(-2, labelref(.fopen))
						int shift(-2, labelref(.freopen))
						int shift(-2, labelref(.fclose))
						int shift(-2, labelref(._fillbuf))
						int shift(-2, labelref(.fflush))
						int shift(-2, labelref(._flushbuf))
						int shift(-2, labelref(.ungetc))
						int shift(-2, labelref(.setbuf))
						int shift(-2, labelref(.setvbuf))
						int shift(-2, labelref(.clearerr))
						int shift(-2, labelref(.ferror))
						int shift(-2, labelref(.feof))
						int shift(-2, labelref(.putchar))
						int shift(-2, labelref(.putc))
						int shift(-2, labelref(.getchar))
						int shift(-2, labelref(.getc))
						int shift(-2, labelref(.fputc))
						int shift(-2, labelref(.fgetc))
						int shift(-2, labelref(._main))
				_FuncTableEnd:			
		ref	Kernel.library
		ref	SysLib.library
		ref	Util.library
		ref	Posix.library
	align
	align
	byte	0,0,0,0		
._ctype:
	byte	64,64,64,64,   64,64,64,64,    64,65,65,65, 65,65,64,64
	byte	64,64,64,64,   64,64,64,64,    64,64,64,64, 64,64,64,64
	byte	5,2,2,2,       2,2,2,2,        2,2,2,2,     2,2,2,2	
	byte	32,32,32,32,   32,32,32,32,    32,32,2,2,   2,2,2,2	
	byte	2,144,144,144, 144,144,144,16, 16,16,16,16, 16,16,16,16 
	byte	16,16,16,16,   16,16,16,16,    16,16,16,2,  2,2,2,2	
	byte	2,136,136,136, 136,136,136,8,  8,8,8,8,     8,8,8,8	
	byte	8,8,8,8,       8,8,8,8,        8,8,8,2,     2,2,2,64	
	byte	0,0,0,0,       0,0,0,0,        0,0,0,0,     0,0,0,0
	byte	0,0,0,0,       0,0,0,0,        0,0,0,0,     0,0,0,0
	byte	0,0,0,0,       0,0,0,0,        0,0,0,0,     0,0,0,0
	byte	0,0,0,0,       0,0,0,0,        0,0,0,0,     0,0,0,0
	byte	0,0,0,0,       0,0,0,0,        0,0,0,0,     0,0,0,0
	byte	0,0,0,0,       0,0,0,0,        0,0,0,0,     0,0,0,0
	byte	0,0,0,0,       0,0,0,0,        0,0,0,0,     0,0,0,0
	byte	0,0,0,0,       0,0,0,0,        0,0,0,0,     0,0,0,0
		byte " %W% %G% Copyright (C) 1987 - 1992, Perihelion Software Ltd.", 0
	align
