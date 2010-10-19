#!/bin/bash
i=0
count=`df -kh /tank | tr -s ' %' ' ' | cut -f5 -d ' '|tail -n 1`
while [ $count -lt 90 ]
do
    dd if=/dev/urandom of=/tank/fs$i bs=4K count=256
    if test $? -ne 0
    then
        echo " Value i = $i"
    fi
    count=`df -kh /tank | tr -s ' %' ' ' | cut -f5 -d ' '|tail -n 1`
    i=`expr $i + 1`
done
