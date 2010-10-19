#!/bin/bash

#############################################################

#############################################################

## Function writtened check command successfully work or fail
command_success_or_fail(){
if test $? -ne 0
then
 echo "Command fail"
 exit 1
fi
}
#1. create a big file size > 1GB
#Note the du -sh file output
#Note the df -kh output
#Note the number of blocks allocated to the file

modprobe lzfs
command_success_or_fail
zpool create -f tank /dev/sdb1
command_success_or_fail
dd if=/dev/urandom of=/tank/file bs=16K count=65536
command_success_or_fail
du -sh /tank/file
sleep 1
echo "Output of du -sh "
command_success_or_fail
df -kh /tank/file
sleep 1
echo "Output of df -kh "
command_success_or_fail
stat /tank/file
sleep 1
echo "Output of stat cmd  "
command_success_or_fail

#2. Overwrite  the file with to make it of 50M
#Note the du -sh file output
#Note the df -kh output
#Note the number of blocks allocated to the file
sleep 1
echo "Overwrite the file "
dd if=/dev/urandom of=/tank/file bs=16K count=3200
command_success_or_fail
du -sh /tank/file
sleep 1
echo "Output of du -sh "
command_success_or_fail
df -kh /tank/file
sleep 1
echo "Output of du -sh "
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

