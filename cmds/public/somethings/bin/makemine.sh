#!/bin/csh
mkdir /tmp/$USER
foreach j ( $* )
	cp $j /tmp/$USER/$j
	rm -f $j
	mv /tmp/$USER/$j $j
end
rm -rf /tmp/$USER
