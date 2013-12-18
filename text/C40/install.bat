echo off
cls

echo **************************************************************************
echo *                                                                        *
echo *                 Helios 1.3 Batch File Installation                     *
echo *                                                                        *
echo *                       Perihelion Software Ltd                          *
echo *                                                                        *
echo **************************************************************************

if %1x==x goto erruse

if not exist %1\install1.bat goto errinst

REM set command processor's environment block size
command /E:1024 /C %1\install1.bat %1 %2

REM clean up
if exist %2\tmp\install2.bat        erase %2\tmp\install2.bat
if exist %2\helios\tmp\install2.bat erase %2\helios\tmp\install2.bat
goto exit

:errinst
echo ERROR: failed to find %1\install1.bat
goto erruse

:erruse
echo Usage: install source_drive [target_drive][target_directory]
echo e.g. a:install a:
echo      a:install a: c:
echo      a:install a: \helios
echo default target drive is current drive
echo default target directory is \helios
goto exit

:exit
REM Helios Installation script
