        align
        module  6
.ModStart:
        word    0x60f160f1
		word modsize
	blkb    31, "Posix"
	byte 0
	word	modnum
	word	1000
		word	datasymb(.MaxData)
        init
	word	codesymb(.MaxCodeP)
Posix.library:
		global	Posix.library
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
				codetable __posix_init
				global __posix_init
			data _errno, 4
			global _errno 
			data _environ, 4
			global _environ 
				codetable _open
				global _open
				codetable _creat
				global _creat
				codetable _umask
				global _umask
				codetable _link
				global _link
				codetable _mkdir
				global _mkdir
				codetable _mkfifo
				global _mkfifo
				codetable _unlink
				global _unlink
				codetable _rmdir
				global _rmdir
				codetable _rename
				global _rename
				codetable _stat
				global _stat
				codetable _fstat
				global _fstat
				codetable _access
				global _access
				codetable _chmod
				global _chmod
				codetable _chown
				global _chown
				codetable _utime
				global _utime
				codetable _pathconf
				global _pathconf
				codetable _fpathconf
				global _fpathconf
				codetable _pipe
				global _pipe
				codetable _dup
				global _dup
				codetable _dup2
				global _dup2
				codetable _close
				global _close
				codetable _read
				global _read
				codetable _write
				global _write
				codetable _fcntl
				global _fcntl
				codetable _lseek
				global _lseek
				codetable _fdstream
				global _fdstream
				codetable _cdobj
				global _cdobj
				codetable _chdir
				global _chdir
				codetable _getcwd
				global _getcwd
				codetable _opendir
				global _opendir
				codetable _readdir
				global _readdir
				codetable _rewinddir
				global _rewinddir
				codetable _closedir
				global _closedir
				codetable _getpwent
				global _getpwent
				codetable _getpwuid
				global _getpwuid
				codetable _getpwnam
				global _getpwnam
				codetable _setpwent
				global _setpwent
				codetable _endpwent
				global _endpwent
				codetable _getpid
				global _getpid
				codetable _getppid
				global _getppid
				codetable _getuid
				global _getuid
				codetable _geteuid
				global _geteuid
				codetable _getgid
				global _getgid
				codetable _getegid
				global _getegid
				codetable _setuid
				global _setuid
				codetable _setgid
				global _setgid
				codetable _getgroups
				global _getgroups
				codetable _getlogin
				global _getlogin
				codetable _cuserid
				global _cuserid
				codetable _getpgrp
				global _getpgrp
				codetable _setsid
				global _setsid
				codetable _setpgid
				global _setpgid
				codetable _uname
				global _uname
				codetable _time
				global _time
				codetable _times
				global _times
				codetable _getenv
				global _getenv
				codetable _ctermid
				global _ctermid
				codetable _ttyname
				global _ttyname
				codetable _isatty
				global _isatty
				codetable _sysconf
				global _sysconf
				codetable __exit
				global __exit
				codetable _vfork
				global _vfork
				codetable _execl
				global _execl
				codetable _execv
				global _execv
				codetable _execle
				global _execle
				codetable _execve
				global _execve
				codetable _execlp
				global _execlp
				codetable _execvp
				global _execvp
				codetable _wait
				global _wait
				codetable _wait2
				global _wait2
				codetable _system
				global _system
				codetable _kill
				global _kill
				codetable _sigemptyset
				global _sigemptyset
				codetable _sigfillset
				global _sigfillset
				codetable _sigaddset
				global _sigaddset
				codetable _sigdelset
				global _sigdelset
				codetable _sigismember
				global _sigismember
				codetable _sigaction
				global _sigaction
				codetable _sigprocmask
				global _sigprocmask
				codetable _sigpending
				global _sigpending
				codetable _sigsuspend
				global _sigsuspend
				codetable _alarm
				global _alarm
				codetable _pause
				global _pause
				codetable _sleep
				global _sleep
				codetable _signal
				global _signal
				codetable _raise
				global _raise
				codetable __ignore_signal_handler
				global __ignore_signal_handler
				codetable __default_signal_handler
				global __default_signal_handler
				codetable __error_signal_marker
				global __error_signal_marker
				codetable _find_file
				global _find_file
			data _oserr, 4
			global _oserr 
				codetable __posixflags
				global __posixflags
				codetable _exit
				global _exit
				codetable _atexit
				global _atexit
				codetable _abort
				global _abort
				codetable _cf_getospeed
				global _cf_getospeed
				codetable _cf_setospeed
				global _cf_setospeed
				codetable _cf_getispeed
				global _cf_getispeed
				codetable _cf_setispeed
				global _cf_setispeed
				codetable _tcgetattr
				global _tcgetattr
				codetable _tcsetattr
				global _tcsetattr
				codetable _tcsendbreak
				global _tcsendbreak
				codetable _tcdrain
				global _tcdrain
				codetable _tcflush
				global _tcflush
				codetable _tcflow
				global _tcflow
				codetable _tcgetpgrp
				global _tcgetpgrp
				codetable _tcsetpgrp
				global _tcsetpgrp
				codetable _sopen
				global _sopen
				codetable _svopen
				global _svopen
				codetable _getenviron
				global _getenviron
				codetable _select
				global _select
				codetable _socket
				global _socket
				codetable _bind
				global _bind
				codetable _listen
				global _listen
				codetable _accept
				global _accept
				codetable _connect
				global _connect
				codetable _socketpair
				global _socketpair
				codetable _gethostid
				global _gethostid
				codetable _gethostname
				global _gethostname
				codetable _getpeername
				global _getpeername
				codetable _getsockname
				global _getsockname
				codetable _getsockopt
				global _getsockopt
				codetable _setsockopt
				global _setsockopt
				codetable _recv
				global _recv
				codetable _recvfrom
				global _recvfrom
				codetable _recvmsg
				global _recvmsg
				codetable _send
				global _send
				codetable _sendto
				global _sendto
				codetable _sendmsg
				global _sendmsg
				codetable _shutdown
				global _shutdown
				codetable _opendb
				global _opendb
				codetable _scandb
				global _scandb
				codetable _closedb
				global _closedb
			data _dbinfo, 4
			global _dbinfo 
				codetable _swap_long
				global _swap_long
				codetable _swap_short
				global _swap_short
				codetable _gettimeofday
				global _gettimeofday
				codetable __posix_exception_handler
				global __posix_exception_handler
				codetable _setsigstacksize
				global _setsigstacksize
				codetable _getdtablesize
				global _getdtablesize
				codetable _waitpid
				global _waitpid
				codetable _wait3
				global _wait3
				codetable _lstat
				global _lstat
				codetable _fderror
				global _fderror
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
						int shift(-2, labelref(.fderror))
						int shift(-2, labelref(.lstat))
						int shift(-2, labelref(.wait3))
						int shift(-2, labelref(.waitpid))
						int shift(-2, labelref(.getdtablesize))
						int shift(-2, labelref(.setsigstacksize))
						int shift(-2, labelref(._posix_exception_handler))
						int shift(-2, labelref(.gettimeofday))
						int shift(-2, labelref(.swap_short))
						int shift(-2, labelref(.swap_long))
						int shift(-2, labelref(.closedb))
						int shift(-2, labelref(.scandb))
						int shift(-2, labelref(.opendb))
						int shift(-2, labelref(.shutdown))
						int shift(-2, labelref(.sendmsg))
						int shift(-2, labelref(.sendto))
						int shift(-2, labelref(.send))
						int shift(-2, labelref(.recvmsg))
						int shift(-2, labelref(.recvfrom))
						int shift(-2, labelref(.recv))
						int shift(-2, labelref(.setsockopt))
						int shift(-2, labelref(.getsockopt))
						int shift(-2, labelref(.getsockname))
						int shift(-2, labelref(.getpeername))
						int shift(-2, labelref(.gethostname))
						int shift(-2, labelref(.gethostid))
						int shift(-2, labelref(.socketpair))
						int shift(-2, labelref(.connect))
						int shift(-2, labelref(.accept))
						int shift(-2, labelref(.listen))
						int shift(-2, labelref(.bind))
						int shift(-2, labelref(.socket))
						int shift(-2, labelref(.select))
						int shift(-2, labelref(.getenviron))
						int shift(-2, labelref(.svopen))
						int shift(-2, labelref(.sopen))
						int shift(-2, labelref(.tcsetpgrp))
						int shift(-2, labelref(.tcgetpgrp))
						int shift(-2, labelref(.tcflow))
						int shift(-2, labelref(.tcflush))
						int shift(-2, labelref(.tcdrain))
						int shift(-2, labelref(.tcsendbreak))
						int shift(-2, labelref(.tcsetattr))
						int shift(-2, labelref(.tcgetattr))
						int shift(-2, labelref(.cf_setispeed))
						int shift(-2, labelref(.cf_getispeed))
						int shift(-2, labelref(.cf_setospeed))
						int shift(-2, labelref(.cf_getospeed))
						int shift(-2, labelref(.abort))
						int shift(-2, labelref(.atexit))
						int shift(-2, labelref(.exit))
						int shift(-2, labelref(._posixflags))
						int shift(-2, labelref(.find_file))
						int shift(-2, labelref(._error_signal_marker))
						int shift(-2, labelref(._default_signal_handler))
						int shift(-2, labelref(._ignore_signal_handler))
						int shift(-2, labelref(.raise))
						int shift(-2, labelref(.signal))
						int shift(-2, labelref(.sleep))
						int shift(-2, labelref(.pause))
						int shift(-2, labelref(.alarm))
						int shift(-2, labelref(.sigsuspend))
						int shift(-2, labelref(.sigpending))
						int shift(-2, labelref(.sigprocmask))
						int shift(-2, labelref(.sigaction))
						int shift(-2, labelref(.sigismember))
						int shift(-2, labelref(.sigdelset))
						int shift(-2, labelref(.sigaddset))
						int shift(-2, labelref(.sigfillset))
						int shift(-2, labelref(.sigemptyset))
						int shift(-2, labelref(.kill))
						int shift(-2, labelref(.system))
						int shift(-2, labelref(.wait2))
						int shift(-2, labelref(.wait))
						int shift(-2, labelref(.execvp))
						int shift(-2, labelref(.execlp))
						int shift(-2, labelref(.execve))
						int shift(-2, labelref(.execle))
						int shift(-2, labelref(.execv))
						int shift(-2, labelref(.execl))
						int shift(-2, labelref(.vfork))
						int shift(-2, labelref(._exit))
						int shift(-2, labelref(.sysconf))
						int shift(-2, labelref(.isatty))
						int shift(-2, labelref(.ttyname))
						int shift(-2, labelref(.ctermid))
						int shift(-2, labelref(.getenv))
						int shift(-2, labelref(.times))
						int shift(-2, labelref(.time))
						int shift(-2, labelref(.uname))
						int shift(-2, labelref(.setpgid))
						int shift(-2, labelref(.setsid))
						int shift(-2, labelref(.getpgrp))
						int shift(-2, labelref(.cuserid))
						int shift(-2, labelref(.getlogin))
						int shift(-2, labelref(.getgroups))
						int shift(-2, labelref(.setgid))
						int shift(-2, labelref(.setuid))
						int shift(-2, labelref(.getegid))
						int shift(-2, labelref(.getgid))
						int shift(-2, labelref(.geteuid))
						int shift(-2, labelref(.getuid))
						int shift(-2, labelref(.getppid))
						int shift(-2, labelref(.getpid))
						int shift(-2, labelref(.endpwent))
						int shift(-2, labelref(.setpwent))
						int shift(-2, labelref(.getpwnam))
						int shift(-2, labelref(.getpwuid))
						int shift(-2, labelref(.getpwent))
						int shift(-2, labelref(.closedir))
						int shift(-2, labelref(.rewinddir))
						int shift(-2, labelref(.readdir))
						int shift(-2, labelref(.opendir))
						int shift(-2, labelref(.getcwd))
						int shift(-2, labelref(.chdir))
						int shift(-2, labelref(.cdobj))
						int shift(-2, labelref(.fdstream))
						int shift(-2, labelref(.lseek))
						int shift(-2, labelref(.fcntl))
						int shift(-2, labelref(.write))
						int shift(-2, labelref(.read))
						int shift(-2, labelref(.close))
						int shift(-2, labelref(.dup2))
						int shift(-2, labelref(.dup))
						int shift(-2, labelref(.pipe))
						int shift(-2, labelref(.fpathconf))
						int shift(-2, labelref(.pathconf))
						int shift(-2, labelref(.utime))
						int shift(-2, labelref(.chown))
						int shift(-2, labelref(.chmod))
						int shift(-2, labelref(.access))
						int shift(-2, labelref(.fstat))
						int shift(-2, labelref(.stat))
						int shift(-2, labelref(.rename))
						int shift(-2, labelref(.rmdir))
						int shift(-2, labelref(.unlink))
						int shift(-2, labelref(.mkfifo))
						int shift(-2, labelref(.mkdir))
						int shift(-2, labelref(.link))
						int shift(-2, labelref(.umask))
						int shift(-2, labelref(.creat))
						int shift(-2, labelref(.open))
						int shift(-2, labelref(._posix_init))
				_FuncTableEnd:			
		ref	Kernel.library
		ref	SysLib.library
		ref	Util.library
