#!/bin/bash
###############################################################################
## Written By : 
## Purpose    : Check atime ctime and mtime after file operation
###############################################################################
# run this script should be require root
# here create zpool /mytank
# Check time (atime ctime and mtime) behave with created file on zpool
command_success_or_fail(){  ## Function for check command successfully executed or not
if test $? -ne 0
then
 echo "Command fail"
 exit 1
fi
}      
## Create file on zpool
touch /tank/file 
## Create file on zpool
command_success_or_fail
pre_atime=`stat -c %x /tank/file 2> /dev/null | cut -d' ' -f2`
command_success_or_fail
pre_ctime=`stat -c %y /tank/file 2> /dev/null | cut -d' ' -f2`
command_success_or_fail
pre_mtime=`stat -c %z /tank/file 2> /dev/null | cut -d' ' -f2`
command_success_or_fail
sleep 1
echo "Hi, Well Come in Linux World " >> file
sleep 1
#echo "Check atime , ctime and mtime after appending file "
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
chmod 0777 file
command_success_or_fail
sleep 1
post_chmod_ctime=`stat -c %y /tank/file 2> /dev/null | cut -d' ' -f2`
command_success_or_fail
#echo "post chmod ctime = $post_chmod_ctime"
if test $pre_ctime != $post_chmod_ctime
then
      echo  " PASS "    
else
      echo  " FAIL "
fi

rm -f file
command_success_or_fail
