#!/bin/bash
#Description :- This script creates number of hardlinks or symlinks , wirte the
#		name of successsfully file in "temp_directory/log"  and checks 
#		given number of links created or not.It create links inside 
#		"temp_directory" directory and success message inside "/tmp/logs" file .  
#               while running this script second time "rm /tmp/logs"  and "rm -r temp_directory" 

USAGE=" usage :\n[ -s | -h ]  no_of_symlink or no_of_hardlink 
		  -s for symlink , -h for hardlink"

if [ $# -ne 2 ] 
then 

      echo " $USAGE  " ; exit 1;

fi

mkdir temp_directory
cd temp_directory

case $1 in 

  -s)	mkdir symlink 
	cd symlink
	touch file0
	for (( i =1 ; i<= $2 ; i++ ))	
	do	
		ln -s  file0  file$i 2> /dev/null
		if test $? -ne 0
		then
			echo " cant create link for file$i"
			exit -1
		fi			
		echo "file$i"  >>  ../logs
	done
	cd ../../	
	;;

  -h)	mkdir hardlink
	cd hardlink
	touch file0
	for (( i =1 ; i <= $2 ; i++ ))
	do
		ln  file0  file$i
     	        if test $? -ne 0
                then
                        echo " cant create link for file$i"
                        exit -1
                fi
		echo "file$i"  >> ../logs
	done
	cd ../../
	;;

   *)    echo $usage ;;

esac



case $1  in


   -s)  cd  temp_directory/symlink
	count1=`ls -R | wc -l` 
	count=`expr $count1 - 1`
	if [ $i -eq $count ]
	then
		count2=$(($count -1))
		echo "successfully created $count2 symlinks"  >> /tmp/logs
	fi
	;;

   -h) 	cd  temp_directory/hardlink
	for (( j=0 ; j < $2 ; j++ ))
	do
		count1=`stat file$j | grep Links | cut  -c 47- `
        	if [ $i -eq $count1 ]
        	then
			count=$(($count1 - 1))
                	echo "success  for file$j" >> /tmp/logs
        	fi
	done
	;;
esac
