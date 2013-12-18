echo off
set kbytes=3100

set source=%1
if "%source%"=="" goto usage
REM *should* never happen (i.e. if called from install ...)

set target=%2

if not exist %source%\hfree.exe goto errinst2

REM enough free disk space ?
%source%\hfree %kbytes% %2
if errorlevel 2 goto errfree
if errorlevel 1 set target=%2\helios

if exist %target%\* goto errmkdir

if not exist %source%\install2.bat goto errinst1

echo -
echo Please verify source and target specification :
echo Source drive     = %source%
echo Target directory = %target%
echo If incorrect, type CTRL-C and rerun install with the correct parameters 
echo (Usage: install source_drive [target_drive][target_directory])

pause

cls
echo creating %target% directories ...
mkdir %target%
mkdir %target%\etc
mkdir %target%\lib
mkdir %target%\bin
mkdir %target%\users
mkdir %target%\users\root
mkdir %target%\users\guest
mkdir %target%\users\shutdown
mkdir %target%\tmp
mkdir %target%\local
mkdir %target%\local\bin
mkdir %target%\local\lib
mkdir %target%\system
mkdir %target%\include
mkdir %target%\include\sys

copy %source%\install2.bat %target%\tmp > NUL
%target%\tmp\install2.bat %source% %target%

:errmkdir
echo ERROR: Target directory: %target% already exists
goto usage

:errinst1
echo ERROR: Failed to find %source%\install2.bat
goto usage

:errinst2
echo ERROR: Failed to find %source%\hfree.exe
goto usage

:errfree
echo ERROR: Insufficient disk space for installation (%kbytes% Kbytes required)
goto exit

:usage
echo Usage: install source_drive [target_drive][target_directory]
echo default target drive is current drive
echo default target directory is \helios
goto exit

:exit
REM Helios Installation script
