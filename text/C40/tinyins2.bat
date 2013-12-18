echo off

echo copying ... source                target
echo             %1\bin                %2\bin
copy             %1\bin\*.*            %2\bin            > NUL

:disk2

echo Insert Helios Installation Disk 2 into drive %1 
pause

if not exist     %1\bin\y*.*           goto disk2

echo copying ... source                target
echo             %1\bin                %2\bin
copy             %1\bin\*.*            %2\bin            > NUL
echo             %1\etc                %2\etc
copy             %1\etc\*.*            %2\etc            > NUL
echo             %1\system             %2\system
copy             %1\system\*.*         %2\system         > NUL
echo             %1\users\root         %2\users\root
copy             %1\users\root\*.*     %2\users\root     > NUL
echo             %1\users\guest        %2\users\guest
copy             %1\users\guest\*.*    %2\users\guest    > NUL
echo             %1\users\shutdown     %2\users\shutdown
copy             %1\users\shutdown\*.* %2\users\shutdown > NUL
echo             %1\lib                %2\lib
copy             %1\lib\*.*            %2\lib            > NUL
echo             %1\local\bin          %2\local\bin
copy             %1\local\bin\*.*      %2\local\bin      > NUL

:disk3

echo Insert Helios Installation Disk 3 into drive %1 
pause

if not exist     %1\server.exe         goto disk3

echo copying ... source                target
echo             %1\*.con              %2
copy             %1\*.con              %2		> NUL
echo             %1\server.exe         %2\server.exe
copy             %1\server.exe         %2\server.exe    > NUL
echo             %1\bin                %2\bin
copy             %1\bin\*.*            %2\bin		> NUL
echo             %1\local\bin          %2\bin
copy             %1\local\bin\*.*      %2\bin		> NUL
echo             %1\etc\flaws          %2\etc\flaws
copy             %1\etc\flaws          %2\etc\flaws    > NUL

echo -
echo Your Tiny Helios 1.3 system has been copied.
echo -
echo If you have a previous version of the system, you may wish to copy the 
echo relevant system specific configuration files (e.g. host.con, initrc, etc)
echo to your new Helios directory.
echo -
echo To install the remaining disks, boot up Helios and use loadpac to 
echo install them.
echo -
echo There is a "flaws" file in /helios/etc that describes the known
echo problems that exist in this version of Helios.
echo -

:disk1a
echo Re-insert Helios Installation Disk 1 into drive %1 to complete installation
pause
REM check an arbitrary file ...
if not exist     %1\hfree.exe goto disk1a
