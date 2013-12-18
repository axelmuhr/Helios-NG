:

#
# extract MachineDep's value and output it
#

arch=$1/Arch.c

if [ $arch = "/Arch.c" ]; then
    echo "usage:  /bin/sh $0 $(IVCONFIGSRC)"
    exit 1
fi

cc -E $arch | sed -e '/^$/d' -e '/^#  *[0-9]/d' -e 's/^# architecture:  //'
