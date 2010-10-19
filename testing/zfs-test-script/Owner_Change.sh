#!/bin/bash

 
#To check command successfully work or fail
command_success_or_fail(){
if test $? -ne 0
then
 echo "Command fail"
 exit 1
fi
}
modprobe lzfs
command_success_or_fail
#Create a zpool
zpool create -f tank /dev/sdb1
zpool list
#check the module usage count of the zfs module
lsmod
#try to unload the zfs module and it should fai
modprobe -r lzfs
modprobe -r zfs
modprobe -r zcommon
modprobe -r zunicode
modprobe -r znvpair
modprobe -r zavl
modprobe -r spl
modprobe -r zlib_deflate
#create 100 zfs file systems and mount them all
for i in `seq 1 10`;do zfs create tank/fs$i;done
mount -a
#notice the module usage count again (usage count > 1)
lsmod
#unload zfs module operatin should fail
modprobe -r lzfs
modprobe -r zfs
modprobe -r zcommon
modprobe -r zunicode
modprobe -r znvpair
modprobe -r zavl
modprobe -r spl
modprobe -r zlib_deflate
#umount one of the zfs file system
zfs umount tank/fs1
#unload zfs module operatin should fail
modprobe -r lzfs
modprobe -r zfs
modprobe -r zcommon
modprobe -r zunicode
modprobe -r znvpair
modprobe -r zavl
modprobe -r spl
modprobe -r zlib_deflate
