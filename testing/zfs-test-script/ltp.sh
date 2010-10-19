#!/bin/bash

##########################################################################
##
## Perpose : To test ltp test on zpool (fedora12 64 bit)
##
##########################################################################
##
## Fuction written check fired command successfully work or fail
##
## Function written check fired command success or fail
command_success_or_fail(){
if test $? -ne 0
then
 echo "Command fail"
 exit 1
fi
}
## This script rum must in root user
## This script run for all fs on zfs
modprobe lzfs
command_success_or_fail
zpool create -f tank /dev/sdb1
command_success_or_fail
zpool list
command_success_or_fail
## cd /home/kqinfo/Desktop/ltp-full-20100630
## command_success_or_fail
cd /tank
command_success_or_fail
/opt/ltp/runltp -p -q -l /tmp/abc.log -f /opt/ltp/runtest/fs 
command_success_or_fail
##zfs/zfs unmount tank
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
