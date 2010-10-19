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
option=$1
device=$2
d="-dev"


if test $option != $d 
then
   echo "Usage : [script name] [-dev] [device name]"
   exit 1
fi


cd /home/kqinfo/ZFS_kq/zfs-0.4.7/scripts
command_success_or_fail
./zfs.sh
cd ../cmd/
command_success_or_fail
## zpool/zpool create -f tank /dev/sdc
zpool/zpool create -f tank $2
command_success_or_fail
zpool/zpool list
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
zfs/zfs unmount -a 
command_success_or_fail
zpool/zpool destroy -f tank
command_success_or_fail
zpool/zpool list
command_success_or_fail
cd ../scripts/
command_success_or_fail
./zfs.sh -u
command_success_or_fail

