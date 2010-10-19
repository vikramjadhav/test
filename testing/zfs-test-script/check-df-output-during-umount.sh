#!/bin/bash

###############################################################
##
## Purpose : Check df at umount fs time
## NOTE    : Observe the df -kh output while the umount operation is still in progress. zfs file system (tank) should not be visible in the df -kh output.
##           If the zfs file system (tank) is still visible in df -kh  output, note if the size and %fill parameters of tanks are same as root file system
##
###############################################################
##
## Run this script should require fill_zfs.sh shell script 
## Fuction written check fired command successfully work or fail
##
command_success_or_fail(){if test $? -ne 0
then
 echo "Command fail"
 exit 1
fi
}
##
## This script fill fs 95%
## Load module 
##
modprobe lzfs
command_success_or_fail
##
## Create zpool(tank) with /dev/sdc device
##
zpool create -f tank /dev/sdb1
command_success_or_fail
zpool list
command_success_or_fail
##
## To run fill_zfs.sh script fill up fs(zfs fs) with 95% of that size
##
./fill_zfs.sh
##
##zfs/zfs umount tank
##
umount tank
command_success_or_fail
###################################################################
##
## Try to umount the zfs file systems (this would result in data being sync on to disk)
## 
sleep 1
echo "##################################################################"
echo ""
echo " Observe the df -kh output while the umount operation is still in progress "
echo ""
echo "##################################################################"
sleep 1
