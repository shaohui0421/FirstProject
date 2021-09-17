#!/bin/bash

src=$1
dst=$2
rcc_log="/var/log/rcc_client.log"

CP_SUCCESS=0
CP_ERR_SYNTAX=1
CP_ERR_NOT_FILE=2

src_size=0
dst_size=0
last_dst_size=0

function isalive()
{
    ps -p $1 > /dev/null
    return $?
}

if [ -z $src ] || [ -z $dst ]; then
    echo "cp command err!" >> $rcc_log
    exit $CP_ERR_SYNTAX
fi

if [ ! -f $src ]; then
    echo "cp commmand err, not file!" >> $rcc_log
    exit $CP_ERR_NOT_FILE
fi

cp -f "$src" "$dst" &
cp_pid=$!

count=0
while [ 1 ]; do
    src_size=`ls -l ${src} | awk '{print $5}'`
    if [ -f ${dst} ]; then
        last_dst_size=${dst_size}
        dst_size=`ls -l ${dst} | awk '{print $5}'`
    fi
    if [ ${src_size} != "0" ]; then	    
        percent=`echo "scale=2; ${dst_size}*100/${src_size}" | bc -l`
        rate=`echo "scale=2; (${dst_size}-${last_dst_size})/1024/1024" | bc -l`
    fi
    isalive ${cp_pid}
    if [ $? != "0" ]; then
        echo "100%"
        exit $CP_SUCCESS
    fi
    if [ ${dst_size} -eq 0 ]; then
        echo "dst size is 0" >> $rcc_log
        echo "0%"
    elif [ ${count} -eq 0 ]; then
        echo "count is 0" >> $rcc_log
        echo "0%"
    elif [ ${dst_size} -gt ${src_size} ]; then
        echo "dst size is greater than src size, dst=$dst_size, src=$src_size" >> $rcc_log
        echo "0%"
    elif [ ${last_dst_size} -gt ${dst_size} ]; then
        echo "last dst size is greater than dst size, last=$last_dst_size, dst=$dst_size" >> $rcc_log
        echo "0%"
    else
        echo "${percent}% ${rate}MB/s"
    fi
    ((count++))
    sleep 1
done
