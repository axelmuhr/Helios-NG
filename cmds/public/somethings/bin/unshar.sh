#!/bin/sh
#
#  unshar shell - unpack one or more shar files
#
#  Copyright 1988 bill davidsen
#	this program may be used by any person for any purpose.

for Name in $*
do	sed -n '/^[:#]/,$p' $Name | sh
done
