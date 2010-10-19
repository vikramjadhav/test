#!/bin/bash

######################################################
######################################################
## This script check inode information
## Function written check fired command successfully work or fail
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
echo "Create file on zpool with size 128K"
dd if=/dev/urandom of=/tank/file bs=4 count=32768
command_success_or_fail
sleep 1
echo "stat command output "
stat /tank/file
command_success_or_fail
sleep 1
echo "du cmd output"
du -sh /tank/file 
command_success_or_fail
echo "Create file on zpool with size 256K"
dd if=/dev/urandom of=/tank/file bs=4 count=65536
command_success_or_fail
sleep 1
echo "stat command output "
stat /tank/file
command_success_or_fail
sleep 1
echo "du cmd output"
du -sh /tank/file
command_success_or_fail
echo "Create file on zpool with size 1M"
dd if=/dev/urandom of=/tank/file bs=4 count=262144
command_success_or_fail
sleep 1
echo "stat command output "
stat /tank/file
command_success_or_fail
sleep 1
echo "du cmd output"
du -sh /tank/file
command_success_or_fail
echo "Truncate file 50K"
dd if=/dev/urandom of=/tank/file bs=4 count=12800
command_success_or_fail
sleep 1
echo "stat command output "
stat /tank/file
command_success_or_fail
sleep 1
echo "du cmd output"
du -sh /tank/file 
command_success_or_fail
rm -f /tank/file 
command_success_or_fail
zfs umount -a 
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
