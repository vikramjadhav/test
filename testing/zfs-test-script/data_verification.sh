#!/bin/bash

command_success_or_fail(){
if test $? -ne 0
then
 echo "Command fail"
 exit 1
fi
}
modprobe lzfs
echo "create a zpool"
echo "##############################################"
zpool create -f tank /dev/sdb1
##############################################
echo "###############################################"
echo "create a 100M file on ext2"
echo "###############################################"
dd if=/dev/urandom of=/home/kqinfo/file1 bs=100K count=1024
command_success_or_fail
cp -fr /home/kqinfo/file1 /tank/file
echo "#################################################"
echo "Verify that file on ZFS and ext2 match"
echo "#################################################"
diff /home/kqinfo/file1 /tank/file
command_success_or_fail
#cd /home/kqinfo/2.6.31.5-127.fc12.x86_64/zfs-test-script
gcc mmap_cp.c
./a.out /home/kqinfo/file1  /tank/file
zfs umount -a
#########################################
##zfs/zfs umount
#########################################
diff home/kqinfo/file1 /tank/file
echo "###################################################"
echo "Truncate a zfs file (file2) to size 10M"
echo "###################################################"
truncate -s 10M /tank/file2
echo "###################################################"
echo " out put du -sh cmd "
echo "###################################################"
du -sh /tank/file2
zfs umount -a
###########################################
##zfs/zfs mount 
##########################################
cd /home/kqinfo
diff /home/kqinfo/file1 /tank/file2
rm -f /tank/file2
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
command_success_or_fail
