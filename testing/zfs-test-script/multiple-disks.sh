#!/bin/bash
####################################################################
####################################################################
##
## This script work for check multiple disk uses
## Two zpools names: tank (tank11) and tank1 (tank12)
## Two disk names: disk1(/dev/sdc) (size > 1G) and disk2 (/dev/sdb) (size > 2G)
## Written a function check command succeessfully work or fail
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
## 1) Create a zpool (tank) using a disk (disk1)
zpool create -f tank11 /dev/sdb1 
command_success_or_fail
zpool list
command_success_or_fail
## 2) Try to create same zpool (tank) using same disk (disk1)
zpool create -f tank11 /dev/sdb1
command_success_or_fail
## 3) Try to create same zpool (tank) using a disk (disk2) 
zpool create -f tank11 /dev/sda1
command_success_or_fail
## 4) unmount the zfs file system (tank)
zfs umount tank11
command_success_or_fail
## 5) Try to create another zpool (tank1) using disk (disk1)
zpool create -f tank12 /dev/sdb1
command_success_or_fail
## 6) Create a second zpool (tank1) with disk (disk2)
zpool create -f tank12 /dev/sda1
command_success_or_fail
zpool list
command_success_or_fail
#zpool/zpool create -f tank11 /dev/sdb
#command_success_or_fail
## 7) umount the zfs file system (tank1)
zfs umount tank12
command_success_or_fail
## 8) Try to add disk2 to zpool tank
zpool add -f tank11 /dev/sda1
command_success_or_fail
## 9) Destroy tank1
zpool destroy -f tank12
command_success_or_fail
zpool list
command_success_or_fail
## 10) Try to add disk2 to zpool tank
zpool add -f tank11 /dev/sda1
command_success_or_fail
## 12) Note the size of the file system changed as a disk is added to pool
zpool list
command_success_or_fail
## called fill_tank.sh script on here
## Create files to fille tank to 90%
./fill_tank.sh 
#14 
## Try to remove a disk (disk1 /dev/sdc) from the zpool (it should this fail or successed)
## Here added disk can't be remove/detach so onward script not work
## 14) Try to remove a disk (disk1) from the zpool

