#!/bin/bash
############################################################################
## Written By  : Sachin Dhomse
## Perpose     : Check atime ctime and mtime of created file on zpool
############################################################################
##
## Run this script user must be superuser
##
## Written function check fired command success or fail

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
touch /tank/file
command_success_or_fail
pre_atime=`stat -c %x /tank/file 2> /dev/null | cut -d' ' -f2`
command_success_or_fail
pre_ctime=`stat -c %y /tank/file 2> /dev/null | cut -d' ' -f2`
command_success_or_fail
pre_mtime=`stat -c %z /tank/file 2> /dev/null | cut -d' ' -f2`
command_success_or_fail
sleep 1
echo "Hi, Well Come in Linux World " >> /tank/file
sleep 1
post_atime=`stat -c %x /tank/file 2> /dev/null | cut -d' ' -f2`
command_success_or_fail
post_ctime=`stat -c %y /tank/file 2> /dev/null | cut -d' ' -f2`
command_success_or_fail
post_mtime=`stat -c %z /tank/file 2> /dev/null | cut -d' ' -f2`
command_success_or_fail
if test $pre_ctime !=  $post_ctime -a $pre_mtime != $post_mtime
then
      echo  " PASS "    
else
      echo  " FAIL "
fi
sleep 1
chmod 0777 /tank/file
command_success_or_fail
sleep 1
post_chmod_ctime=`stat -c %y /tank/file 2> /dev/null | cut -d' ' -f2`
command_success_or_fail
if test $pre_ctime != $post_chmod_ctime
then
      echo  " PASS "    
else
      echo  " FAIL "
fi
rm -f /tank/file
command_success_or_fail
zfs unmount tank
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
