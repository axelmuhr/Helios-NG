echo off

echo copying ... source                target
echo             %1\etc                %2\etc
copy             %1\etc\*.*            %2\etc            > NUL
echo             %1\bin                %2\bin
copy             %1\bin\*.*            %2\bin            > NUL
echo             %1\system             %2\system
copy             %1\system\*.*         %2\system         > NUL
echo             %1\users\root         %2\users\root
copy             %1\users\root\*.*     %2\users\root     > NUL
echo             %1\users\guest        %2\users\guest
copy             %1\users\guest\*.*    %2\users\guest    > NUL
echo             %1\users\shutdown     %2\users\shutdown
copy             %1\users\shutdown\*.* %2\users\shutdown > NUL

:disk2

echo Insert Helios Release Disk 2 into drive %1
pause

if not exist     %1\server.exe         goto disk2
if not exist     %1\host.con           goto disk2
if not exist     %1\lib\*              goto disk2
if not exist     %1\local\bin\*        goto disk2

echo copying ... source                target
echo             %1\server.exe         %2\server.exe
copy             %1\server.exe         %2\server.exe     > NUL
echo             %1\host.con           %2\host.con
copy             %1\host.con           %2\host.con       > NUL
echo             %1\lib                %2\lib
copy             %1\lib\*.*            %2\lib            > NUL
echo             %1\local\bin          %2\local\bin
copy             %1\local\bin\*.*      %2\local\bin      > NUL

echo -
echo Your Helios 1.2.2 system has been copied.
echo A summary of the enhancements provided by version 1.2.2 over earlier
echo releases is located in %2\system\readme.122
echo -
echo If you have a previous version of the system, you may wish to copy the 
echo relevant system specific configuration files (e.g. host.con, initrc,
echo resource maps, etc) to your new Helios directory.
echo -
echo If you have more discs to install, boot up Helios and use loadpac to 
echo install them.
echo -

:disk1a
echo Re-insert Helios Release Disk 1 into drive %1 to complete installation
pause
REM check an arbitrary directory ...
if not exist     %1\users\shutdown\*.* goto disk1a
