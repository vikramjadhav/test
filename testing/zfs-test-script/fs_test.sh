#!/bin/bash

##########################################################################
##
## Perpose : fs test on zpool (fedora12 64 bit)
##
##########################################################################
##
## Befor running this script required prove on your machine
##
## function written check fired command success or fail
##
command_success_or_fail(){
if test $? -ne 0
then
 echo "Command fail"
 exit 1
fi
}
modprobe lzfs
command_success_or_fail
zpool create -f tank /dev/sdb1
command_success_or_fail
zpool list
command_success_or_fail
##cd 
##command_success_or_fail
cd /tank
command_success_or_fail
prove /home/kqinfo/ZFS-test/testing/fstest/pjd-fstest-20080816/tests/* > /tmp/fstest.log
#######################################
##
## zfs umount tank
##
umount tank
command_success_or_fail
zpool destroy -f tank
command_success_or_fail
zpool list
command_success_or_fail
modprobe -r lzfs
modprobe -r zfs
modprobe -r zcommon
modprobe -r zunicode
modprobe -r znvpair
modprobe -r zavl
modprobe -r spl
modprobe -r zlib_deflate
command_success_or_fail
